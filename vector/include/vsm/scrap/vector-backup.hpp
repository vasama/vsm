#pragma once

#include <vsm/assert.h>
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

template<typename T>
struct buffer_ref
{
	T* data;
	size_t size;
	size_t capacity;
};

namespace detail::vector_ {

using std::byte;

template<typename T>
using const_param = std::conditional<
	std::is_trivially_copyable_v<T> &&
	(std::is_empty_v<T> || sizeof(T) <= sizeof(void*)),
	T const, T const&>;

template<typename T>
using const_param_t = typename const_param<T>::type;

template<typename T>
using is_trivially_relocatable = std::is_trivially_copyable<T>;

template<typename T>
constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<T>::value;

template<typename T>
using allocator_param = std::conditional<std::is_empty_v<T>, T, T&>;

template<typename T>
using allocator_param_t = typename allocator_param<T>::type;

template<typename T>
auto is_iterator_2(T it)
	-> decltype(it != it, *it, ++it, std::true_type{});

template<typename T>
auto is_iterator_1(int) -> decltype(is_iterator_2(std::declval<T>()));

template<typename>
std::false_type is_iterator_1(...);

template<typename T>
using is_iterator = decltype(is_iterator_1<T>(0));

template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<typename T>
void relocate(T* dst, T* src_beg, T* const src_end)
{
	if constexpr (is_trivially_relocatable_v<T>)
	{
		std::memcpy(
			reinterpret_cast<byte*>(dst),
			reinterpret_cast<byte*>(src_beg),
			reinterpret_cast<byte*>(src_end) -
				reinterpret_cast<byte*>(src_beg));
	}
	else
	{
		for (; src_beg != src_end; ++dst, ++src_beg)
		{
			new(dst) T(vsm_move(*src_beg));
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
			reinterpret_cast<byte*>(src_end) -
				reinterpret_cast<byte*>(src_beg));
	}
	else
	{
		for (; src_beg != src_end; ++dst, ++src_beg)
		{
			new(dst) T(vsm_move(*src_beg));
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
			reinterpret_cast<byte*>(src_end) -
				reinterpret_cast<byte*>(src_beg));
	}
	else
	{
		for (dst += src_end - src_beg; src_end-- != src_beg;)
		{
			new(--dst) T(vsm_move(*src_end));
			src_end->~T();
		}
	}
}

struct core
{
	byte* beg;
	byte* mid;
	byte* end;
};

template<bool>
struct capacity_wrapper
{
	constexpr capacity_wrapper(size_t)
	{
	}
};

template<>
struct capacity_wrapper<true>
{
	size_t capacity;

	constexpr capacity_wrapper(size_t capacity)
		: capacity(capacity)
	{
	}
};

template<bool HasLocalStorage>
bool is_dynamic(core* const array, byte* const beg)
{
	if constexpr (HasLocalStorage)
	{
		return beg != reinterpret_cast<byte*>(array + 1);
	}
	else
	{
		return beg != nullptr;
	}
}

template<bool HasLocalStorage>
void construct(core* const array, capacity_wrapper<HasLocalStorage> const local_capacity)
{
	if constexpr (HasLocalStorage)
	{
		byte* const storage = reinterpret_cast<byte*>(array + 1);

		array->beg = storage;
		array->mid = storage;
		array->end = storage + local_capacity.capacity;
	}
	else
	{
		array->beg = nullptr;
		array->mid = nullptr;
		array->end = nullptr;
	}
}

template<typename DestroyT, typename Allocator, bool HasLocalStorage>
void destroy(core* const array, allocator_param_t<Allocator> allocator)
{
	byte* const beg = array->beg;
	byte* const mid = array->mid;

	std::destroy(
		reinterpret_cast<DestroyT*>(beg),
		reinterpret_cast<DestroyT*>(mid));

	if (is_dynamic<HasLocalStorage>(array, beg))
	{
		allocator.deallocate(beg, array->end - beg);
	}
}

template<typename RelocateT, typename Allocator, bool HasLocalStorage>
struct operations
{
	typedef allocator_param_t<Allocator> allocator_param_type;

	template<typename DestroyT, typename CopyT>
	static void copy_assign(core* const array,
		core const* const src, allocator_param_type allocator)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;

		std::destroy(
			reinterpret_cast<DestroyT*>(beg),
			reinterpret_cast<DestroyT*>(mid));

		array->mid = beg;

		push_back_range(array,
			reinterpret_cast<CopyT const*>(src->beg),
			reinterpret_cast<CopyT const*>(src->mid), allocator);
	}

	template<typename DestroyT, typename CopyT>
	static void copy_assign_allocator(core* const array, core* const src,
		Allocator& allocator, const_param_t<Allocator> src_allocator,
		capacity_wrapper<HasLocalStorage> const local_capacity)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;

		std::destroy(
			reinterpret_cast<DestroyT*>(beg),
			reinterpret_cast<DestroyT*>(mid));

		if (is_dynamic<HasLocalStorage>(array, beg))
		{
			allocator.deallocate(beg, array->end - beg);
		}

		allocator = src_allocator;
		construct<HasLocalStorage>(array, local_capacity);

