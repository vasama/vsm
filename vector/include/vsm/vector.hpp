#pragma once

#include <vsm/algorithm/remove_unstable.hpp>
#include <vsm/allocator.hpp>
#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/platform.h>
#include <vsm/relocate.hpp>
#include <vsm/standard.hpp>
#include <vsm/standard/stdexcept.hpp>
#include <vsm/type_traits.hpp>
#include <vsm/utility.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <new>
#include <ranges>

#include <cstddef>
#include <cstring>

vsm_msvc_warning(push)
vsm_msvc_warning(disable: 4102) // unreferenced label

namespace vsm {
namespace detail {

template<bool>
struct _front_pad;

template<>
struct _front_pad<0>
{
	template<typename T, size_t Alignment>
	using type = T;
};

template<>
struct _front_pad<1>
{
	template<size_t Size, size_t Alignment>
	struct base
	{
		unsigned char _[((Size + Alignment - 1) & ~(Alignment - 1)) - Size];
	};

	template<typename T, size_t Alignment>
	struct type : base<sizeof(T), Alignment>, T
	{
		using T::T;
	};
};

template<typename T, size_t Alignment>
using front_pad = _front_pad<alignof(T) < Alignment>::template type<T, Alignment>;


struct init_none_t;
struct init_zero_t;

struct _vector
{
	std::byte* beg;
	std::byte* mid;
	std::byte* end;
};

template<typename Allocator>
struct _wrap_allocator
{
	vsm_no_unique_address Allocator allocator;

	_wrap_allocator() = default;

	explicit vsm_always_inline _wrap_allocator(any_cvref_of<Allocator> auto&& allocator)
		: allocator(allocator)
	{
	}
};

template<typename Allocator>
struct _vector_allocator
	: _wrap_allocator<Allocator>
	, front_pad<_vector, alignof(Allocator)>
{
	using _wrap_allocator<Allocator>::_wrap_allocator;
};

template<typename T, typename Allocator, size_t Capacity>
struct _vector_storage : _vector_allocator<Allocator>
{
	using _vector_allocator<Allocator>::_vector_allocator;
	std::byte mutable storage alignas(T)[sizeof(T) * Capacity];
};

template<typename T, typename Allocator>
struct _vector_storage<T, Allocator, 0> : _vector_allocator<Allocator>
{
	using _vector_allocator<Allocator>::_vector_allocator;
};

template<bool>
struct _vector_capacity
{
	static constexpr size_t capacity = 0;

	constexpr _vector_capacity(size_t)
	{
	}
};

template<>
struct _vector_capacity<true>
{
	size_t capacity;

