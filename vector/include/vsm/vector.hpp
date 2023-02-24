#pragma once

#include <vsm/assert.h>
#include <vsm/algorithm/remove_unstable.hpp>
#include <vsm/default_allocator.hpp>
#include <vsm/platform.h>
#include <vsm/utility.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <new>

#include <cstddef>
#include <cstring>

vsm_msvc_warning(push)
vsm_msvc_warning(disable: 4102) // unreferenced label

namespace vsm {
namespace detail::vector_ {

using std::byte;

template<typename T>
using is_trivially_relocatable = std::is_trivially_copyable<T>;

template<typename T>
constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<T>::value;

template<typename T>
void relocate(T* dst, T* src_beg, T* const src_end)
{
	if constexpr (is_trivially_relocatable_v<T>)
	{
		std::memcpy(
			reinterpret_cast<byte*>(dst),
			reinterpret_cast<byte*>(src_beg),
			reinterpret_cast<byte*>(src_end) - reinterpret_cast<byte*>(src_beg));
	}
	else
	{
		for (; src_beg != src_end; ++dst, ++src_beg)
		{
			new (dst) T(vsm_move(*src_beg));
			src_beg->~T();
		}
	}
}

template<typename T>
void relocate_left(T* dst, T* src_beg, T* const src_end)
{
	if constexpr (is_trivially_relocatable_v<T>)
	{
		std::memmove(
			reinterpret_cast<byte*>(dst),
			reinterpret_cast<byte*>(src_beg),
			reinterpret_cast<byte*>(src_end) - reinterpret_cast<byte*>(src_beg));
	}
	else
	{
		for (; src_beg != src_end; ++dst, ++src_beg)
		{
			new (dst) T(vsm_move(*src_beg));
			src_beg->~T();
		}
	}
}

template<typename T>
void relocate_right(T* dst, T* const src_beg, T* src_end)
{
	if constexpr (is_trivially_relocatable_v<T>)
	{
		std::memmove(
			reinterpret_cast<byte*>(dst),
			reinterpret_cast<byte*>(src_beg),
			reinterpret_cast<byte*>(src_end) - reinterpret_cast<byte*>(src_beg));
	}
	else
	{
		for (dst += src_end - src_beg; src_end-- != src_beg;)
		{
			new (--dst) T(vsm_move(*src_end));
			src_end->~T();
		}
	}
}

template<bool>
struct front_pad_1;

template<>
struct front_pad_1<0>
{
	template<typename T, size_t Alignment>
	using type = T;
};

template<>
struct front_pad_1<1>
{
	template<size_t Size, size_t Alignment>
	struct padding
	{
		byte padding[((Size + Alignment - 1) & ~(Alignment - 1)) - Size];
	};

	template<typename T, size_t Alignment>
	struct type : padding<sizeof(T), Alignment>, T
	{
		using T::T;
	};
};

template<typename T, size_t Alignment>
using front_pad = front_pad_1<alignof(T) < Alignment>::template type<T, Alignment>;

struct core
{
	byte* beg;
	byte* mid;
	byte* end;
};

template<typename Allocator>
struct allocator_wrapper
{
	[[no_unique_address]] Allocator allocator;

	explicit vsm_always_inline allocator_wrapper(any_cvref_of<Allocator> auto&& allocator)
		: allocator(allocator)
	{
	}
};

template<typename Allocator>
struct allocator_layout
	: allocator_wrapper<Allocator>
	, front_pad<core, alignof(Allocator)>
{
	using allocator_wrapper<Allocator>::allocator_wrapper;
};

template<typename T, typename Allocator, size_t Capacity>
struct storage_layout : allocator_layout<Allocator>
{
	using allocator_layout<Allocator>::allocator_layout;
	byte mutable storage alignas(T)[sizeof(T) * Capacity];
};

template<typename T, typename Allocator>
struct storage_layout<T, Allocator, 0> : allocator_layout<Allocator>
{
	using allocator_layout<Allocator>::allocator_layout;
};

template<bool>
struct capacity_param
{
	constexpr capacity_param(size_t)
	{
	}
};

template<>
struct capacity_param<true>
{
	size_t capacity;