		push_back_range(array,
			reinterpret_cast<CopyT const*>(src->beg),
			reinterpret_cast<CopyT const*>(src->mid), allocator);
	}

	template<bool SrcHasLocalStorage>
	static void move_construct(core* const array,
		core* const src, allocator_param_type allocator,
		capacity_wrapper<HasLocalStorage> const local_capacity,
		capacity_wrapper<SrcHasLocalStorage> const src_local_capacity)
	{
		byte* const srcbeg = src->beg;
		byte* const srcmid = src->mid;

		if (srcbeg == srcmid)
		{
			construct<HasLocalStorage>(array, local_capacity);
			return;
		}

		size_t const size = srcmid - srcbeg;

		if (is_dynamic<SrcHasLocalStorage>(src, srcbeg))
		{
			byte* const srcend = src->end;

			bool adopt = true;
			if constexpr (HasLocalStorage)
				adopt = (size_t)(srcend - srcbeg) < local_capacity.capacity;

			if (adopt)
			{
				array->beg = srcbeg;
				array->mid = srcmid;
				array->end = srcend;

				construct<SrcHasLocalStorage>(src, src_local_capacity);

				return;
			}
		}

		byte* const beg = [&]
		{
			if constexpr (HasLocalStorage)
			{
				if (size <= local_capacity.capacity)
				{
					byte* const beg = reinterpret_cast<byte*>(array + 1);
					array->end = beg + local_capacity.capacity;
					return beg;
				}
			}

			beg = allocator.allocate(size);
			array->end = beg + size;
			return beg;
		}();

		relocate(
			reinterpret_cast<RelocateT*>(beg),
			reinterpret_cast<RelocateT*>(srcbeg),
			reinterpret_cast<RelocateT*>(srcmid));

		array->beg = beg;
		array->mid = beg + size;
		src->mid = srcbeg;
	}

	template<typename DestroyT, bool SrcHasLocalStorage>
	static void move_assign(core* const array,
		core* const src, allocator_param_type allocator,
		capacity_wrapper<HasLocalStorage> const local_capacity,
		capacity_wrapper<SrcHasLocalStorage> const src_local_capacity)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;

		std::destroy(
			reinterpret_cast<DestroyT*>(beg),
			reinterpret_cast<DestroyT*>(mid));

		byte* const srcbeg = src->beg;
		byte* const srcmid = src->mid;

		if (srcbeg == srcmid)
		{
			array->mid = beg;
			return;
		}

		size_t const size = srcmid - srcbeg;

		byte* const end = array->end;
		if (is_dynamic<SrcHasLocalStorage>(src, srcbeg))
		{
			byte* const srcend = src->end;

			bool adopt = true;
			if constexpr (HasLocalStorage)
				adopt = srcend - srcbeg < local_capacity.capacity;

			if (adopt)
			{
				if (is_dynamic<HasLocalStorage>(array, beg))
				{
					allocator.deallocate(beg, end - beg);
				}

				array->beg = srcbeg;
				array->mid = srcmid;
				array->end = srcend;

				construct<SrcHasLocalStorage>(src, src_local_capacity);

				return;
			}
		}

		size_t const capacity = end - beg;
		if (size > capacity)
		{
			allocator.deallocate(beg, capacity);
			beg = allocator.allocate(size);

			array->beg = beg;
			array->end = beg + size;
		}

		relocate(
			reinterpret_cast<RelocateT*>(beg),
			reinterpret_cast<RelocateT*>(srcbeg),
			reinterpret_cast<RelocateT*>(srcmid));

		array->mid = beg + size;
		src->mid = srcbeg;
	}

	template<typename DestroyT>
	static void move_assign_allocator(core* const array,
		core* const src, Allocator& allocator, Allocator& src_allocator,
		capacity_wrapper<HasLocalStorage> const local_capacity)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;

		std::destroy(
			reinterpret_cast<DestroyT*>(beg),
			reinterpret_cast<DestroyT*>(mid));

		if (is_dynamic<HasLocalStorage>(array, beg))
		{
			allocator.deallocate(beg, array->end - beg);
		}

		allocator = vsm_move(src_allocator);
		construct<HasLocalStorage>(array, local_capacity);

		move_elements(array, src, allocator);
	}

	template<typename DestroyT>
	static void move_assign_elements(core* const array,
		core* const src, allocator_param_type allocator)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;

		std::destroy(
			reinterpret_cast<DestroyT*>(beg),
			reinterpret_cast<DestroyT*>(mid));

		array->mid = array->beg;

		move_elements(array, src, allocator);
	}

	static void move_elements(core* const array,
		core* const src, allocator_param_type allocator)
	{
		byte* const src_beg = src->beg;
		byte* const src_mid = src->mid;

		byte* const slots = push_back(array, src_mid - src_beg, allocator);

		relocate(
			reinterpret_cast<RelocateT*>(slots),
			reinterpret_cast<RelocateT*>(src_beg),
			reinterpret_cast<RelocateT*>(src_mid));

		src->mid = src_beg;
	}

	static byte* expand(core* const array,
		size_t const min_capacity, allocator_param_type allocator)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;
		byte* const end = array->end;

		size_t const capacity = end - beg;
		size_t const new_capacity = std::max(min_capacity, capacity * 2);

		byte* const new_beg = reinterpret_cast<byte*>(allocator.allocate(new_capacity));
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
			empty = reinterpret_cast<byte*>(array + 1);

		if (beg != empty)
		{
			allocator.deallocate(beg, capacity);
		}

		array->beg = new_beg;
		array->mid = new_beg + (mid - beg);
		array->end = new_end;

		return new_beg;
	}

	static vsm_never_inline byte* push_back_slow_path(
		core* const array, size_t const size, allocator_param_type allocator)
	{
		size_t const cur_size = array->mid - array->beg;
		size_t const new_size = cur_size + size;

		byte* const new_beg = expand(array, new_size, allocator);

		array->mid = new_beg + new_size;
		return new_beg + cur_size;
	}

	static byte* push_back(core* const array,
		size_t const size, allocator_param_type allocator)
	{
		byte* const mid = array->mid;
		byte* const new_mid = mid + size;

		if (vsm_unlikely(new_mid > array->end))
		{
			return push_back_slow_path(array, size, allocator);
		}

		array->mid = new_mid;
		return mid;
	}

	template<typename CopyT>
	static void push_back_range(core* const array,
		CopyT const* const first, CopyT const* const last, allocator_param_type allocator)
	{
		byte* const slots = push_back(array,
			reinterpret_cast<byte const*>(last) -
				reinterpret_cast<byte const*>(first), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<CopyT*>(slots));
	}

	template<typename T, typename InputIt>
	static void push_back_range(core* const array,
		InputIt const first, InputIt const last, allocator_param_type allocator)
	{
		byte* const slots = push_back(array,
			std::distance(first, last) * sizeof(T), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<T*>(slots));
	}
	
	static vsm_never_inline byte* insert_slow_path(
		core* const array, byte* const pos, size_t const size, allocator_param_type allocator)
	{
		byte* const beg = array->beg;

		size_t const offset = pos - beg;
		size_t const cur_size = array->mid - beg;
		size_t const new_size = cur_size + size;

		byte* const new_beg = expand(array, new_size, allocator);

		byte* const new_pos = new_beg + offset;

		relocate_right(
			reinterpret_cast<RelocateT*>(new_pos + size),
			reinterpret_cast<RelocateT*>(new_pos),
			reinterpret_cast<RelocateT*>(new_beg + cur_size));

		array->mid = new_beg + new_size;
		return new_pos;
	}

	static byte* insert(core* const array,
		byte* const pos, size_t const size, allocator_param_type allocator)
	{
		byte* const mid = array->mid;
		byte* const new_mid = mid + size;

		if (vsm_unlikely(new_mid > array->end))
		{
			return insert_slow_path(array, pos, size, allocator);
		}

		if (pos != mid)
		{
			relocate_right(
				reinterpret_cast<RelocateT*>(pos + size),
				reinterpret_cast<RelocateT*>(pos),
				reinterpret_cast<RelocateT*>(mid));
		}

		array->mid = new_mid;
		return pos;
	}

	template<typename CopyT>
	static byte* insert_range(core* const array, byte* const pos,
		CopyT const* const first, CopyT const* const last, allocator_param_type allocator)
	{
		byte* const slots = insert(array, pos,
			reinterpret_cast<byte const*>(last) -
				reinterpret_cast<byte const*>(first), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<CopyT*>(slots));

		return slots;
	}

	template<typename T, typename InputIt>
	static byte* insert_range(core* const array, byte* const pos,
		InputIt const first, InputIt const last, allocator_param_type allocator)
	{
		byte* const slots = insert(array, pos,
			std::distance(first, last) * sizeof(T), allocator);

		std::uninitialized_copy(first, last, reinterpret_cast<T*>(slots));

		return slots;
	}

	template<bool Construct, typename ConstructT, typename DestroyT>
	static byte* resize(core* const array, size_t const new_size, allocator_param_type allocator)
	{
		byte* const beg = array->beg;
		byte* const mid = array->mid;
		byte* const new_mid = beg + new_size;

		if (vsm_likely(new_mid > mid))
		{
			if (new_mid > array->end)
			{
				byte* const new_beg = expand(array, new_size, allocator);

				mid = new_beg + (mid - beg);
				new_mid = new_beg + new_size;
			}

			array->mid = new_mid;

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
			array->mid = new_mid;

			std::destroy(
				reinterpret_cast<DestroyT*>(new_mid),
				reinterpret_cast<DestroyT*>(mid));

			return new_mid;
		}

		return mid;
	}

	static void reserve(core* const array,
		size_t const min_capacity, allocator_param_type allocator)
	{
		if (static_cast<size_t>(array->end - array->beg) < min_capacity)
		{
			expand(array, min_capacity, allocator);
		}
	}

	static void shrink_to_fit(core* const array,
		allocator_param_type const allocator, capacity_wrapper<HasLocalStorage> const local_capacity)
	{
		byte* const mid = array->mid;
		byte* const end = array->end;

		if (mid == end) return;

		byte* const beg = array->beg;
		size_t const capacity = end - beg;
		size_t const new_capacity = mid - beg;

		byte* new_beg;
		if constexpr (HasLocalStorage)
		{
			if (capacity <= local_capacity.capacity)
				return;

			if (new_capacity <= local_capacity.capacity)
			{
				new_beg = reinterpret_cast<byte*>(array + 1);
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
				allocator.deallocate(beg, capacity);

				array->beg = nullptr;
				array->mid = nullptr;
				array->end = nullptr;

				return;
			}
		}

		relocate(
			reinterpret_cast<RelocateT*>(new_beg),
			reinterpret_cast<RelocateT*>(beg),
			reinterpret_cast<RelocateT*>(mid));

		allocator.deallocate(beg, capacity);

		byte* const new_end = new_beg + new_capacity;

		array->beg = new_beg;
		array->mid = new_end;
		array->end = new_end;
	}
};