	constexpr _vector_capacity(size_t const capacity)
		: capacity(capacity)
	{
	}
};

template<bool Local>
inline vsm_always_inline bool _vector_dynamic(_vector const& self, std::byte const* const beg)
{
	if constexpr (Local)
	{
		return beg != reinterpret_cast<std::byte const*>(&self + 1);
	}
	else
	{
		return beg != nullptr;
	}
}

template<bool Local>
inline vsm_always_inline bool _vector_requires_dynamic(
	_vector_capacity<Local> const local,
	size_t const size)
{
	if constexpr (Local)
	{
		return size > local.capacity;
	}
	else
	{
		return true;
	}
}

template<bool Local>
void _vector_construct(_vector& self, _vector_capacity<Local> const local)
{
	if constexpr (Local)
	{
		std::byte* const storage = reinterpret_cast<std::byte*>(&self + 1);

		self.beg = storage;
		self.mid = storage;
		self.end = storage + local.capacity;
	}
	else
	{
		self.beg = nullptr;
		self.mid = nullptr;
		self.end = nullptr;
	}
}

template<typename DtorT, bool Local, typename A>
void _vector_destroy(_vector_allocator<A>& self)
{
	std::byte* const beg = self.beg;
	std::byte* const mid = self.mid;

	if constexpr (!std::is_same_v<DtorT, init_none_t>)
	{
		std::destroy(
			reinterpret_cast<DtorT*>(beg),
			reinterpret_cast<DtorT*>(mid));
	}

	if (_vector_dynamic<Local>(self, beg))
	{
		self.allocator.deallocate(allocation
		{
			beg,
			static_cast<size_t>(self.end - beg)
		});
	}
}

template<size_t Size, typename A>
std::byte* _vector_allocate(_vector_allocator<A>& self, size_t& new_capacity)
{
	allocation const allocation = self.allocator.allocate(new_capacity);
	new_capacity = allocation.size - allocation.size % Size;
	return reinterpret_cast<std::byte*>(allocation.buffer);
}

template<size_t Size, typename A, bool Local>
inline vsm_always_inline std::byte* _vector_allocate_local(
	_vector_allocator<A>& self,
	_vector_capacity<Local> const local,
	size_t& new_capacity)
{
	if constexpr (Local)
	{
		if (new_capacity <= local.capacity)
		{
			new_capacity = local.capacity;
			return reinterpret_cast<std::byte*>(&self + 1);
		}
	}

	return _vector_allocate<Size>(self, new_capacity);
}

template<typename A, bool Local>
inline vsm_always_inline std::byte* _vector_local_allocate(
	_vector_allocator<A>& self,
	_vector_capacity<Local> const local,
	size_t& new_capacity)
{
	if constexpr (Local)
	{
		if (new_capacity <= local.capacity)
		{
			new_capacity = local.capacity;
			return reinterpret_cast<std::byte*>(&self + 1);
		}
	}

	return _vector_allocate(self, new_capacity);
}

template<typename ReloT, size_t Size, typename A, bool Local, bool SrcLocal>
void _vector_move_construct(
	_vector_allocator<A>& self,
	_vector_allocator<A>& src,
	_vector_capacity<Local> const local,
	_vector_capacity<SrcLocal> const src_local)
{
	std::byte* const src_beg = src.beg;
	std::byte* const src_mid = src.mid;

	if (src_beg == src_mid)
	{
		_vector_construct(self, local);
		return;
	}

	size_t const size = static_cast<size_t>(src_mid - src_beg);

	if (_vector_dynamic<SrcLocal>(src, src_beg))
	{
		std::byte* const src_end = src.end;

		if (_vector_requires_dynamic(local, static_cast<size_t>(src_end - src_beg)))
		{
			self.beg = src_beg;
			self.mid = src_mid;
			self.end = src_end;

			_vector_construct(src, src_local);

			return;
		}
	}

	size_t new_capacity = size;
	std::byte* const beg = _vector_allocate_local<Size>(self, local, new_capacity);

	self.beg = beg;
	self.mid = beg + size;
	self.end = beg + new_capacity;

	src.mid = src_beg;

	uninitialized_relocate(
		reinterpret_cast<ReloT*>(src_beg),
		reinterpret_cast<ReloT*>(src_mid),
		reinterpret_cast<ReloT*>(beg));
}

template<typename ReloT, typename DtorT, size_t Size, typename A, bool Local, bool SrcLocal>
void _vector_move_assign(
	_vector_allocator<A>& self,
	_vector_allocator<A>& src,
	_vector_capacity<Local> const local,
	_vector_capacity<SrcLocal> const src_local)
{
	std::byte* const beg = self.beg;
	std::byte* const mid = self.mid;
	std::byte* const end = self.end;

	if constexpr (!std::is_same_v<DtorT, init_none_t>)
	{
		std::destroy(
			reinterpret_cast<DtorT*>(beg),
			reinterpret_cast<DtorT*>(mid));
	}

	std::byte* const src_beg = src.beg;
	std::byte* const src_mid = src.mid;

	if (_vector_dynamic<SrcLocal>(src, src_beg))
	{
		std::byte* const src_end = src.end;

		if (static_cast<size_t>(src_end - src_beg) > src_local.capacity)
		{
			if (_vector_dynamic<Local>(self, beg))
			{
				self.allocator.deallocate(allocation(
					beg,
					static_cast<size_t>(end - beg)));
			}

			self.beg = src_beg;
			self.mid = src_mid;
			self.end = src_end;

			_vector_construct(src, src_local);

			return;
		}
	}

	size_t const size = static_cast<size_t>(src_mid - src_beg);
	size_t const capacity = static_cast<size_t>(end - beg);

	if (size > capacity)
	{
		auto const old_allocation = allocation(beg, capacity);

		size_t new_allocation_size = allocators::resize(
			self.allocator,
			old_allocation,
			size);

		if (new_allocation_size == 0)
		{
			self.allocator.deallocate(old_allocation);
			allocation const new_allocation = self.allocator.allocate(capacity);
			self.beg = reinterpret_cast<std::byte*>(new_allocation.buffer);
			new_allocation_size = new_allocation.size;
		}

		self.end = self.beg + (new_allocation_size - new_allocation_size % Size);
	}

	uninitialized_relocate(
		reinterpret_cast<ReloT*>(src_beg),
		reinterpret_cast<ReloT*>(src_mid),
		reinterpret_cast<ReloT*>(self.beg));

	self.mid = self.beg + size;
	src.mid = src_beg;
}

template<typename ReloT>
void _vector_swap2(_vector& small, _vector& large)
{
	_vector const large_copy = large;

	large.beg = reinterpret_cast<std::byte*>(&large + 1);
	large.mid = large.beg + static_cast<size_t>(small.mid - small.beg);
	large.end = large.beg + static_cast<size_t>(small.end - small.beg);

	uninitialized_relocate(
		reinterpret_cast<ReloT*>(small.beg),
		reinterpret_cast<ReloT*>(small.mid),
		reinterpret_cast<ReloT*>(large.beg));

	small = large_copy;
}

template<typename ReloT, bool Local, typename A>
void _vector_swap(_vector_allocator<A>& lhs, _vector_allocator<A>& rhs)
{
	static_assert(std::is_nothrow_swappable_v<A>);

	if constexpr (Local)
	{
		static_assert(std::is_nothrow_swappable_v<ReloT>);
		static_assert(is_nothrow_relocatable_v<ReloT>);

		bool const lhs_small = !_vector_dynamic<true>(lhs, lhs.beg);
		bool const rhs_small = !_vector_dynamic<true>(rhs, rhs.beg);

		if (lhs_small && rhs_small)
		{
			ReloT* lhs_pos = reinterpret_cast<ReloT*>(lhs.beg);
			ReloT* rhs_pos = reinterpret_cast<ReloT*>(rhs.beg);

			ReloT* const lhs_mid = reinterpret_cast<ReloT*>(lhs.mid);
			ReloT* const rhs_mid = reinterpret_cast<ReloT*>(rhs.mid);

			while (lhs_pos != lhs_mid && rhs_pos != rhs_mid)
			{
				using std::swap;
				swap(*lhs_pos++, *rhs_pos++);
			}

			/**/ if (lhs_pos != lhs_mid)
			{
				uninitialized_relocate(
					lhs_pos,
					lhs_mid,
					rhs_pos);
			}
			else if (rhs_pos != rhs_mid)
			{
				uninitialized_relocate(
					rhs_pos,
					rhs_mid,
					lhs_pos);
			}

			size_t const lhs_size = static_cast<size_t>(lhs.mid - lhs.beg);
			size_t const rhs_size = static_cast<size_t>(rhs.mid - rhs.beg);

			lhs.mid = lhs.beg + rhs_size;
			rhs.mid = rhs.beg + lhs_size;
		}
		else if (lhs_small)
		{
			_vector_swap2<ReloT>(lhs, rhs);
		}
		else if (rhs_small)
		{
			_vector_swap2<ReloT>(rhs, lhs);
		}
		else
		{
			std::swap(lhs.beg, rhs.beg);
			std::swap(lhs.mid, rhs.mid);
			std::swap(lhs.end, rhs.end);
		}
	}
	else
	{
		std::swap(lhs.beg, rhs.beg);
		std::swap(lhs.mid, rhs.mid);
		std::swap(lhs.end, rhs.end);
	}

	using std::swap;
	swap(lhs.allocator, rhs.allocator);
}

template<typename ReloT, bool Local, size_t Size, typename A>
std::byte* _vector_expand_new(
	_vector_allocator<A>& self,
	std::byte* const pos,
	size_t const min_size)
{
	static_assert(is_nothrow_relocatable_v<ReloT>);

	std::byte* const beg = self.beg;
	std::byte* const mid = self.mid;
	std::byte* const end = self.end;

	size_t const capacity = static_cast<size_t>(end - beg);
	size_t new_capacity = std::max(capacity + min_size, capacity * 2);

	std::byte* const new_beg = _vector_allocate<Size>(self, new_capacity);
	std::byte* const new_pos = new_beg + (pos - beg);

	if (beg != mid)
	{
		vsm_assert_slow(pos <= mid);

		uninitialized_relocate(
			reinterpret_cast<ReloT*>(beg),
			reinterpret_cast<ReloT*>(pos),
			reinterpret_cast<ReloT*>(new_beg));

		uninitialized_relocate(
			reinterpret_cast<ReloT*>(pos),
			reinterpret_cast<ReloT*>(mid),
			reinterpret_cast<ReloT*>(new_pos + min_size));
	}

	if (_vector_dynamic<Local>(self, beg))
	{
		self.allocator.deallocate(allocation{ beg, capacity });
	}

	self.beg = new_beg;
	self.mid = new_beg + (mid - beg);
	self.end = new_beg + new_capacity;

	return new_pos;
}

template<typename ReloT, bool Local, size_t Size, typename A>
vsm_never_inline std::byte* _vector_push2(_vector_allocator<A>& self, size_t const size)
{
	std::byte* const new_pos = _vector_expand_new<ReloT, Local, Size>(self, self.mid, size);

	vsm_assert_slow(new_pos == self.mid);
	self.mid = new_pos + size;

	return new_pos;
}

template<typename ReloT, bool Local, size_t Size, typename A>
std::byte* _vector_push(_vector_allocator<A>& self, size_t const size)
{
	std::byte* const mid = self.mid;

	if (vsm_unlikely(size > static_cast<size_t>(self.end - mid)))
	{
		return _vector_push2<ReloT, Local, Size>(self, size);
	}

	self.mid = mid + size;
	return mid;
}

template<typename ReloT, bool Local, size_t Size, typename A>
vsm_never_inline std::byte* _vector_insert2(
	_vector_allocator<A>& self,
	std::byte* const pos,
	size_t const size)
{
	std::byte* const new_pos = _vector_expand_new<ReloT, Local, Size>(self, pos, size);
	self.mid += size;
	return new_pos;
}

template<typename ReloT, bool Local, size_t Size, typename A>
std::byte* _vector_insert(_vector_allocator<A>& self, std::byte* const pos, size_t const size)
{
	static_assert(is_nothrow_relocatable_v<ReloT>);

	std::byte* const mid = self.mid;

	if (vsm_unlikely(size > static_cast<size_t>(self.end - mid)))
	{
		return _vector_insert2<ReloT, Local, Size>(self, pos, size);
	}

	self.mid = mid + size;

	if (pos != mid)
	{
		uninitialized_relocate_backward(
			reinterpret_cast<ReloT*>(pos),
			reinterpret_cast<ReloT*>(mid),
			reinterpret_cast<ReloT*>(self.mid));
	}

	return pos;
}

template<typename ReloT, typename DtorT, bool Local, size_t Size, typename A>
std::byte* _vector_resize(_vector_allocator<A>& self, size_t const new_size)
{
	size_t const cur_size = static_cast<size_t>(self.mid - self.beg);

	if (vsm_likely(new_size > cur_size))
	{
		std::byte* pos = self.mid;

		if (new_size > static_cast<size_t>(self.end - self.beg))
		{
			pos = _vector_expand_new<ReloT, Local, Size>(
				self,
				self.mid,
				new_size - cur_size);
		}

		self.mid = self.beg + new_size;

		return pos;
	}

	if (vsm_likely(new_size < cur_size))
	{
		std::byte* const mid = self.mid;
		std::byte* const new_mid = self.beg + new_size;

		if constexpr (!std::is_same_v<DtorT, init_none_t>)
		{
			std::destroy(
				reinterpret_cast<DtorT*>(new_mid),
				reinterpret_cast<DtorT*>(mid));
		}

		self.mid = new_mid;
	}

	return nullptr;
}

template<typename ReloT, typename DtorT>
std::byte* _vector_erase(_vector& self, std::byte* const begin, std::byte* const end)
{
	std::byte* const mid = self.mid;
	std::byte* const new_mid = mid - (end - begin);

	if constexpr (!std::is_same_v<DtorT, init_none_t>)
	{
		std::destroy(
			reinterpret_cast<DtorT*>(begin),
			reinterpret_cast<DtorT*>(end));
	}

	if (end != mid)
	{
		uninitialized_relocate(
			reinterpret_cast<ReloT*>(end),
			reinterpret_cast<ReloT*>(mid),
			reinterpret_cast<ReloT*>(begin));
	}

	self.mid = new_mid;

	return begin;
}

template<typename ReloT, typename DtorT>
std::byte* _vector_erase_u(_vector& self, std::byte* const begin, std::byte* const end)
{
	std::byte* const mid = self.mid;
	std::byte* const new_mid = mid - (end - begin);

	if constexpr (!std::is_same_v<DtorT, init_none_t>)
	{
		std::destroy(
			reinterpret_cast<DtorT*>(begin),
			reinterpret_cast<DtorT*>(end));
	}

	if (end != mid)
	{
		uninitialized_relocate(
			reinterpret_cast<ReloT*>(new_mid),
			reinterpret_cast<ReloT*>(mid),
			reinterpret_cast<ReloT*>(begin));
	}

	self.mid = new_mid;

	return begin;
}

template<typename DtorT>
void _vector_clear(_vector& self)
{
	std::byte* const beg = self.beg;
	std::byte* const mid = self.mid;
	self.mid = beg;

	if constexpr (!std::is_same_v<DtorT, init_none_t>)
	{
		std::destroy(
			reinterpret_cast<DtorT*>(beg),
			reinterpret_cast<DtorT*>(mid));
	}
}

template<typename ReloT, bool Local, size_t Size, typename A>
void _vector_reserve(_vector_allocator<A>& self, size_t const min_capacity)
{
	if (min_capacity > static_cast<size_t>(self.end - self.beg))
	{
		_vector_expand_new<ReloT, Local, Size>(
			self,
			self.mid,
			min_capacity - static_cast<size_t>(self.mid - self.beg));
	}
}

template<typename ReloT, typename A, bool Local>
void _vector_shrink(_vector_allocator<A>& self, _vector_capacity<Local> const local)
{
	std::byte* const mid = self.mid;
	std::byte* const end = self.end;

	if (mid == end)
	{
		return;
	}

	std::byte* const beg = self.beg;
	size_t const capacity = static_cast<size_t>(end - beg);
	size_t const new_capacity = static_cast<size_t>(mid - beg);

	std::byte* new_beg;
	if constexpr (Local)
	{
		if (capacity <= local.capacity)
		{
			return;
		}

		if (new_capacity <= local.capacity)
		{
			new_beg = reinterpret_cast<std::byte*>(self + 1);
		}
		else
		{
			new_beg = self.allocator.allocate(new_capacity);
		}
	}
	else
	{
		if (new_capacity > 0)
		{
			new_beg = self.allocator.allocate(new_capacity);
		}
		else
		{
			self.allocator.deallocate(allocation{ beg, capacity });

			self.beg = nullptr;
			self.mid = nullptr;
			self.end = nullptr;

			return;
		}
	}

	uninitialized_relocate(
		reinterpret_cast<ReloT*>(beg),
		reinterpret_cast<ReloT*>(mid),
		reinterpret_cast<ReloT*>(new_beg));

	self.allocator.deallocate(allocation{ beg, capacity });
	std::byte* const new_end = new_beg + new_capacity;

	self.beg = new_beg;
	self.mid = new_end;
	self.end = new_end;
}

template<typename DtorT>
void _vector_pop(_vector& self, size_t const size)
{
	std::byte* const mid = self.mid;
	std::byte* const new_mid = mid - size;
	self.mid = new_mid;

	if constexpr (!std::is_same_v<DtorT, init_none_t>)
	{
		std::destroy(
			reinterpret_cast<DtorT*>(new_mid),
			reinterpret_cast<DtorT*>(mid));
	}
}

template<typename T>
bool _vector_equal(_vector const& lhs, _vector const& rhs)
{
	std::byte* const lhs_beg = lhs.beg;
	std::byte* const lhs_mid = lhs.mid;
	std::byte* const rhs_beg = rhs.beg;
	std::byte* const rhs_mid = rhs.mid;

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
auto _vector_compare(_vector const& lhs, _vector const& rhs)
{
	return std::lexicographical_compare_three_way(
		reinterpret_cast<T const*>(lhs.beg),
		reinterpret_cast<T const*>(lhs.mid),
		reinterpret_cast<T const*>(rhs.beg),
		reinterpret_cast<T const*>(rhs.mid));
}

} // namespace detail

template<typename T, size_t Capacity, allocator Allocator = default_allocator>
class small_vector
{
	static constexpr bool local = Capacity > 0;
	using capacity_t = detail::_vector_capacity<local>;