	constexpr capacity_param(size_t const capacity)
		: capacity(capacity)
	{
	}
};

template<bool HasLocalStorage>
bool vsm_always_inline is_dynamic(core const& self, byte const* const beg)
{
	if constexpr (HasLocalStorage)
	{
		return beg != reinterpret_cast<byte const*>(&self + 1);
	}
	else
	{
		return beg != nullptr;
	}
}

template<bool HasLocalStorage>
void construct(core& self, capacity_param<HasLocalStorage> const local_capacity)
{
	if constexpr (HasLocalStorage)
	{
		byte* const storage = reinterpret_cast<byte*>(&self + 1);

		self.beg = storage;
		self.mid = storage;
		self.end = storage + local_capacity.capacity;
	}
	else
	{
		self.beg = nullptr;
		self.mid = nullptr;
		self.end = nullptr;
	}
}

template<typename DestroyT, bool HasLocalStorage, typename Allocator>
void destroy(allocator_layout<Allocator>& self)
{
	byte* const beg = self.beg;
	byte* const mid = self.mid;

	std::destroy(
		reinterpret_cast<DestroyT*>(beg),
		reinterpret_cast<DestroyT*>(mid));

	if (is_dynamic<HasLocalStorage>(self, beg))
	{
		self.allocator.deallocate(allocation{ beg, static_cast<size_t>(self.end - beg) });
	}
}

template<size_t ElementSize, typename RelocateT, typename Allocator, bool HasLocalStorage>
struct operations
{
	using allocator_layout_type = allocator_layout<Allocator>;
	using capacity_param_type = capacity_param<HasLocalStorage>;

	static vsm_always_inline byte* allocate(allocator_layout_type& self, size_t& new_capacity)
	{
		allocation const allocation = self.allocator.allocate(new_capacity);
		new_capacity = allocation.size - allocation.size % ElementSize;
		return reinterpret_cast<byte*>(allocation.buffer);
	}

	static vsm_always_inline byte* local_allocate(allocator_layout_type& self, capacity_param_type const local_capacity, size_t& new_capacity)
	{
		if constexpr (HasLocalStorage)
		{
			if (new_capacity <= local_capacity.capacity)
			{
				new_capacity = local_capacity.capacity;
				return reinterpret_cast<byte*>(&self + 1);
			}
		}

		return allocate<ElementSize>(self, new_capacity);
	}

	static vsm_always_inline bool requires_allocation(capacity_param_type const local_capacity, size_t const size)
	{
		if constexpr (HasLocalStorage)
		{
			return size > local_capacity.capacity;
		}

		return true;
	}

	template<bool SrcHasLocalStorage>
	static void move_construct_adopt(
		allocator_layout_type& self,
		allocator_layout_type& src,
		capacity_param_type const local_capacity,
		capacity_param<SrcHasLocalStorage> const src_local_capacity)
	{
		byte* const src_beg = src.beg;
		byte* const src_mid = src.mid;

		if (src_beg == src_mid)
		{
			construct(self, local_capacity);
			return;
		}

		size_t const size = src_mid - src_beg;

		if (is_dynamic<SrcHasLocalStorage>(src, src_beg))
		{
			byte* const src_end = src.end;
			
			if (requires_allocation(local_capacity, src_end - src_beg))
			{
				self.beg = src_beg;
				self.mid = src_mid;
				self.end = src_end;

				construct(src, src_local_capacity);
				
				return;
			}
		}

		size_t new_capacity = size;
		byte* const beg = local_allocate(self, new_capacity);

		self.beg = beg;
		self.mid = beg + size;
		self.end = beg + new_capacity;
		
		src.mid = src_beg;

		relocate(
			reinterpret_cast<RelocateT*>(beg),
			reinterpret_cast<RelocateT*>(src_beg),
			reinterpret_cast<RelocateT*>(src_mid));
	}