template<typename DestroyT>
void clear(core* const array)
{
	byte* const beg = array->beg;
	byte* const mid = array->mid;
	array->mid = beg;

	std::destroy(
		reinterpret_cast<DestroyT*>(beg),
		reinterpret_cast<DestroyT*>(mid));
}

template<typename RelocateT, typename DestroyT>
byte* erase(core* const array, byte* const first, byte* const last)
{
	byte* const mid = array->mid;
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

	array->mid = new_mid;

	return first;
}

template<typename RelocateT, typename DestroyT>
byte* erase_unstable(core* const array, byte* const first, byte* const last)
{
	byte* const mid = array->mid;
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

	array->mid = new_mid;

	return first;
}

template<typename DestroyT>
void pop_back(core* const array, size_t const size)
{
	byte* const mid = array->mid;
	byte* const new_mid = mid - size;
	array->mid = new_mid;

	reinterpret_cast<DestroyT*>(new_mid)->~DestroyT();
}

template<typename DestroyT>
void pop_back_n(core* const array, size_t const size)
{
	byte* const mid = array->mid;
	byte* const new_mid = mid - size;
	array->mid = new_mid;

	std::destroy(
		reinterpret_cast<DestroyT*>(new_mid),
		reinterpret_cast<DestroyT*>(mid));
}