	#pragma push_macro("ctor_t")
	#define ctor_t select_t<std::is_trivially_constructible_v<T>, detail::init_zero_t, T>

	#pragma push_macro("dtor_t")
	#define dtor_t select_t<std::is_trivially_destructible_v<T>, detail::init_none_t, T>

	#pragma push_macro("relo_t")
	#define relo_t select_t<is_trivially_relocatable_v<T>, std::byte, T>

	#pragma push_macro("copy_t")
	#define copy_t select_t<std::is_trivially_copyable_v<T>, std::byte, T>

	detail::_vector_storage<T, Allocator, Capacity> m;

public:
	using value_type                    = T;
	using allocator_type                = Allocator;
	using size_type                     = size_t;
	using difference_type               = ptrdiff_t;
	using reference                     = T&;
	using const_reference               = T const&;
	using pointer                       = T*;
	using const_pointer                 = T const*;
	using iterator                      = T*;
	using const_iterator                = T const*;
	using reverse_iterator              = std::reverse_iterator<T*>;
	using const_reverse_iterator        = std::reverse_iterator<T const*>;


	vsm_always_inline small_vector()
		: m(Allocator())
	{
		detail::_vector_construct<local>(
			m,
			Capacity * sizeof(T));
	}

	explicit vsm_always_inline small_vector(Allocator const& allocator)
		: m(allocator)
	{
		detail::_vector_construct<local>(
			m,
			capacity_t(Capacity * sizeof(T)));
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	small_vector(Iterator const iterator, Sentinel const sentinel, Allocator const& allocator = Allocator())
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
		: m(allocator)
	{
		detail::_vector_construct<local>(
			m,
			capacity_t(Capacity * sizeof(T)));

		if constexpr (std::sized_sentinel_for<Sentinel, Iterator>)
		{
			vsm_except_try
			{
				_append_range(iterator, sentinel);
			}
			vsm_except_catch (...)
			{
				detail::_vector_destroy<detail::init_none_t, local>(m);

				vsm_except_rethrow;
			}
		}
		else
		{
			//TODO: Implement vector::vector
			static_assert(sizeof(T) == 0, "not implemented yet");
		}
	}

	template<std::ranges::input_range R>
	small_vector(std::from_range_t, R&& range, Allocator const& allocator = Allocator())
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
		: m(allocator)
	{
		detail::_vector_construct<local>(
			m,
			capacity_t(Capacity * sizeof(T)));

		vsm_except_try
		{
			_append_range(range);
		}
		vsm_except_catch (...)
		{
			detail::_vector_destroy<detail::init_none_t, local>(m);

			vsm_except_rethrow;
		}
	}

	vsm_always_inline small_vector(small_vector&& other) noexcept
		requires allocators::is_propagatable_v<Allocator>
		: m(vsm_move(other.m))
	{
		detail::_vector_move_construct<relo_t, sizeof(T)>(
			m,
			other.m,
			capacity_t(Capacity * sizeof(T)),
			capacity_t(Capacity * sizeof(T)));
	}

	vsm_always_inline small_vector& operator=(small_vector&& other) & noexcept
		requires allocators::is_propagatable_v<Allocator>
	{
		detail::_vector_move_assign<relo_t, dtor_t, sizeof(T)>(
			m,
			other.m,
			capacity_t(Capacity * sizeof(T)),
			capacity_t(Capacity * sizeof(T)));

		return *this;
	}

	vsm_always_inline ~small_vector()
	{
		detail::_vector_destroy<dtor_t, local>(m);
	}


	[[nodiscard]] vsm_always_inline Allocator get_allocator() const
	{
		return m.allocator;
	}

	[[nodiscard]] vsm_always_inline T& operator[](size_t const index)
	{
		vsm_assert(index < size());
		return reinterpret_cast<T*>(m.beg)[index];
	}

	[[nodiscard]] vsm_always_inline T const& operator[](size_t const index) const
	{
		vsm_assert(index < size());
		return reinterpret_cast<T const*>(m.beg)[index];
	}

	[[nodiscard]] T& at(size_t const index)
	{
		if (index < size())
		{
			vsm_except_throw_or_terminate(std::out_of_range("vector index out of range"));
		}
		return reinterpret_cast<T*>(m.beg)[index];
	}

	[[nodiscard]] T const& at(size_t const index) const
	{
		if (index < size())
		{
			vsm_except_throw_or_terminate(std::out_of_range("vector index out of range"));
		}
		return reinterpret_cast<T const*>(m.beg)[index];
	}

	[[nodiscard]] vsm_always_inline T& front()
	{
		vsm_assert(!empty());
		return *reinterpret_cast<T*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline T const& front() const
	{
		vsm_assert(!empty());
		return *reinterpret_cast<T const*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline T& back()
	{
		vsm_assert(!empty());
		return reinterpret_cast<T*>(m.mid)[-1];
	}

	[[nodiscard]] vsm_always_inline T const& back() const
	{
		vsm_assert(!empty());
		return reinterpret_cast<T const*>(m.mid)[-1];
	}

	[[nodiscard]] vsm_always_inline iterator data()
	{
		return reinterpret_cast<T*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline const_iterator data() const
	{
		return reinterpret_cast<T const*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline iterator begin()
	{
		return reinterpret_cast<T*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline const_iterator begin() const
	{
		return reinterpret_cast<T const*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline const_iterator cbegin() const
	{
		return reinterpret_cast<T const*>(m.beg);
	}

	[[nodiscard]] vsm_always_inline iterator end()
	{
		return reinterpret_cast<T*>(m.mid);
	}

	[[nodiscard]] vsm_always_inline const_iterator end() const
	{
		return reinterpret_cast<T const*>(m.mid);
	}

	[[nodiscard]] vsm_always_inline const_iterator cend() const
	{
		return reinterpret_cast<T const*>(m.mid);
	}

	[[nodiscard]] vsm_always_inline reverse_iterator rbegin()
	{
		return std::make_reverse_iterator(end());
	}

	[[nodiscard]] vsm_always_inline const_reverse_iterator rbegin() const
	{
		return std::make_reverse_iterator(end());
	}

	[[nodiscard]] vsm_always_inline const_reverse_iterator crbegin() const
	{
		return std::make_reverse_iterator(cend());
	}

	[[nodiscard]] vsm_always_inline reverse_iterator rend()
	{
		return std::make_reverse_iterator(begin());
	}

	[[nodiscard]] vsm_always_inline const_reverse_iterator rend() const
	{
		return std::make_reverse_iterator(begin());
	}

	[[nodiscard]] vsm_always_inline const_reverse_iterator crend() const
	{
		return std::make_reverse_iterator(cbegin());
	}

	[[nodiscard]] vsm_always_inline bool empty() const
	{
		return m.beg == m.mid;
	}

	[[nodiscard]] vsm_always_inline size_t size() const
	{
		return static_cast<size_t>(
			reinterpret_cast<T const*>(m.mid) - reinterpret_cast<T const*>(m.beg));
	}

	[[nodiscard]] vsm_always_inline size_t max_size() const
	{
		return std::numeric_limits<size_t>::max() / sizeof(T);
	}

	vsm_always_inline void reserve(size_t const min_capacity)
	{
		detail::_vector_reserve<relo_t, local, sizeof(T)>(
			m,
			min_capacity * sizeof(T));
	}

	[[nodiscard]] vsm_always_inline size_t capacity() const
	{
		return static_cast<size_t>(
			reinterpret_cast<T const*>(m.end) - reinterpret_cast<T const*>(m.beg));
	}

	vsm_always_inline void shrink_to_fit()
	{
		detail::_vector_shrink<relo_t, Allocator, local>(
			m,
			capacity_t(Capacity * sizeof(T)));
	}

	vsm_always_inline void clear()
	{
		detail::_vector_clear<dtor_t>(m);
	}

	template<std::ranges::input_range R>
	void assign_range(R&& range)
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	{
		if constexpr (std::ranges::sized_range<R>)
		{
			return _assign_n(
				std::ranges::begin(range),
				std::ranges::size(range));
		}
		else
		{
			return _assign_range(
				std::ranges::begin(range),
				std::ranges::end(range));
		}
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	void _assign_range(Iterator const begin, Sentinel const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		if constexpr (std::sized_sentinel_for<Sentinel, Iterator>)
		{
			return _assign_n(
				begin,
				static_cast<size_t>(end - begin));
		}
		else
		{
			//TODO: Implement vector::_assign_range
			static_assert(sizeof(T) == 0, "not implemented");
		}
	}

	template<std::input_iterator Iterator>
	void _assign_n(Iterator const begin, size_t const count)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		detail::_vector_clear<dtor_t>(m);

		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		vsm_except_try
		{
			std::uninitialized_copy_n(
				begin,
				count,
				reinterpret_cast<T*>(storage));
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_convertible_v<std::iter_reference_t<Iterator>, T>)
			{
				m.mid = m.beg;
			}

			vsm_except_rethrow;
		}
	}

	template<std::convertible_to<T> U = T>
	iterator insert(const_iterator const pos, U&& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		std::byte* const storage = detail::_vector_insert<relo_t, local, sizeof(T)>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(pos)),
			sizeof(T));

		vsm_except_try
		{
			return new (storage) T(vsm_forward(value));
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_convertible_v<U, T>)
			{
				detail::_vector_erase<relo_t, detail::init_none_t>(
					m,
					storage,
					storage + sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	iterator insert(const_iterator const pos, size_t const count, T const& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		std::byte* const storage = detail::_vector_insert<relo_t, local, sizeof(T)>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(pos)),
			count * sizeof(T));

		vsm_except_try
		{
			T* const objects = reinterpret_cast<T*>(storage);

			std::uninitialized_fill_n(
				objects,
				count,
				value);

			return objects;
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_copy_constructible_v<T>)
			{
				detail::_vector_erase<relo_t, detail::init_none_t>(
					m,
					storage,
					storage + count * sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	template<std::input_iterator Iterator>
	vsm_always_inline iterator insert(const_iterator const pos, Iterator const begin, Iterator const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		return _insert_range(pos, begin, end);
	}

	template<std::input_iterator Iterator>
	iterator _insert_n(const_iterator const pos, Iterator const begin, size_t const count)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		std::byte* const storage = detail::_vector_insert<relo_t, local, sizeof(T)>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(pos)),
			count * sizeof(T));

		vsm_except_try
		{
			T* const objects = reinterpret_cast<T*>(storage);

			std::uninitialized_copy_n(
				begin,
				count,
				objects);

			return objects;
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_convertible_v<std::iter_reference_t<Iterator>, T>)
			{
				detail::_vector_erase<relo_t, detail::init_none_t>(
					m,
					storage,
					storage + count * sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	template<std::ranges::input_range R>
	vsm_always_inline iterator insert_range(const_iterator const pos, R&& range)
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	{
		if constexpr (std::ranges::sized_range<R>)
		{
			return _insert_n(
				pos,
				std::ranges::begin(range),
				std::ranges::size(range));
		}
		else
		{
			return _insert_range(
				pos,
				std::ranges::begin(range),
				std::ranges::end(range));
		}
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	vsm_always_inline iterator _insert_range(const_iterator const pos, Iterator const begin, Sentinel const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		if constexpr (std::sized_sentinel_for<Sentinel, Iterator>)
		{
			return _insert_n(
				pos,
				begin,
				static_cast<size_t>(end - begin));
		}
		else
		{
			//TODO: Implement vector::_insert_range
			static_assert(sizeof(T) == 0, "not implemented");
		}
	}

	template<std::forward_iterator Iterator>
	iterator _insert_range(const_iterator const pos, Iterator const begin, Iterator const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		return _insert_n(
			pos,
			begin,
			static_cast<size_t>(std::distance(begin, end)));
	}

	template<typename... Args>
	iterator emplace(const_iterator const pos, Args&&... args)
		requires std::constructible_from<T, Args...>
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos <= reinterpret_cast<T const*>(m.mid));

		std::byte* const storage = detail::_vector_insert<relo_t, local, sizeof(T)>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(pos)),
			sizeof(T));

		vsm_except_try
		{
			return new (storage) T(vsm_forward(args)...);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_constructible_v<T, Args...>)
			{
				detail::_vector_erase<relo_t, detail::init_none_t>(
					m,
					storage,
					storage + sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	vsm_always_inline iterator erase(const_iterator const pos)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos < reinterpret_cast<T const*>(m.mid));

		auto const next = detail::_vector_erase<relo_t, dtor_t>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(pos)),
			reinterpret_cast<std::byte*>(const_cast<T*>(pos) + 1));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline iterator _erase_unstable(const_iterator const pos)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.beg) &&
			pos < reinterpret_cast<T const*>(m.mid));

		auto const next = detail::_vector_erase_u<relo_t, dtor_t>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(pos)),
			reinterpret_cast<std::byte*>(const_cast<T*>(pos) + 1));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline iterator erase(const_iterator const begin, const_iterator const end)
	{
		vsm_assert(
			begin <= end &&
			begin >= reinterpret_cast<T const*>(m.beg) &&
			end <= reinterpret_cast<T const*>(m.mid));

		auto const next = detail::_vector_erase<relo_t, dtor_t>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(begin)),
			reinterpret_cast<std::byte*>(const_cast<T*>(end)));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline iterator _erase_unstable(const_iterator const begin, const_iterator const end)
	{
		vsm_assert(
			begin <= end &&
			begin >= reinterpret_cast<T const*>(m.beg) &&
			end <= reinterpret_cast<T const*>(m.mid));

		auto const next = detail::_vector_erase_u<relo_t, dtor_t>(
			m,
			reinterpret_cast<std::byte*>(const_cast<T*>(begin)),
			reinterpret_cast<std::byte*>(const_cast<T*>(end)));

		return reinterpret_cast<T*>(next);
	}