	template<typename DestroyT, bool SrcHasLocalStorage, size_t ElementSize>
	static void move_assign(
		allocator_layout_type& self,
		allocator_layout_type& src,
		capacity_param_type const local_capacity,
		capacity_param<SrcHasLocalStorage> const src_local_capacity)
	{
		byte* const beg = self.beg;
		byte* const mid = self.mid;
		byte* const end = self.end;

		std::destroy(
			reinterpret_cast<DestroyT*>(beg),
			reinterpret_cast<DestroyT*>(mid));

		byte* const src_beg = src.beg;
		byte* const src_mid = src.mid;

		if (is_dynamic<SrcHasLocalStorage>(src, src_beg))
		{
			byte* const src_end = src.end;
			
			if (requires_allocation(local_capacity, src_end - src_beg))
			{
				if (is_dynamic<HasLocalStorage>(self, beg))
				{
					allocator.deallocate(allocation{ beg, end - beg });
				}

				self.beg = src_beg;
				self.mid = src_mid;
				self.end = src_end;

				construct(src, src_local_capacity);

				return;
			}
		}

		size_t const size = src_mid - src_beg;
		size_t const capacity = end - beg;

		if (size > capacity)
		{
			size_t new_capacity = allocators::resize(
				self.allocator, allocation{ beg, capacity }, size);

			if (new_capacity == 0)
			{
				allocator.deallocate(allocation);

				allocation const new_allocation = allocator.allocate(capacity);
				beg = reinterpret_cast<byte*>(new_allocation.buffer);
				new_capacity = new_allocation.size - new_allocation.size % ElementSize;
			}
		
			allocator.deallocate(allocation{ beg, capacity });
			beg = allocator.allocate(size);

			self.beg = beg;
			self.end = beg + size;
		}

		relocate(
			reinterpret_cast<RelocateT*>(beg),
			reinterpret_cast<RelocateT*>(src_beg),
			reinterpret_cast<RelocateT*>(src_mid));

		self.mid = beg + size;
		src.mid = src_beg;
	}

	static byte* expand(allocator_layout_type& self, size_t const min_capacity)
	{
		byte* const beg = self.beg;
		byte* const mid = self.mid;
		byte* const end = self.end;

		size_t const capacity = end - beg;
		size_t new_capacity = std::max(min_capacity, capacity * 2);

		byte* const new_beg = allocate(self, new_capacity);
		byte* const new_end = new_beg + new_capacity;

		if (beg != mid)
		{
			relocate(
				reinterpret_cast<RelocateT*>(new_beg),
				reinterpret_cast<RelocateT*>(beg),
				reinterpret_cast<RelocateT*>(mid));
		}

		byte* empty = nullptr;

		if constexpr (HasLocalStorage)
		{
			empty = reinterpret_cast<byte*>(&self + 1);
		}

		if (beg != empty)
		{
			self.allocator.deallocate(allocation{ beg, capacity });
		}

		self.beg = new_beg;
		self.mid = new_beg + (mid - beg);
		self.end = new_end;

		return new_beg;
	}

	static vsm_never_inline byte* push_back_slow(allocator_layout_type& self, size_t const size)
	{
		size_t const cur_size = self.mid - self.beg;
		size_t const new_size = cur_size + size;

		byte* const new_beg = expand(self, new_size);

		self.mid = new_beg + new_size;
		return new_beg + cur_size;
	}

	static byte* push_back(allocator_layout_type& self, size_t const size)
	{
		byte* const mid = self.mid;
		byte* const new_mid = mid + size;

		if (vsm_unlikely(new_mid > self.end))
		{
			return push_back_slow(self, size);
		}

		self.mid = new_mid;
		return mid;
	}

	template<typename CopyT>
	static byte* push_back_range(allocator_layout_type& self, CopyT const* const first, CopyT const* const last)
	{
		byte* const slots = push_back(self,
			reinterpret_cast<byte const*>(last) -
				reinterpret_cast<byte const*>(first), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<CopyT*>(slots));
	}

	template<typename T, typename InputIt>
	static byte* push_back_range(allocator_layout_type& self, InputIt const first, InputIt const last)
	{
		byte* const slots = push_back(self,
			std::distance(first, last) * sizeof(T), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<T*>(slots));
	}
	
	static vsm_never_inline byte* insert_slow(allocator_layout_type& self, byte* const pos, size_t const size)
	{
		byte* const beg = self.beg;

		size_t const offset = pos - beg;
		size_t const cur_size = self.mid - beg;
		size_t const new_size = cur_size + size;

		byte* const new_beg = expand(self, new_size);

		byte* const new_pos = new_beg + offset;

		relocate_right(
			reinterpret_cast<RelocateT*>(new_pos + size),
			reinterpret_cast<RelocateT*>(new_pos),
			reinterpret_cast<RelocateT*>(new_beg + cur_size));

		self.mid = new_beg + new_size;
		return new_pos;
	}