template<typename T>
bool equal(core const* const lhs, core const* const rhs)
{
	byte* const lhs_beg = lhs->beg;
	byte* const lhs_mid = lhs->mid;
	byte* const rhs_beg = rhs->beg;
	byte* const rhs_mid = rhs->mid;

	if (lhs_mid - lhs_beg != rhs_mid - rhs_beg)
		return false;
	
	return std::equal(
		reinterpret_cast<T const*>(lhs_beg),
		reinterpret_cast<T const*>(lhs_mid),
		reinterpret_cast<T const*>(rhs_beg),
		reinterpret_cast<T const*>(rhs_mid));
}

template<typename T>
bool compare(core const* const lhs, core const* const rhs)
{
	return std::lexicographical_compare(
		reinterpret_cast<T const*>(lhs->beg),
		reinterpret_cast<T const*>(lhs->mid),
		reinterpret_cast<T const*>(rhs->beg),
		reinterpret_cast<T const*>(rhs->mid));
}

template<typename, bool>
struct storage_alignment
{
	static constexpr size_t value = 1;
};

template<typename T>
struct storage_alignment<T, true>
{
	static constexpr size_t value = alignof(T);
};

template<typename T, size_t LocalCapacity, bool =
	(storage_alignment<T, (LocalCapacity > 0)>::value > alignof(core))>
struct storage;

template<typename T, size_t LocalCapacity>
struct storage<T, LocalCapacity, 0>
{
	core c;
	std::aligned_storage_t<
		sizeof(T) * LocalCapacity,
		alignof(T)
	> mutable s;
};

template<typename T, size_t LocalCapacity>
struct storage<T, LocalCapacity, 1>
{
	// Some padding is needed. Place it here to avoid padding between core and storage.
	byte p[((sizeof(core) + alignof(T) - 1) & ~(alignof(T) - 1)) - sizeof(core)];

	core c;
	byte mutable s alignas(T)[sizeof(T) * LocalCapacity];
};

template<typename T>
struct storage<T, 0, 0>
{
	core c;
};

template<typename T>
struct storage<T, 0, 1>
{
	core c;
};

template<typename T, typename Allocator>
struct allocator_ebo
	: Allocator
	, T
{
	allocator_ebo() = default;

	vsm_always_inline allocator_ebo(Allocator const& allocator)
		: Allocator(allocator)
	{
	}
};

template<typename T, typename Allocator, size_t LocalCapacity>
class vector
{
	static constexpr bool has_local_storage = LocalCapacity > 0;

#define vsm_detail_vector_storage_size (LocalCapacity * sizeof(T))

	typedef typename std::allocator_traits<Allocator>
		::template rebind_alloc<byte> byte_allocator;

	typedef std::allocator_traits<byte_allocator> byte_allocator_traits;

#define vsm_detail_vector_construct_type \
	std::conditional_t<std::is_trivially_constructible_v<T>, byte, T>

#define vsm_detail_vector_destroy_type \
	std::conditional_t<std::is_trivially_destructible_v<T>, byte, T>

#define vsm_detail_vector_relocate_type \
	std::conditional_t<is_trivially_relocatable_v<T>, byte, T>

#define vsm_detail_vector_copy_type \
	std::conditional_t<std::is_trivially_copyable_v<T>, byte, T>

#define vsm_detail_vector_operations \
	vector_::operations<vsm_detail_vector_relocate_type, byte_allocator, has_local_storage>

	allocator_ebo<storage<T, LocalCapacity>, byte_allocator> m;

public:
	typedef T value_type;
	typedef Allocator allocator_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T& reference;
	typedef T const& const_reference;
	typedef T* pointer;
	typedef T const* const_pointer;
	typedef T* iterator;
	typedef T const* const_iterator;
	typedef std::reverse_iterator<T*> reverse_iterator;
	typedef std::reverse_iterator<T const*> const_reverse_iterator;