	template<std::convertible_to<T> U = T>
	T& push_back(U&& value)
	{
		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			sizeof(T));

		vsm_except_try
		{
			return *new (storage) T(vsm_move(value));
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_convertible_v<U, T>)
			{
				detail::_vector_pop<detail::init_none_t>(
					m,
					sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	template<std::ranges::input_range R>
	vsm_always_inline iterator append_range(R&& range)
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	{
		if constexpr (std::ranges::sized_range<R>)
		{
			return _append_n(
				std::ranges::begin(range),
				std::ranges::size(range));
		}
		else
		{
			return _append_range(
				std::ranges::begin(range),
				std::ranges::end(range));
		}
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	iterator _append_range(Iterator begin, Sentinel const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		if (std::sized_sentinel_for<Sentinel, Iterator>)
		{
			return _append_n(
				begin,
				static_cast<size_t>(end - begin));
		}
		else
		{
			//TODO: Implement vector::_append_range
			static_assert(sizeof(T) == 0, "not implemented");
		}
	}

	template<std::forward_iterator Iterator>
	vsm_always_inline iterator _append_range(Iterator const begin, Iterator const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		return _append_n(
			begin,
			static_cast<size_t>(std::distance(begin, end)));
	}

	template<std::input_iterator Iterator>
	iterator _append_n(Iterator beg, size_t const count)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		vsm_except_try
		{
			T* const objects = reinterpret_cast<T*>(storage);

			std::uninitialized_copy_n(
				beg,
				count,
				objects);

			return objects;
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_copy_constructible_v<T>)
			{
				detail::_vector_pop<detail::init_none_t>(
					m,
					count * sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	iterator _append_fill(size_t const count, T const& value)
		requires std::is_copy_constructible_v<T>
	{
		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		vsm_except_try
		{
			T* const objects = reinterpret_cast<T*>(storage);

			std::uninitialized_fill_n(
				objects,
				count,
				value);

			return objects;
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_copy_constructible_v<T>)
			{
				detail::_vector_pop<detail::init_none_t>(
					m,
					count * sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	iterator _append_default(size_t const count)
		requires std::is_default_constructible_v<T>
	{
		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		vsm_except_try
		{
			T* const objects = reinterpret_cast<T*>(storage);

			std::uninitialized_default_construct_n(
				objects,
				count);

			return objects;
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_default_constructible_v<T>)
			{
				detail::_vector_pop<detail::init_none_t>(
					m,
					count * sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	vsm_always_inline iterator _append_unitialized(size_t const count)
	{
		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		T* const objects = reinterpret_cast<T*>(storage);

		start_lifetime_as_array<T>(
			objects,
			count);

		return objects;
	}

	template<typename... Args>
	T& emplace_back(Args&&... args)
		requires std::constructible_from<T, Args...>
	{
		std::byte* const storage = detail::_vector_push<relo_t, local, sizeof(T)>(
			m,
			sizeof(T));

		vsm_except_try
		{
			return *new (storage) T(vsm_forward(args)...);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_constructible_v<T, Args...>)
			{
				detail::_vector_pop<detail::init_none_t>(
					m,
					sizeof(T));
			}

			vsm_except_rethrow;
		}
	}

	vsm_always_inline void pop_back()
	{
		vsm_assert(!empty());

		detail::_vector_pop<dtor_t>(
			m,
			sizeof(T));
	}

	vsm_always_inline void _pop_back_n(size_t const count)
	{
		vsm_assert(count <= size());

		detail::_vector_pop<dtor_t>(
			m,
			count * sizeof(T));
	}

	vsm_always_inline void _pop_back_uninitialized(size_t const count)
	{
		vsm_assert(count <= size());

		detail::_vector_pop<detail::init_none_t>(
			m,
			count * sizeof(T));
	}

	[[nodiscard]] T _pop_back_value()
	{
		vsm_assert(!empty());

		struct popper_back
		{
			detail::_vector& vector;
			bool pop_back = true;

			popper_back(detail::_vector& vector)
				: vector(vector)
			{
			}

			popper_back(popper_back const&) = delete;
			popper_back& operator=(popper_back const&) = delete;

			~popper_back()
			{
				if (pop_back)
				{
					detail::_vector_pop<dtor_t>(
						vector,
						sizeof(T));
				}
			}
		};

		[[maybe_unused]] popper_back popper(m);

		if constexpr (std::is_nothrow_move_constructible_v<T>)
		{
			return vsm_move(reinterpret_cast<T*>(m.mid)[-1]);
		}
		else
		{
			vsm_except_try
			{
				return vsm_move(reinterpret_cast<T*>(m.mid)[-1]);
			}
			vsm_except_catch (...)
			{
				popper.pop_back = false;
				vsm_except_rethrow;
			}
		}
	}

	void resize(size_t const count)
		requires std::is_default_constructible_v<T>
	{
		auto const storage = detail::_vector_resize<relo_t, dtor_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		if (storage != nullptr)
		{
			vsm_except_try
			{
				std::uninitialized_value_construct(
					reinterpret_cast<T*>(storage),
					reinterpret_cast<T*>(m.mid));
			}
			vsm_except_catch (...)
			{
				m.mid = storage;
				vsm_except_rethrow;
			}
		}
	}

	void _resize_default(size_t const count)
		requires std::is_default_constructible_v<T>
	{
		auto const storage = detail::_vector_resize<relo_t, dtor_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		if (storage != nullptr)
		{
			vsm_except_try
			{
				std::uninitialized_default_construct_n(
					reinterpret_cast<T*>(storage),
					count);
			}
			vsm_except_catch (...)
			{
				if constexpr (!std::is_nothrow_default_constructible_v<T>)
				{
					detail::_vector_erase(
						m,
						storage,
						storage + count * sizeof(T));
				}

				vsm_except_rethrow;
			}
		}
	}

	void resize(size_t const count, T const& value)
		requires std::is_copy_constructible_v<T>
	{
		auto const storage = detail::_vector_resize<relo_t, dtor_t, local, sizeof(T)>(
			m,
			count * sizeof(T));

		if (storage != nullptr)
		{
			vsm_except_try
			{
				std::uninitialized_fill_n(
					reinterpret_cast<T*>(storage),
					count,
					value);
			}
			vsm_except_catch (...)
			{
				if constexpr (!std::is_nothrow_copy_constructible_v<T>)
				{
					detail::_vector_erase(
						m,
						storage,
						storage + count * sizeof(T));
				}

				vsm_except_rethrow;
			}
		}
	}

	vsm_always_inline void swap(small_vector& other)
	{
		detail::_vector_swap<relo_t, local>(m, other.m);
	}

	[[nodiscard]] T* _release_storage()
	{
		vsm_assert(detail::_vector_dynamic<local>(m, m.beg));

		T* const objects = reinterpret_cast<T*>(m.beg);

		detail::_vector_construct<local>(
			m,
			capacity_t(Capacity * sizeof(T)));

		return objects;
	}

	void _acquire_storage(T* const objects, size_t const size, size_t const capacity)
	{
		vsm_assert(size > Capacity);

		detail::_vector_destroy<dtor_t, local>(m);

		std::byte* const beg = reinterpret_cast<std::byte*>(objects);
		m.beg = beg;
		m.mid = beg + size * sizeof(T);
		m.end = beg + capacity * sizeof(T);
	}

	friend vsm_always_inline bool operator==(small_vector const& lhs, small_vector const& rhs)
	{
		return equal<T>(lhs.m, rhs.m);
	}

	friend vsm_always_inline auto operator<=>(small_vector const& lhs, small_vector const& rhs)
	{
		return compare<T>(lhs.m, rhs.m);
	}

	#pragma pop_macro("ctor_t")
	#pragma pop_macro("dtor_t")
	#pragma pop_macro("relo_t")
	#pragma pop_macro("copy_t")
};

template<typename T, typename Allocator = default_allocator>
using vector = small_vector<T, 0, Allocator>;

template<typename T, size_t C, typename A, typename U>
typename small_vector<T, C, A>::size_type erase(small_vector<T, C, A>& v, U const& value)
{
	auto it = std::remove(v.begin(), v.end(), value);
	typename small_vector<T, C, A>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

template<typename T, size_t C, typename A, typename Pred>
typename small_vector<T, C, A>::size_type erase_if(small_vector<T, C, A>& v, Pred&& pred)
{
	auto it = std::remove_if(v.begin(), v.end(), vsm_forward(pred));
	typename small_vector<T, C, A>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

template<typename T, size_t C, typename A, typename U>
typename small_vector<T, C, A>::size_type erase_unstable(small_vector<T, C, A>& v, U const& value)
{
	auto it = remove_unstable(v.begin(), v.end(), value);
	typename small_vector<T, C, A>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

template<typename T, size_t C, typename A, typename Pred>
typename small_vector<T, C, A>::size_type erase_if_unstable(small_vector<T, C, A>& v, Pred&& pred)
{
	auto it = remove_if_unstable(v.begin(), v.end(), vsm_forward(pred));
	typename small_vector<T, C, A>::size_type n = v.end() - it;
	v._pop_back_n(n);
	return n;
}

} // namespace vsm

vsm_msvc_warning(pop)