	static byte* insert(allocator_layout_type& self, byte* const pos, size_t const size)
	{
		byte* const mid = self.mid;
		byte* const new_mid = mid + size;

		if (vsm_unlikely(new_mid > self.end))
		{
			return insert_slow(self, pos, size);
		}

		if (pos != mid)
		{
			relocate_right(
				reinterpret_cast<RelocateT*>(pos + size),
				reinterpret_cast<RelocateT*>(pos),
				reinterpret_cast<RelocateT*>(mid));
		}

		self.mid = new_mid;
		return pos;
	}

	template<typename CopyT>
	static byte* insert_range(allocator_layout_type& self, byte* const pos, CopyT const* const first, CopyT const* const last)
	{
		byte* const slots = insert(self, pos,
			reinterpret_cast<byte const*>(last) -
				reinterpret_cast<byte const*>(first), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<CopyT*>(slots));

		return slots;
	}

	template<typename T, typename InputIt>
	static byte* insert_range(allocator_layout_type& self, byte* const pos, InputIt const first, InputIt const last)
	{
		byte* const slots = insert(self, pos, std::distance(first, last) * sizeof(T));
		std::uninitialized_copy(first, last, reinterpret_cast<T*>(slots));
		return slots;
	}

	template<bool Construct, typename ConstructT, typename DestroyT>
	static byte* resize(allocator_layout_type& self, size_t const new_size)
	{
		byte* const beg = self.beg;
		byte* const mid = self.mid;
		byte* const new_mid = beg + new_size;

		if (vsm_likely(new_mid > mid))
		{
			if (new_mid > self.end)
			{
				byte* const new_beg = expand(self, new_size, allocator);

				mid = new_beg + (mid - beg);
				new_mid = new_beg + new_size;
			}

			self.mid = new_mid;

			if constexpr (Construct)
			{
				std::uninitialized_value_construct(
					reinterpret_cast<ConstructT*>(mid),
					reinterpret_cast<ConstructT*>(new_mid));
			}

			return mid;
		}
		else if (vsm_likely(new_mid < mid))
		{
			self.mid = new_mid;

			std::destroy(
				reinterpret_cast<DestroyT*>(new_mid),
				reinterpret_cast<DestroyT*>(mid));

			return new_mid;
		}

		return mid;
	}

	static void reserve(allocator_layout_type& self, size_t const min_capacity)
	{
		if (static_cast<size_t>(self.end - self.beg) < min_capacity)
		{
			expand(self, min_capacity, allocator);
		}
	}