	vsm_always_inline vector()
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);
	}
	
	explicit vsm_always_inline vector(Allocator const& allocator)
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);
	}

	explicit vsm_always_inline vector(size_t const count,
		Allocator const& allocator = Allocator())
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);
		resize(count);
	}

	vsm_always_inline vector(size_t const count,
		T const& value, Allocator const& allocator = Allocator())
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

		resize(count, value);
	}

	vsm_always_inline vector(T const* const first, T const* const last,
		Allocator const& allocator = Allocator())
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

		vsm_detail_vector_operations::template push_back_range<vsm_detail_vector_copy_type>(&m.c,
			reinterpret_cast<vsm_detail_vector_copy_type const*>(first),
			reinterpret_cast<vsm_detail_vector_copy_type const*>(last),
			static_cast<byte_allocator&>(m));
	}

	template<typename InputIt>
	vsm_always_inline vector(InputIt const first, InputIt const last,
		Allocator const& allocator = Allocator())
		requires is_iterator_v<InputIt>
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

		vsm_detail_vector_operations::template push_back_range<T, InputIt>(
			&m.c, first, last, static_cast<byte_allocator&>(m));
	}

	vsm_always_inline vector(std::initializer_list<T> list,
		Allocator const& allocator = Allocator())
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

		vsm_detail_vector_operations::template push_back_range<vsm_detail_vector_copy_type>(&m.c,
			reinterpret_cast<vsm_detail_vector_copy_type const*>(list.begin()),
			reinterpret_cast<vsm_detail_vector_copy_type const*>(list.end()),
			static_cast<byte_allocator&>(m));
	}

	vsm_always_inline vector(vector&& other)
		: m(vsm_move(other.m))
	{
		vsm_detail_vector_operations::template move_construct<has_local_storage>(
			&m.c, &other.m.c, static_cast<byte_allocator&>(m),
			vsm_detail_vector_storage_size, vsm_detail_vector_storage_size);
	}

	vsm_always_inline vector(vector&& other, Allocator const& allocator)
		: m(byte_allocator(allocator))
	{
		if constexpr (byte_allocator_traits::is_always_equal::value)
		{
			vsm_detail_vector_operations::template move_construct<has_local_storage>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m),
				vsm_detail_vector_storage_size, vsm_detail_vector_storage_size);
		}
		else if (static_cast<byte_allocator const&>(m) ==
			static_cast<byte_allocator const&>(other.m))
		{
			vsm_detail_vector_operations::template move_construct<has_local_storage>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m),
				vsm_detail_vector_storage_size, vsm_detail_vector_storage_size);
		}
		else
		{
			construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

			vsm_detail_vector_operations::move_elements(&m.c,
				&other.m.c, static_cast<byte_allocator&>(m));
		}
	}

	vsm_always_inline vector(vector const& other)
		: m(byte_allocator_traits::select_on_container_copy_construction(
				static_cast<byte_allocator const&>(other.m)))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

		vsm_detail_vector_operations::template push_back_range<vsm_detail_vector_copy_type>(&m.c,
			reinterpret_cast<vsm_detail_vector_copy_type const*>(other.m.c.beg),
			reinterpret_cast<vsm_detail_vector_copy_type const*>(other.m.c.mid),
			static_cast<byte_allocator&>(m));
	}

	vsm_always_inline vector(vector const& other, Allocator const& allocator)
		: m(byte_allocator(allocator))
	{
		construct<has_local_storage>(&m.c, vsm_detail_vector_storage_size);

		vsm_detail_vector_operations::template push_back_range<vsm_detail_vector_copy_type>(&m.c,
			reinterpret_cast<vsm_detail_vector_copy_type const*>(other.m.c.beg),
			reinterpret_cast<vsm_detail_vector_copy_type const*>(other.m.c.mid),
			static_cast<byte_allocator&>(m));
	}

	vsm_always_inline ~vector()
	{
		destroy<vsm_detail_vector_destroy_type, byte_allocator, has_local_storage>(
			&m.c, static_cast<byte_allocator&>(m));
	}

	vsm_always_inline vector& operator=(vector&& other) &
	{
		if constexpr (byte_allocator_traits::is_always_equal::value)
		{
			vsm_detail_vector_operations::template move_assign<
				vsm_detail_vector_destroy_type, has_local_storage>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m),
				vsm_detail_vector_storage_size, vsm_detail_vector_storage_size);
		}
		else if (static_cast<byte_allocator const&>(m) ==
			static_cast<byte_allocator const&>(other.m))
		{
			vsm_detail_vector_operations::template move_assign<
				vsm_detail_vector_destroy_type, has_local_storage>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m));
		}
		else if constexpr (byte_allocator_traits
			::propagate_on_container_move_assignment::value)
		{
			vsm_detail_vector_operations::template
				move_assign_allocator<vsm_detail_vector_destroy_type>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m),
				static_cast<byte_allocator&>(other.m),
				vsm_detail_vector_storage_size);
		}
		else
		{
			vsm_detail_vector_operations::template
				move_assign_elements<vsm_detail_vector_destroy_type>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m));
		}

		return *this;
	}

	vsm_always_inline vector& operator=(vector const& other) &
	{
		if constexpr (!byte_allocator_traits
			::propagate_on_container_copy_assignment::value ||
			byte_allocator_traits::is_always_equal::value)
		{
			vsm_detail_vector_operations::template copy_assign<
				vsm_detail_vector_destroy_type, vsm_detail_vector_copy_type>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m));
		}
		else if (static_cast<byte_allocator const&>(m) ==
			static_cast<byte_allocator const&>(other.m))
		{
			vsm_detail_vector_operations::template copy_assign<
				vsm_detail_vector_destroy_type, vsm_detail_vector_copy_type>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m));
		}
		else
		{
			vsm_detail_vector_operations::template copy_assign_allocator<
				vsm_detail_vector_destroy_type, vsm_detail_vector_copy_type>(
				&m.c, &other.m.c, static_cast<byte_allocator&>(m),
				static_cast<byte_allocator const&>(other.m),
				vsm_detail_vector_storage_size);
		}

		return *this;
	}

	vsm_always_inline vector& operator=(std::initializer_list<T> const list) &
	{
		clear();

		vsm_detail_vector_operations::template push_back_range<vsm_detail_vector_copy_type>(&m.c,
			reinterpret_cast<vsm_detail_vector_copy_type const*>(list.begin()),
			reinterpret_cast<vsm_detail_vector_copy_type const*>(list.end()),
			static_cast<byte_allocator&>(m));

		return *this;
	}

	vsm_always_inline void assign(size_t const count, T const& value) &
	{
		clear();

		resize(count, value);
	}

	template<typename InputIt>
	vsm_always_inline void assign(InputIt const first, InputIt const last) &
	{
		clear();

		vsm_detail_vector_operations::template push_back_range<T, InputIt>(
			&m.c, first, last, static_cast<byte_allocator&>(m));
	}

	vsm_always_inline void assign(std::initializer_list<T> const list) &
	{
		clear();

		vsm_detail_vector_operations::template push_back_range<vsm_detail_vector_copy_type>(&m.c,
			reinterpret_cast<vsm_detail_vector_copy_type const*>(list.begin()),
			reinterpret_cast<vsm_detail_vector_copy_type const*>(list.end()),
			static_cast<byte_allocator&>(m));
	}

	vsm_always_inline Allocator get_allocator() const
	{
		return Allocator(static_cast<byte_allocator const&>(m));
	}

	vsm_always_inline T& operator[](size_t const index)
	{
		vsm_assert(index < size());
		return reinterpret_cast<T*>(m.c.beg)[index];
	}

	vsm_always_inline T const& operator[](size_t const index) const
	{
		vsm_assert(index < size());
		return reinterpret_cast<T const*>(m.c.beg)[index];
	}

	vsm_always_inline T& at(size_t const index)
	{
		vsm_assert(index < size());
		return reinterpret_cast<T*>(m.c.beg)[index];
	}

	vsm_always_inline T const& at(size_t const index) const
	{
		vsm_assert(index < size());
		return reinterpret_cast<T const*>(m.c.beg)[index];
	}

	vsm_always_inline T& front()
	{
		vsm_assert(!empty());
		return *reinterpret_cast<T*>(m.c.beg);
	}

	vsm_always_inline T const& front() const
	{
		vsm_assert(!empty());
		return *reinterpret_cast<T const*>(m.c.beg);
	}

	vsm_always_inline T& back()
	{
		vsm_assert(!empty());
		return reinterpret_cast<T*>(m.c.mid)[-1];
	}

	vsm_always_inline T const& back() const
	{
		vsm_assert(!empty());
		return reinterpret_cast<T const*>(m.c.mid)[-1];
	}

	vsm_always_inline T* data()
	{
		return reinterpret_cast<T*>(m.c.beg);
	}

	vsm_always_inline T const* data() const
	{
		return reinterpret_cast<T const*>(m.c.beg);
	}

	vsm_always_inline T* begin()
	{
		return reinterpret_cast<T*>(m.c.beg);
	}

	vsm_always_inline T const* begin() const
	{
		return reinterpret_cast<T const*>(m.c.beg);
	}

	vsm_always_inline T const* cbegin() const
	{
		return reinterpret_cast<T const*>(m.c.beg);
	}

	vsm_always_inline T* end()
	{
		return reinterpret_cast<T*>(m.c.mid);
	}

	vsm_always_inline T const* end() const
	{
		return reinterpret_cast<T const*>(m.c.mid);
	}

	vsm_always_inline T const* cend() const
	{
		return reinterpret_cast<T const*>(m.c.mid);
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
		return m.c.beg == m.c.mid;
	}

	vsm_always_inline size_t size() const
	{
		return reinterpret_cast<T const*>(m.c.mid) - reinterpret_cast<T const*>(m.c.beg);
	}

	vsm_always_inline size_t max_size() const
	{
		return std::numeric_limits<size_t>::max();
	}

	vsm_always_inline void reserve(size_t const min_capacity)
	{
		vsm_detail_vector_operations::reserve(&m.c,
			min_capacity * sizeof(T), static_cast<byte_allocator&>(m));
	}

	vsm_always_inline size_t capacity() const
	{
		return reinterpret_cast<T const*>(m.c.end) - reinterpret_cast<T const*>(m.c.beg);
	}

	vsm_always_inline void shrink_to_fit()
	{
		vsm_detail_vector_operations::shrink_to_fit(&m.c,
			static_cast<byte_allocator&>(m), vsm_detail_vector_storage_size);
	}

	vsm_always_inline void clear()
	{
		vector_::clear<vsm_detail_vector_destroy_type>(&m.c);
	}

	vsm_always_inline T* insert(T const* const pos, T const& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos <= reinterpret_cast<T const*>(m.c.mid));

		byte* const slot = vsm_detail_vector_operations::insert(&m.c,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			sizeof(T), static_cast<byte_allocator&>(m));

		return new(slot) T(value);
	}

	vsm_always_inline T* insert(T const* const pos, T&& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos <= reinterpret_cast<T const*>(m.c.mid));

		byte* const slot = vsm_detail_vector_operations::insert(&m.c,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			sizeof(T), static_cast<byte_allocator&>(m));

		return new(slot) T(vsm_move(value));
	}

	T* insert(T const* const pos, size_t const count, T const& value)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos <= reinterpret_cast<T const*>(m.c.mid));

		size_t const size = count * sizeof(T);

		byte* const slots = vsm_detail_vector_operations::insert(&m.c,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			size, static_cast<byte_allocator&>(m));

		std::uninitialized_fill(
			reinterpret_cast<T*>(slots),
			reinterpret_cast<T*>(slots + size), value);

		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* insert(T const* const pos, T const* const first, T const* const last)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos <= reinterpret_cast<T const*>(m.c.mid));

		byte* const slots = vsm_detail_vector_operations
			::template insert_range<vsm_detail_vector_copy_type>(
				&m.c, reinterpret_cast<byte*>(const_cast<T*>(pos)),
				reinterpret_cast<vsm_detail_vector_copy_type const*>(first),
				reinterpret_cast<vsm_detail_vector_copy_type const*>(last),
				static_cast<byte_allocator&>(m));

		return reinterpret_cast<T*>(slots);
	}

	template<typename InputIt>
	vsm_always_inline T* insert(T const* const pos, InputIt const first, InputIt const last)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos <= reinterpret_cast<T const*>(m.c.mid));

		byte* const slots = vsm_detail_vector_operations::template insert_range<T, InputIt>(
			&m.c, reinterpret_cast<byte*>(const_cast<T*>(pos)),
			first, last, static_cast<byte_allocator&>(m));

		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* insert(T const* const pos, std::initializer_list<T> const list)
	{
		return insert(pos, list.begin(), list.end());
	}

	template<typename... Args>
	vsm_always_inline T* emplace(T const* const pos, Args&&... args)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos <= reinterpret_cast<T const*>(m.c.mid));

		byte* const slot = vsm_detail_vector_operations::insert(&m.c,
			reinterpret_cast<byte*>(const_cast<T*>(pos)),
			sizeof(T), static_cast<byte_allocator&>(m));

		return new(slot) T(vsm_forward(args)...);
	}

	vsm_always_inline T* erase(T const* const pos)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos < reinterpret_cast<T const*>(m.c.mid));

		byte* const next = vector_::erase<vsm_detail_vector_relocate_type, vsm_detail_vector_destroy_type>(
			&m.c, reinterpret_cast<byte*>(const_cast<T*>(pos)),
			reinterpret_cast<byte*>(const_cast<T*>(pos) + 1));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline T* _erase_unstable(T const* const pos)
	{
		vsm_assert(
			pos >= reinterpret_cast<T const*>(m.c.beg) &&
			pos < reinterpret_cast<T const*>(m.c.mid));

		byte* const next = vector_::erase_unstable<vsm_detail_vector_relocate_type, vsm_detail_vector_destroy_type>(
			&m.c, reinterpret_cast<byte*>(const_cast<T*>(pos)),
			reinterpret_cast<byte*>(const_cast<T*>(pos) + 1));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline T* erase(T const* const first, T const* const last)
	{
		vsm_assert(first <= last &&
			first >= reinterpret_cast<T const*>(m.c.beg) &&
			last <= reinterpret_cast<T const*>(m.c.mid));

		byte* const next = vector_::erase<vsm_detail_vector_relocate_type, vsm_detail_vector_destroy_type>(
			&m.c, reinterpret_cast<byte*>(const_cast<T*>(first)),
			reinterpret_cast<byte*>(const_cast<T*>(last)));

		return reinterpret_cast<T*>(next);
	}

	vsm_always_inline T& push_back(T&& value)
	{
		byte* const slot = vsm_detail_vector_operations::push_back(&m.c,
			sizeof(T), static_cast<byte_allocator&>(m));

		return *new(slot) T(vsm_move(value));
	}

	vsm_always_inline T& push_back(T const& value)
	{
		byte* const slot = vsm_detail_vector_operations::push_back(&m.c,
			sizeof(T), static_cast<byte_allocator&>(m));

		return *new(slot) T(value);
	}

	template<typename InputIt>
	vsm_always_inline T* _push_back_range(InputIt const beg, InputIt const end)
	{
		byte* const slots = vsm_detail_vector_operations::push_back(&m.c,
			std::distance(beg, end) * sizeof(T), static_cast<byte_allocator&>(m));

		std::uninitialized_copy(beg, end, reinterpret_cast<T*>(slots));
		return reinterpret_cast<T*>(slots);
	}

	template<typename InputIt>
	vsm_always_inline T* _push_back_n(InputIt const beg, size_t const count)
	{
		byte* const slots = vsm_detail_vector_operations::push_back(&m.c,
			count * sizeof(T), static_cast<byte_allocator&>(m));

		std::uninitialized_copy_n(beg, count, reinterpret_cast<T*>(slots));
		return reinterpret_cast<T*>(slots);
	}

	template<typename U = T>
	vsm_always_inline T* _push_back(U const& value, size_t const count)
	{
		byte* const slots = vsm_detail_vector_operations::push_back(&m.c,
			count * sizeof(T), static_cast<byte_allocator&>(m));

		std::uninitialized_fill_n(reinterpret_cast<T*>(slots), count, value);
		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* _push_back_default(size_t const count)
	{
		byte* const slots = vsm_detail_vector_operations::push_back(&m.c,
			count * sizeof(T), static_cast<byte_allocator&>(m));

		std::uninitialized_default_construct_n(reinterpret_cast<T*>(slots), count);
		return reinterpret_cast<T*>(slots);
	}

	vsm_always_inline T* _push_back_uninitialized(size_t const count)
	{
		byte* const slots = vsm_detail_vector_operations::push_back(&m.c,
			count * sizeof(T), static_cast<byte_allocator&>(m));

		return reinterpret_cast<T*>(slots);
	}

	template<typename... Args>
	vsm_always_inline T& emplace_back(Args&&... args)
	{
		byte* const slot = vsm_detail_vector_operations::push_back(&m.c,
			sizeof(T), static_cast<byte_allocator&>(m));

		return *new(slot) T(vsm_forward(args)...);
	}

	vsm_always_inline void pop_back()
	{
		vsm_assert(!empty());
		vector_::pop_back<vsm_detail_vector_destroy_type>(&m.c, sizeof(T));
	}

	vsm_always_inline void _pop_back_n(size_t const count)
	{
		vsm_assert(count <= size());
		vector_::pop_back_n<vsm_detail_vector_destroy_type>(&m.c, count * sizeof(T));
	}

	vsm_always_inline void _pop_back_uninitialized(size_t const count)
	{
		vsm_assert(count <= size());
		vector_::pop_back<byte>(&m.c, count * sizeof(T));
	}

	vsm_always_inline T _pop_back_value()
	{
		vsm_assert(!empty());
		T value = vsm_move(reinterpret_cast<T*>(m.c.mid)[-1]);
		vector_::pop_back<vsm_detail_vector_destroy_type>(&m.c, sizeof(T));
		return value;
	}

	vsm_always_inline void resize(size_t const count)
	{
		vsm_detail_vector_operations::template resize<
			true, vsm_detail_vector_construct_type, vsm_detail_vector_destroy_type>(
			&m.c, count * sizeof(T), static_cast<byte_allocator&>(m));
	}

	vsm_always_inline void _resize_uninitialized(size_t const count)
	{
		vsm_detail_vector_operations::template resize<false, byte, byte>(
			&m.c, count * sizeof(T), static_cast<byte_allocator&>(m));
	}

	void resize(size_t const count, T const& value)
	{
		byte* const pos = vsm_detail_vector_operations::template resize<
			false, void, vsm_detail_vector_destroy_type>(&m.c,
			count * sizeof(T), static_cast<byte_allocator&>(m));

		std::uninitialized_fill(reinterpret_cast<T*>(pos),
			reinterpret_cast<T*>(m.c.mid), value);
	}

	vsm_always_inline void swap(vector& other)
	{
		if constexpr (!byte_allocator_traits::is_always_equal::value)
		{
			using namespace std;
			swap(static_cast<byte_allocator&>(m),
				static_cast<byte_allocator&>(other.m));
		}

		if constexpr (has_local_storage)
		{
			vsm_detail_vector_operations::swap(&m.c, &other.m.c,
				static_cast<byte_allocator&>(m),
				static_cast<byte_allocator&>(other.m));
		}
		else
		{
			std::swap(m.c, other.m.c);
		}
	}

	buffer_ref<T> _release_storage()
	{
		vsm_assert(is_dynamic<has_local_storage>(m.c, m.c.beg));
		T* const beg = reinterpret_cast<T*>(std::exchange(m.c.beg, nullptr));
	
		return
		{
			beg,
			reinterpret_cast<T*>(std::exchange(m.c.mid, nullptr)) - beg,
			reinterpret_cast<T*>(std::exchange(m.c.end, nullptr)) - beg,
		};
	}
	
	void _acquire_storage(buffer_ref<T> const buffer)
	{
		//TODO: destroy existing.
		static_assert(!sizeof(T));

		byte* const beg = reinterpret_cast<byte*>(buffer.beg);
		m.c = { beg, beg + buffer.size, beg + buffer.capacity };
	}

#undef vsm_detail_vector_storage_size
#undef vsm_detail_vector_construct_type
#undef vsm_detail_vector_destroy_type
#undef vsm_detail_vector_relocate_type
#undef vsm_detail_vector_copy_type
#undef vsm_detail_vector_operations
};

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline void swap(
	vector<T, Allocator, LocalCapacity>& lhs,
	vector<T, Allocator, LocalCapacity>& rhs)
{
	lhs.swap(rhs);
}

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline bool operator==(
	vector<T, Allocator, LocalCapacity> const& lhs,
	vector<T, Allocator, LocalCapacity> const& rhs)
{
	return equal<T>(&lhs.m.c, &rhs.m.c);
}

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline bool operator!=(
	vector<T, Allocator, LocalCapacity> const& lhs,
	vector<T, Allocator, LocalCapacity> const& rhs)
{
	return !equal<T>(&lhs.m.c, &rhs.m.c);
}

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline bool operator<(
	vector<T, Allocator, LocalCapacity> const& lhs,
	vector<T, Allocator, LocalCapacity> const& rhs)
{
	return compare<T>(&lhs.m.c, &rhs.m.c);
}

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline bool operator>(
	vector<T, Allocator, LocalCapacity> const& lhs,
	vector<T, Allocator, LocalCapacity> const& rhs)
{
	return compare<T>(&rhs.m.c, &lhs.m.c);
}

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline bool operator<=(
	vector<T, Allocator, LocalCapacity> const& lhs,
	vector<T, Allocator, LocalCapacity> const& rhs)
{
	return !compare<T>(&rhs.m.c, &lhs.m.c);
}

template<typename T, typename Allocator, size_t LocalCapacity>
vsm_always_inline bool operator>=(
	vector<T, Allocator, LocalCapacity> const& lhs,
	vector<T, Allocator, LocalCapacity> const& rhs)
{
	return !compare<T>(&lhs.m.c, &rhs.m.c);
}

} // namespace detail::vector_