	static void shrink_to_fit(allocator_layout_type& self, capacity_param_type const local_capacity)
	{
		byte* const mid = self.mid;
		byte* const end = self.end;

		if (mid == end)
		{
			return;
		}

		byte* const beg = self.beg;
		size_t const capacity = end - beg;
		size_t const new_capacity = mid - beg;

		byte* new_beg;
		if constexpr (HasLocalStorage)
		{
			if (capacity <= local_capacity.capacity)
			{
				return;
			}

			if (new_capacity <= local_capacity.capacity)
			{
				new_beg = reinterpret_cast<byte*>(self + 1);
			}
			else
			{
				new_beg = allocator.allocate(new_capacity);
			}
		}
		else
		{
			if (new_capacity > 0)
			{
				new_beg = allocator.allocate(new_capacity);
			}
			else
			{
				allocator.deallocate(allocation{ beg, capacity });

				self.beg = nullptr;
				self.mid = nullptr;
				self.end = nullptr;

				return;
			}
		}

		relocate(
			reinterpret_cast<RelocateT*>(new_beg),
			reinterpret_cast<RelocateT*>(beg),
			reinterpret_cast<RelocateT*>(mid));

		allocator.deallocate(allocation{ beg, capacity });

		byte* const new_end = new_beg + new_capacity;

		self.beg = new_beg;
		self.mid = new_end;
		self.end = new_end;
	}
};

template<typename DestroyT>
void clear(core& self)
{
	byte* const beg = self.beg;
	byte* const mid = self.mid;
	self.mid = beg;

	std::destroy(
		reinterpret_cast<DestroyT*>(beg),
		reinterpret_cast<DestroyT*>(mid));
}

template<typename RelocateT, typename DestroyT>
byte* erase(core& self, byte* const first, byte* const last)
{
	byte* const mid = self.mid;
	byte* const new_mid = mid - (last - first);

	std::destroy(
		reinterpret_cast<DestroyT*>(first),
		reinterpret_cast<DestroyT*>(last));

	if (last != mid)
	{
		relocate_left(
			reinterpret_cast<RelocateT*>(first),
			reinterpret_cast<RelocateT*>(last),
			reinterpret_cast<RelocateT*>(mid));
	}

	self.mid = new_mid;

	return first;
}

template<typename RelocateT, typename DestroyT>
byte* erase_unstable(core& self, byte* const first, byte* const last)
{
	byte* const mid = self.mid;
	byte* const new_mid = mid - (last - first);

	std::destroy(
		reinterpret_cast<DestroyT*>(first),
		reinterpret_cast<DestroyT*>(last));

	if (last != mid)
	{
		relocate(
			reinterpret_cast<RelocateT*>(first),
			reinterpret_cast<RelocateT*>(new_mid),
			reinterpret_cast<RelocateT*>(mid));
	}

	self.mid = new_mid;

	return first;
}

template<typename DestroyT>
void pop_back(core& self, size_t const size)
{
	byte* const mid = self.mid;
	byte* const new_mid = mid - size;
	self.mid = new_mid;

	reinterpret_cast<DestroyT*>(new_mid)->~DestroyT();
}

template<typename DestroyT>
void pop_back_n(core& self, size_t const size)
{
	byte* const mid = self.mid;
	byte* const new_mid = mid - size;
	self.mid = new_mid;

	std::destroy(
		reinterpret_cast<DestroyT*>(new_mid),
		reinterpret_cast<DestroyT*>(mid));
}

template<typename T>
bool equal(core const& lhs, core const& rhs)
{
	byte* const lhs_beg = lhs.beg;
	byte* const lhs_mid = lhs.mid;
	byte* const rhs_beg = rhs.beg;
	byte* const rhs_mid = rhs.mid;

	if (lhs_mid - lhs_beg != rhs_mid - rhs_beg)
	{
		return false;
	}
	
	return std::equal(
		reinterpret_cast<T const*>(lhs_beg),
		reinterpret_cast<T const*>(lhs_mid),
		reinterpret_cast<T const*>(rhs_beg),
		reinterpret_cast<T const*>(rhs_mid));
}

template<typename T>
auto compare(core const& lhs, core const& rhs)
{
	return std::lexicographical_compare_three_way(
		reinterpret_cast<T const*>(lhs.beg),
		reinterpret_cast<T const*>(lhs.mid),
		reinterpret_cast<T const*>(rhs.beg),
		reinterpret_cast<T const*>(rhs.mid));
}

template<typename T, typename Allocator, size_t Capacity>
class vector
{
	static constexpr bool has_local_storage = Capacity > 0;

#pragma push_macro("storage_capacity")
#define storage_capacity (Capacity * sizeof(T))

#pragma push_macro("construct_type")
#define construct_type std::conditional_t<std::is_trivially_constructible_v<T>, byte, T>

#pragma push_macro("destroy_type")
#define destroy_type std::conditional_t<std::is_trivially_destructible_v<T>, byte, T>

#pragma push_macro("relocate_type")
#define relocate_type std::conditional_t<is_trivially_relocatable_v<T>, byte, T>

#pragma push_macro("copy_type")
#define copy_type std::conditional_t<std::is_trivially_copyable_v<T>, byte, T>

#pragma push_macro("operations")
#define operations operations<sizeof(T), relocate_type, Allocator, has_local_storage>

	storage_layout<T, Allocator, Capacity> m;

public:
	using value_type                                        = T;
	using allocator_type                                    = Allocator;
	using size_type                                         = size_t;
	using difference_type                                   = ptrdiff_t;
	using reference                                         = T&;
	using const_reference                                   = T const&;
	using pointer                                           = T*;
	using const_pointer                                     = T const*;
	using iterator                                          = T*;
	using const_iterator                                    = T const*;
	using reverse_iterator =                                std::reverse_iterator<T*>;
	using const_reverse_iterator =                          std::reverse_iterator<T const*>;

	vsm_always_inline vector()
	{
		construct<has_local_storage>(m, storage_capacity);
	}
	
	explicit vsm_always_inline vector(Allocator const& allocator)
		: m(allocator)
	{
		construct<has_local_storage>(m, storage_capacity);
	}

	vsm_always_inline vector(vector&& other)
		requires allocators::is_propagatable_v<Allocator>
		: m(vsm_move(other.m))
	{
		operations::template move_construct<has_local_storage>(
			m,
			other.m,
			storage_capacity,
			storage_capacity);
	}

	vsm_always_inline vector& operator=(vector&& other) &
		requires allocators::is_propagatable_v<Allocator>
	{
		operations::template move_assign_allocator<destroy_type>(
			m,
			other.m,
			storage_capacity);

		return *this;
	}

	vsm_always_inline ~vector()
	{
		destroy<destroy_type, has_local_storage>(m);
	}

	vsm_always_inline Allocator get_allocator() const
	{
		return m.allocator;
	}

	vsm_always_inline T& operator[](size_t const index)
	{
		vsm_assert(index < size());
		return reinterpret_cast<T*>(m.beg)[index];
	}

	vsm_always_inline T const& operator[](size_t const index) const
	{
		vsm_assert(index < size());
		return reinterpret_cast<T const*>(m.beg)[index];
	}

	vsm_always_inline T& front()
	{
		vsm_assert(!empty());
		return *reinterpret_cast<T*>(m.beg);
	}

	vsm_always_inline T const& front() const
	{
		vsm_assert(!empty());
		return *reinterpret_cast<T const*>(m.beg);
	}

	vsm_always_inline T& back()
	{
		vsm_assert(!empty());
		return reinterpret_cast<T*>(m.mid)[-1];
	}

	vsm_always_inline T const& back() const
	{
		vsm_assert(!empty());
		return reinterpret_cast<T const*>(m.mid)[-1];
	}

	vsm_always_inline T* data()
	{
		return reinterpret_cast<T*>(m.beg);
	}

	vsm_always_inline T const* data() const
	{
		return reinterpret_cast<T const*>(m.beg);
	}

	vsm_always_inline T* begin()
	{
		return reinterpret_cast<T*>(m.beg);
	}

	vsm_always_inline T const* begin() const
	{
		return reinterpret_cast<T const*>(m.beg);
	}

	vsm_always_inline T const* cbegin() const
	{
		return reinterpret_cast<T const*>(m.beg);
	}

	vsm_always_inline T* end()
	{
		return reinterpret_cast<T*>(m.mid);
	}

	vsm_always_inline T const* end() const
	{
		return reinterpret_cast<T const*>(m.mid);
	}

	vsm_always_inline T const* cend() const
	{
		return reinterpret_cast<T const*>(m.mid);
	}

	vsm_always_inline std::reverse_iterator<T*> rbegin()
	{
		return std::make_reverse_iterator(end());
	}

	vsm_always_inline std::reverse_iterator<T const*> rbegin() const
	{
		return std::make_reverse_iterator(end());
	}

	vsm_always_inline std::reverse_iterator<T const*> crbegin() const
	{
		return std::make_reverse_iterator(cend());
	}

	vsm_always_inline std::reverse_iterator<T*> rend()
	{
		return std::make_reverse_iterator(begin());
	}

	vsm_always_inline std::reverse_iterator<T const*> rend() const
	{
		return std::make_reverse_iterator(begin());
	}

	vsm_always_inline std::reverse_iterator<T const*> crend() const
	{
		return std::make_reverse_iterator(cbegin());
	}

	vsm_always_inline bool empty() const
	{
		return m.beg == m.mid;
	}

	vsm_always_inline size_t size() const
	{
		return reinterpret_cast<T const*>(m.mid) - reinterpret_cast<T const*>(m.beg);
	}

	vsm_always_inline size_t max_size() const
	{
		return std::numeric_limits<size_t>::max() / sizeof(T);
	}

	vsm_always_inline void reserve(size_t const min_capacity)
	{
		operations::reserve(m, min_capacity * sizeof(T));
	}

	vsm_always_inline size_t capacity() const
	{
		return reinterpret_cast<T const*>(m.end) - reinterpret_cast<T const*>(m.beg);
	}

	vsm_always_inline void shrink_to_fit()
	{
		operations::shrink_to_fit(m, storage_capacity);
	}

	vsm_always_inline void clear()
	{
		vector_::clear<destroy_type>(m);
	}

	void _assign(T const* const beg, size_t const count)
	{
		vector_::clear<destroy_type>(m);
		byte* const slots = operations::push_back(m, count * sizeof(T));
		std::uninitialized_copy_n(beg, count, reinterpret_cast<T*>(slots));
	}