template<typename T, typename Allocator = std::allocator<T>>
using vector = detail::vector_::vector<T, Allocator, 0>;

template<typename T, size_t LocalCapacity, typename Allocator = std::allocator<T>>
using small_vector = detail::vector_::vector<T, Allocator, LocalCapacity>;

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
	auto beg = v.begin();
	auto end = v.end();

	while (true)
	{
		while (true)
		{
			if (beg == end) goto exit;
			if (*beg == value) break;
			++beg;
		}

		while (true)
		{
			if (beg == --end) goto exit;
			if (*end != value) break;
		}

		*beg++ = vsm_move(*end);
	}

exit:
	typename detail::vector_::vector<T, A, C>::size_type n = v.end() - end;
	v._pop_back_n(n);
	return n;
}

template<typename T, typename A, size_t C, typename Pred>
typename detail::vector_::vector<T, A, C>::size_type erase_if_unstable(detail::vector_::vector<T, A, C>& v, Pred&& pred)
{
	auto beg = v.begin();
	auto end = v.end();

	while (true)
	{
		while (true)
		{
			if (beg == end) goto exit;
			if (pred(*beg)) break;
			++beg;
		}

		while (true)
		{
			if (beg == --end) goto exit;
			if (!pred(*end)) break;
		}

		*beg++ = vsm_move(*end);
	}

exit:
	typename detail::vector_::vector<T, A, C>::size_type n = v.end() - end;
	v._pop_back_n(n);
	return n;
}

} // namespace vsm

vsm_msvc_warning(pop)