	vsm_always_inline T* insert(T const* const pos, T const& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		byte* const slot = operations::insert(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			sizeof(T));

		return new (slot) T(value);
	}

	vsm_always_inline T* insert(T const* const pos, T&& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		byte* const slot = operations::insert(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			sizeof(T));

		return new (slot) T(vsm_move(value));
	}

	T* insert(T const* const pos, size_t const count, T const& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		size_t const size = count * sizeof(T);

		byte* const slots = operations::insert(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			size);

		std::uninitialized_fill(
			reinterpret_cast<T*>(slots),
			reinterpret_cast<T*>(slots + size), value);

		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* insert(T const* const pos, T const* const first, T const* const last)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		byte* const slots = operations::template insert_range<copy_type>(
				m,
				reinterpret_cast<byte*>(const_cast<T*>(pos)),
				reinterpret_cast<copy_type const*>(first),
				reinterpret_cast<copy_type const*>(last));

		return reinterpret_cast<T*>(slots);
	}

	template<typename InputIt>
	vsm_always_inline T* insert(T const* const pos, InputIt const first, InputIt const last)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		byte* const slots = operations::template insert_range<T, InputIt>(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			first,
			last);

		return reinterpret_cast<T*>(slots);
	}

	template<typename... Args>
	vsm_always_inline T* emplace(T const* const pos, Args&&... args)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		byte* const slot = operations::insert(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			sizeof(T));

		return new (slot) T(vsm_forward(args)...);
	}

	vsm_always_inline T* erase(T const* const pos)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos < reinterpret_cast<T const*>(m.mid));

		byte* const next = vector_::erase<relocate_type, destroy_type>(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			reinterpret_cast<byte*>(const_cast<T*>(pos) + 1));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline T* _erase_unstable(T const* const pos)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos < reinterpret_cast<T const*>(m.mid));

		byte* const next = vector_::erase_unstable<relocate_type, destroy_type>(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			reinterpret_cast<byte*>(const_cast<T*>(pos) + 1));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline T* erase(T const* const first, T const* const last)
	{
		vsm_assert(first <= last &&
			first >= reinterpret_cast<T const*>(m.beg) &&
			last <= reinterpret_cast<T const*>(m.mid));

		byte* const next = vector_::erase<relocate_type, destroy_type>(
			m,
			reinterpret_cast<byte*>(const_cast<T*>(first)),
			reinterpret_cast<byte*>(const_cast<T*>(last)));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline T& push_back(T&& value)
	{
		byte* const slot = operations::push_back(m, sizeof(T));
		return *new (slot) T(vsm_move(value));
	}

	vsm_always_inline T& push_back(T const& value)
	{
		byte* const slot = operations::push_back(m, sizeof(T));
		return *new (slot) T(value);
	}

	template<typename InputIt>
	vsm_always_inline T* _push_back_range(InputIt const beg, InputIt const end)
	{
		byte* const slots = operations::push_back(m, std::distance(beg, end) * sizeof(T));
		std::uninitialized_copy(beg, end, reinterpret_cast<T*>(slots));
		return reinterpret_cast<T*>(slots);
	}

	template<typename InputIt>
	vsm_always_inline T* _push_back_n(InputIt const beg, size_t const count)
	{
		byte* const slots = operations::push_back(m, count * sizeof(T));
		std::uninitialized_copy_n(beg, count, reinterpret_cast<T*>(slots));
		return reinterpret_cast<T*>(slots);
	}

	template<typename U = T>
	vsm_always_inline T* _push_back_fill(size_t const count, U const& value)
	{
		byte* const slots = operations::push_back(m, count * sizeof(T));
		std::uninitialized_fill_n(reinterpret_cast<T*>(slots), count, value);
		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* _push_back_default(size_t const count)
	{
		byte* const slots = operations::push_back(m, count * sizeof(T));
		std::uninitialized_default_construct_n(reinterpret_cast<T*>(slots), count);
		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* _push_back_uninitialized(size_t const count)
	{
		byte* const slots = operations::push_back(m, count * sizeof(T));
		return reinterpret_cast<T*>(slots);
	}

	template<typename... Args>
	vsm_always_inline T& emplace_back(Args&&... args)
	{
		byte* const slot = operations::push_back(m, sizeof(T));
		return *new (slot) T(vsm_forward(args)...);
	}

	vsm_always_inline void pop_back()
	{
		vsm_assert(!empty());
		vector_::pop_back<destroy_type>(m, sizeof(T));
	}

	vsm_always_inline void _pop_back_n(size_t const count)
	{
		vsm_assert(count <= size());
		vector_::pop_back_n<destroy_type>(m, count * sizeof(T));
	}

	vsm_always_inline void _pop_back_uninitialized(size_t const count)
	{
		vsm_assert(count <= size());
		vector_::pop_back<byte>(m, count * sizeof(T));
	}

	vsm_always_inline T _pop_back_value()
	{
		vsm_assert(!empty());
		T value = vsm_move(reinterpret_cast<T*>(m.mid)[-1]);
		vector_::pop_back<destroy_type>(m, sizeof(T));
		return value;
	}

	vsm_always_inline void resize(size_t const count)
	{
		operations::template resize< true, construct_type, destroy_type>(
			m,
			count * sizeof(T));
	}

	vsm_always_inline void _resize_uninitialized(size_t const count)
	{
		operations::template resize<false, byte, byte>(
			m,
			count * sizeof(T));
	}

	void resize(size_t const count, T const& value)
	{
		byte* const pos = operations::template resize<false, void, destroy_type>(
			m,
			count * sizeof(T));

		std::uninitialized_fill(reinterpret_cast<T*>(pos), reinterpret_cast<T*>(m.mid), value);
	}

	T* _release_storage()
	{
		vsm_assert(is_dynamic<has_local_storage>(m, m.beg));
		T* const buffer = reinterpret_cast<T*>(m.beg);
		construct<has_local_storage>(m, storage_capacity);
		return buffer;
	}

	void _acquire_storage(T* const buffer, size_t const size, size_t const capacity)
	{
		vsm_assert(size > Capacity);
		destroy<destroy_type, has_local_storage>(m);
		byte* const beg = reinterpret_cast<byte*>(buffer);
		m.beg = beg;
		m.mid = beg + size * sizeof(T);
		m.end = beg + capacity * sizeof(T);
	}

	template<typename T, typename Allocator, size_t Capacity>
	friend vsm_always_inline bool operator==(vector const& lhs, vector const& rhs)
	{
		return equal<T>(lhs.m, rhs.m);
	}

	template<typename T, typename Allocator, size_t Capacity>
	friend vsm_always_inline auto operator<=>(vector const& lhs, vector const& rhs)
	{
		return compare<T>(lhs.m, rhs.m);
	}

#pragma pop_macro("storage_capacity")
#pragma pop_macro("construct_type")
#pragma pop_macro("destroy_type")
#pragma pop_macro("relocate_type")
#pragma pop_macro("copy_type")
#pragma pop_macro("operations")
};

} // namespace detail::vector_

template<typename T, typename Allocator = default_allocator>
using vector = detail::vector_::vector<T, Allocator, 0>;

template<typename T, size_t Capacity, typename Allocator = default_allocator>
using small_vector = detail::vector_::vector<T, Allocator, Capacity>;

template<typename T, typename A, size_t C, typename U>
typename detail::vector_::vector<T, A, C>::size_type erase(detail::vector_::vector<T, A, C>& v, U const& value)
{
	auto it = std::remove(v.begin(), v.end(), value);
	typename detail::vector_::vector<T, A, C>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

template<typename T, typename A, size_t C, typename Pred>
typename detail::vector_::vector<T, A, C>::size_type erase_if(detail::vector_::vector<T, A, C>& v, Pred&& pred)
{
	auto it = std::remove_if(v.begin(), v.end(), vsm_forward(pred));
	typename detail::vector_::vector<T, A, C>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

template<typename T, typename A, size_t C, typename U>
typename detail::vector_::vector<T, A, C>::size_type erase_unstable(detail::vector_::vector<T, A, C>& v, U const& value)
{
	auto it = remove_unstable(v.begin(), v.end(), value);
	typename detail::vector_::vector<T, A, C>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

template<typename T, typename A, size_t C, typename Pred>
typename detail::vector_::vector<T, A, C>::size_type erase_if_unstable(detail::vector_::vector<T, A, C>& v, Pred&& pred)
{
	auto it = remove_if_unstable(v.begin(), v.end(), vsm_forward(pred));
	typename detail::vector_::vector<T, A, C>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

} // namespace vsm

vsm_msvc_warning(pop)
