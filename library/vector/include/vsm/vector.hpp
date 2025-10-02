#pragma once

#include <vsm/sanitizer/address.h>
#include <vsm/algorithm/remove_unstable.hpp>
#include <vsm/allocator.hpp>
#include <vsm/exceptions.hpp>
#include <vsm/relocate.hpp>
#include <vsm/standard/stdexcept.hpp>

#include <cstddef>

namespace vsm {

template<typename T, allocator Allocator = default_allocator>
class small_vector_base;

template<typename T, size_t Capacity, allocator Allocator = default_allocator>
class small_vector;

namespace detail {

template<size_t Size>
struct _vector_padding
{
	unsigned char m_padding[Size];
};

template<>
struct _vector_padding<0>
{
};

template<typename T, size_t Alignment>
using _vector_padding_before = _vector_padding<Alignment - sizeof(T) % Alignment>;

template<typename T1, typename T2, size_t Alignment>
using _vector_padding_between = _vector_padding<Alignment - (sizeof(T1) + sizeof(T2)) % Alignment>;

struct _vector_size
{
	size_t m_size;
};

struct _vector_capacity
{
	size_t m_capacity : sizeof(size_t) * 8 - 1;
	size_t m_is_local : 1;
};

struct _vector
	: _vector_size
	, _vector_capacity
{
	void _set(size_t const capacity, bool const is_local)
	{
		vsm_gcc_diagnostic(push)
		vsm_gcc_diagnostic(ignored "-Wconversion")

		m_capacity = capacity;
		m_is_local = is_local;

		vsm_gcc_diagnostic(pop)
	}

	template<typename T>
	T* _get_ptr() const
	{
		return m_is_local ? _get_storage<T>() : _get_storage_ptr<T>();
	}

	template<typename T>
	T* _get_storage() const
	{
		return reinterpret_cast<T*>(const_cast<_vector*>(this) + 1);
	}

	template<typename T>
	T* _get_storage_ptr() const
	{
		return *reinterpret_cast<T* const*>(this + 1);
	}

	template<typename T>
	void _set_storage_ptr(T* const ptr)
	{
		*reinterpret_cast<T**>(this + 1) = ptr;
	}
};

template<typename A>
struct _vector_allocator
{
	vsm_no_unique_address A m_allocator;

	_vector_allocator() = default;

	_vector_allocator(auto&& allocator)
		: m_allocator(vsm_forward(allocator))
	{
	}
};

template<typename A>
struct _vector_base
	: _vector_allocator<A>
	, _vector_padding_between<A, _vector, std::max(alignof(A), alignof(_vector))>
	, _vector
{
	using _vector_allocator<A>::_vector_allocator;
};

template<typename T, typename A, size_t Capacity>
class _vector_storage
	: _vector_padding_before<small_vector_base<T, A>, std::max(alignof(T), alignof(T*))>
	, public small_vector_base<T, A>
{
	using small_vector_base<T, A>::small_vector_base;

	union
	{
		T* m_storage_ptr;
		alignas(T) unsigned char m_storage[Capacity * sizeof(T)];
	};

	template<typename T2, size_t C2, allocator A2>
	friend class vsm::small_vector;
};

template<typename T, typename A>
class _vector_storage<T, A, 0>
	: _vector_padding_before<small_vector_base<T, A>, alignof(T*)>
	, public small_vector_base<T, A>
{
	using small_vector_base<T, A>::small_vector_base;

	T* m_storage_ptr;

	template<typename T2, size_t C2, allocator A2>
	friend class vsm::small_vector;
};

#if vsm_has_address_sanitizer
template<typename T>
void _vector_apply_annotation(
	T* const beg,
	T* const end,
	T* const old_mid,
	T* const new_mid) noexcept
{
	__sanitizer_annotate_contiguous_container(
		beg,
		end,
		old_mid,
		new_mid);
}

template<typename T>
void _vector_apply_annotation(
	T* const ptr,
	size_t const capacity,
	size_t const old_size,
	size_t const new_size) noexcept
{
	_vector_apply_annotation(
		ptr,
		ptr + capacity,
		ptr + old_size,
		ptr + new_size);
}

template<typename T>
void _vector_create_annotation(_vector& vector) noexcept
{
	_vector_apply_annotation<T>(
		vector.template _get_ptr<T>(),
		vector.m_capacity,
		vector.m_capacity,
		vector.m_size);
}

template<typename T>
void _vector_remove_annotation(_vector& vector) noexcept
{
	_vector_apply_annotation<T>(
		vector.template _get_ptr<T>(),
		vector.m_capacity,
		vector.m_size,
		vector.m_capacity);
}

template<typename T>
void _vector_modify_annotation(
	_vector& vector,
	size_t const old_size,
	size_t const new_size) noexcept
{
	_vector_apply_annotation(vector.template _get_ptr<T>(), vector.m_capacity, old_size, new_size);
}

#	define vsm_vector_annotate(function, ...) \
		(detail::_vector_ ## function ## _annotation<T>(__VA_ARGS__))

#else

#	define vsm_vector_annotate(function, ...) \
		((void)sizeof((__VA_ARGS__)))

#endif

inline bool _vector_has_ptr(_vector const& vector)
{
	return vector.m_size != 0 && !vector.m_is_local;
}

template<typename T, typename A>
T* _vector_allocate(_vector_base<A>& vector, size_t& new_capacity)
{
	vsm_assert(new_capacity != 0);

	auto const allocation = vsm::allocate_or_throw(
		vector.m_allocator,
		new_capacity * sizeof(T));

	new_capacity = allocation.size / sizeof(T);
	return static_cast<T*>(allocation.storage);
}

template<typename T, typename A>
T* _vector_reserve_at(_vector_base<A>& vector, size_t const index, size_t const count)
{
	size_t const old_capacity = vector.m_capacity;
	size_t /* */ new_capacity = std::max(old_capacity * 3 / 2, old_capacity + count);

	size_t const old_allocation_size = old_capacity * sizeof(T);
	size_t const min_allocation_size = new_capacity * sizeof(T);

	size_t const old_size = vector.m_size;
	size_t const new_size = vector.m_size + count;

	T* const old_ptr = vector.template _get_ptr<T>();
	T* const old_pos = old_ptr + index;
	T* const old_end = old_ptr + old_size;

	if constexpr (allocators::has_resize_v<A>)
	{
		size_t const new_allocation_size = vector.m_allocator.resize(
			allocation(old_ptr, old_allocation_size),
			min_allocation_size);

		if (new_allocation_size != 0)
		{
			new_capacity = new_allocation_size / sizeof(T);

			T* const new_end = old_ptr + new_size;

			vsm_vector_annotate(
				apply,
				old_ptr,
				old_ptr + new_capacity,
				old_end,
				new_end);

			if (index != old_size)
			{
				vsm::uninitialized_relocate_backward(
					old_pos,
					old_end,
					new_end);
			}

			vector._set(new_capacity, false);

			return old_pos;
		}
	}

	T* const new_ptr = _vector_allocate<T>(vector, new_capacity);
	T* const new_pos = new_ptr + index;

	if (old_size != 0)
	{
		vsm::uninitialized_relocate(old_ptr, old_pos, new_ptr);

		if (index != old_size)
		{
			vsm::uninitialized_relocate(old_pos, old_end, new_pos + count);
		}
	}

	vsm_vector_annotate(remove, vector);

	if (_vector_has_ptr(vector))
	{
		vector.m_allocator.deallocate(allocation(old_ptr, old_allocation_size));
	}

	vector._set(new_capacity, false);
	vector._set_storage_ptr(new_ptr);
	vsm_vector_annotate(create, vector);

	return new_pos;
}

template<typename T, typename A>
T* _vector_reserve(_vector_base<A>& vector, size_t const min_capacity)
{
	return _vector_reserve_at<T>(vector, vector.m_size, min_capacity - vector.m_size);
}

template<typename T, typename A>
void _vector_shrink_to_fit(_vector_base<A>& vector, size_t const storage_capacity)
{
	vsm_assert(!vector.m_is_local);

	vsm_vector_annotate(remove, vector);

	T* const old_ptr = vector.template _get_storage_ptr<T>();
	size_t const old_allocation_size = vector.m_capacity * sizeof(T);

	if (vector.m_size <= storage_capacity)
	{
		T* const new_ptr = vector.template _get_storage<T>();

		vsm::uninitialized_relocate(
			old_ptr,
			old_ptr + vector.m_size,
			new_ptr);

		vector._set(storage_capacity, true);
		vsm_vector_annotate(create, vector);
	}
	else
	{
		// TODO: Implement vector::shrink_to_fit using in-place resizing.

#if 0
		if constexpr (allocators::has_resize_v<A>)
		{
			size_t const new_allocation_size = vector.m_allocator.resize(
				allocation(old_ptr, old_allocation_size),
				vector.m_size * sizeof(T));

			
		}
#endif

		size_t new_capacity = vector.m_size;
		T* const new_ptr = detail::_vector_allocate<T>(vector, new_capacity);

		vsm::uninitialized_relocate(
			old_ptr,
			old_ptr + vector.m_size,
			new_ptr);

		vector._set(new_capacity, false);
		vector._set_storage_ptr(new_ptr);
		vsm_vector_annotate(create, vector);
	}

	vector.m_allocator.deallocate(allocation(old_ptr, old_allocation_size));
}

template<typename T>
void _vector_clear(_vector& vector)
{
	size_t const old_size = vector.m_size;

	T* const ptr = vector._get_ptr<T>();
	std::destroy_n(ptr, old_size);

	vsm_vector_annotate(modify, vector, old_size, 0);
	vector.m_size = 0;
}

template<typename T, typename A>
T* _vector_resize(_vector_base<A>& vector, size_t const new_size)
{
	size_t const old_size = vector.m_size;

	T* hole = nullptr;

	/**/ if (vsm_likely(new_size > old_size))
	{
		hole = _vector_reserve<T>(vector, new_size);
	}
	else if (vsm_likely(new_size < old_size))
	{
		std::destroy_n(vector.template _get_ptr<T>() + old_size, old_size - new_size);
	}

	vsm_vector_annotate(modify, vector, old_size, new_size);
	vector.m_size = new_size;

	return hole;
}

template<typename T, typename A>
T* _vector_push_back(_vector_base<A>& vector, size_t const count)
{
	size_t const old_size = vector.m_size;
	size_t const new_size = old_size + count;

	T* const hole = vsm_likely(new_size <= vector.m_capacity)
		? vector.template _get_ptr<T>() + old_size
		: _vector_reserve_at<T>(vector, old_size, count);

	vsm_vector_annotate(modify, vector, old_size, new_size);
	vector.m_size = new_size;

	return hole;
}

template<typename T>
void _vector_pop_back(_vector& vector, size_t const count)
{
	size_t const old_size = vector.m_size;
	size_t const new_size = old_size - count;

	vsm_vector_annotate(modify, vector, old_size, new_size);
	vector.m_size = new_size;
}

template<typename T, typename A>
T* _vector_insert(_vector_base<A>& vector, T* const pos, size_t const count, bool const stable = true)
{
	T* const old_ptr = vector.template _get_ptr<T>();
	size_t const index = static_cast<size_t>(pos - old_ptr);

	size_t const old_size = vector.m_size;
	size_t const new_size = old_size + count;

	detail::_vector_push_back<T>(vector, count);
	T* const new_ptr = vector.template _get_ptr<T>();
	T* const new_pos = new_ptr + index;

	if (index != old_size)
	{
		vsm::uninitialized_relocate_backward(
			new_pos,
			stable
				? new_ptr + old_size
				: new_pos + count,
			new_ptr + new_size);
	}

	return new_pos;
}

template<typename T>
T* _vector_erase(_vector& vector, T* const hole_begin, T* const hole_end, bool const stable = true)
{
	size_t const count = static_cast<size_t>(hole_end - hole_begin);

	size_t const old_size = vector.m_size;
	size_t const new_size = old_size - count;

	T* const ptr = vector._get_ptr<T>();
	T* const end = ptr + old_size;

	if (hole_end != end)
	{
		vsm::uninitialized_relocate(
			stable
				? hole_end
				: ptr + new_size,
			end,
			hole_begin);
	}

	vsm_vector_annotate(modify, vector, old_size, new_size);
	vector.m_size = new_size;

	return hole_begin;
}

template<typename T>
bool _vector_equal(_vector const& lhs, _vector const& rhs)
{
	T* const lhs_ptr = lhs._get_ptr<T>();
	T* const lhs_end = lhs_ptr + lhs.m_size;

	T* const rhs_ptr = rhs._get_ptr<T>();
	T* const rhs_end = rhs_ptr + rhs.m_size;

	if (lhs_end - lhs_ptr != rhs_end - rhs_ptr)
	{
		return false;
	}

	return std::equal(lhs_ptr, lhs_end, rhs_ptr, rhs_end);
}

template<typename T>
auto _vector_compare(_vector const& lhs, _vector const& rhs)
{
	T* const lhs_ptr = lhs._get_ptr<T>();
	T* const lhs_end = lhs_ptr + lhs.m_size;

	T* const rhs_ptr = rhs._get_ptr<T>();
	T* const rhs_end = rhs_ptr + rhs.m_size;

	return std::lexicographical_compare_three_way(lhs_ptr, lhs_end, rhs_ptr, rhs_end);
}

template<typename T>
void _vector_initialize(_vector& vector, size_t const capacity)
{
	vector.m_size = 0;
	vector._set(capacity, capacity != 0);

	if (capacity != 0)
	{
		vector.template _set_storage_ptr<T>(nullptr);
	}

	vsm_vector_annotate(create, vector);
}

template<typename T, typename A>
void _vector_destroy(_vector_base<A>& vector)
{
	vsm_vector_annotate(remove, vector);

	if (size_t const capacity = vector.m_capacity)
	{
		size_t const size = vector.m_size;

		if (vector.m_is_local)
		{
			std::destroy_n(vector.template _get_storage<T>(), size);
		}
		else
		{
			T* const ptr = vector.template _get_storage_ptr<T>();
			std::destroy_n(ptr, size);
			vector.m_allocator.deallocate(allocation(ptr, capacity * sizeof(T)));
		}
	}
}

template<typename T, typename A>
void _vector_move(_vector_base<A>& dst, _vector_base<A>& src)
{
	if (src.m_is_local)
	{
		if (size_t const size = src.m_size)
		{
			T* const dst_ptr = dst.template _get_storage<T>();
			T* const src_ptr = src.template _get_storage<T>();

			vsm::uninitialized_relocate(
				src_ptr,
				src_ptr + size,
				dst_ptr);
		}
	}
	else
	{
		dst.template _set_storage_ptr<T>(src.template _get_storage_ptr<T>());
	}

	static_cast<_vector&>(dst) = src;
	vsm_vector_annotate(create, dst);
	vsm_vector_annotate(remove, src);
}

template<typename T, typename A>
void _vector_move_assign(_vector_base<A>& dst, _vector_base<A>& src)
{
	detail::_vector_destroy<T>(dst);
	dst.m_allocator = src.m_allocator;
	detail::_vector_move<T>(dst, src);
}

template<typename T>
void _vector_swap_1(_vector& lhs, _vector& rhs)
{
	T* const lhs_ptr = lhs._get_storage_ptr<T>();
	T* const rhs_ptr = rhs._get_storage_ptr<T>();

	lhs._set_storage_ptr(rhs_ptr);
	rhs._set_storage_ptr(lhs_ptr);

	std::swap(lhs, rhs);
}

template<typename T>
void _vector_swap_2(_vector& small, _vector& large)
{
	T* const large_ptr = large._get_storage_ptr<T>();

	T* const old_small_ptr = large._get_storage<T>();
	T* const new_small_ptr = large._get_storage<T>();

	vsm::uninitialized_relocate(
		old_small_ptr,
		old_small_ptr + small.m_size,
		new_small_ptr);

	vsm_vector_annotate(remove, small);

	small._set_storage_ptr<T>(large_ptr);
	std::swap(small, large);

	vsm_vector_annotate(create, large);
}

template<typename T, bool MayBeLocal, typename A>
void _vector_swap(_vector_base<A>& lhs, _vector_base<A>& rhs)
{
	if constexpr (MayBeLocal)
	{
		if (lhs.m_is_local && rhs.m_is_local)
		{
			T* lhs_ptr = lhs.template _get_storage<T>();
			T* const lhs_end = lhs_ptr + lhs.m_size;

			T* rhs_ptr = rhs.template _get_storage<T>();
			T* const rhs_end = rhs_ptr + rhs.m_size;

			while (lhs_ptr != lhs_end && rhs_ptr != rhs_end)
			{
				using std::swap;
				swap(*lhs_ptr++, *rhs_ptr++);
			}

			/**/ if (lhs_ptr != lhs_end)
			{
				vsm_vector_annotate(
					modify,
					lhs,
					lhs.m_size,
					rhs.m_size);

				vsm::uninitialized_relocate(
					lhs_ptr,
					lhs_end,
					rhs_ptr);

				vsm_vector_annotate(
					modify,
					rhs,
					rhs.m_size,
					lhs.m_size);
			}
			else if (rhs_ptr != rhs_end)
			{
				vsm_vector_annotate(
					modify,
					rhs,
					rhs.m_size,
					lhs.m_size);

				vsm::uninitialized_relocate(
					rhs_ptr,
					rhs_end,
					lhs_ptr);

				vsm_vector_annotate(
					modify,
					lhs,
					lhs.m_size,
					rhs.m_size);
			}

			std::swap(lhs.m_size, rhs.m_size);
		}
		else if (lhs.m_is_local)
		{
			detail::_vector_swap_2<T>(lhs, rhs);
		}
		else if (rhs.m_is_local)
		{
			detail::_vector_swap_2<T>(rhs, lhs);
		}
		else
		{
			detail::_vector_swap_1<T>(lhs, rhs);
		}
	}
	else
	{
		detail::_vector_swap_1<T>(lhs, rhs);
	}

	using std::swap;
	swap(lhs.m_allocator, rhs.m_allocator);
}

} // namespace detail

template<typename T, allocator Allocator>
class small_vector_base : detail::_vector_base<Allocator>
{
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


	[[nodiscard]] Allocator const& get_allocator() const noexcept
	{
		return this->m_allocator;
	}

	[[nodiscard]] T& operator[](size_t const index)
	{
		vsm_assert(index < this->m_size); // PRECONDITION
		return this->template _get_ptr<T>()[index];
	}

	[[nodiscard]] T const& operator[](size_t const index) const
	{
		vsm_assert(index < this->m_size); // PRECONDITION
		return this->template _get_ptr<T>()[index];
	}

	[[nodiscard]] T& at(size_t const index)
	{
		if (index < this->m_size)
		{
			vsm_except_throw_or_terminate(std::out_of_range("vector index out of range"));
		}
		return this->template _get_ptr<T>()[index];
	}

	[[nodiscard]] T const& at(size_t const index) const
	{
		if (index < this->m_size)
		{
			vsm_except_throw_or_terminate(std::out_of_range("vector index out of range"));
		}
		return this->template _get_ptr<T>()[index];
	}

	[[nodiscard]] T& front()
	{
		vsm_assert(this->m_size != 0);
		return this->template _get_ptr<T>()[0];
	}

	[[nodiscard]] T const& front() const
	{
		vsm_assert(this->m_size != 0);
		return this->template _get_ptr<T>()[0];
	}

	[[nodiscard]] T& back()
	{
		vsm_assert(this->m_size != 0);
		return this->template _get_ptr<T>()[this->m_size - 1];
	}

	[[nodiscard]] T const& back() const
	{
		vsm_assert(this->m_size != 0);
		return this->template _get_ptr<T>()[this->m_size - 1];
	}

	[[nodiscard]] T* data() noexcept
	{
		return this->template _get_ptr<T>();
	}

	[[nodiscard]] T const* data() const noexcept
	{
		return this->template _get_ptr<T>();
	}

	[[nodiscard]] iterator begin() noexcept
	{
		return iterator(this->template _get_ptr<T>());
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator(this->template _get_ptr<T>());
	}

	[[nodiscard]] const_iterator cbegin() const noexcept
	{
		return const_iterator(this->template _get_ptr<T>());
	}

	[[nodiscard]] iterator end() noexcept
	{
		return iterator(this->template _get_ptr<T>() + this->m_size);
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator(this->template _get_ptr<T>() + this->m_size);
	}

	[[nodiscard]] const_iterator cend() const noexcept
	{
		return const_iterator(this->template _get_ptr<T>() + this->m_size);
	}

	[[nodiscard]] reverse_iterator rbegin() noexcept
	{
		return std::make_reverse_iterator(end());
	}

	[[nodiscard]] const_reverse_iterator rbegin() const noexcept
	{
		return std::make_reverse_iterator(end());
	}

	[[nodiscard]] const_reverse_iterator crbegin() const noexcept
	{
		return std::make_reverse_iterator(end());
	}

	[[nodiscard]] reverse_iterator rend() noexcept
	{
		return std::make_reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator rend() const noexcept
	{
		return std::make_reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator crend() const noexcept
	{
		return std::make_reverse_iterator(begin());
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return this->m_size == 0;
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return this->m_size;
	}

	[[nodiscard]] size_t max_size() const noexcept
	{
		return std::numeric_limits<size_t>::max() / 2 / sizeof(T);
	}

	[[nodiscard]] size_t capacity() const noexcept
	{
		return this->m_capacity;
	}

	void reserve(size_t const min_capacity)
	{
		static_assert(is_nothrow_relocatable_v<T>);

		if (min_capacity > this->m_capacity)
		{
			detail::_vector_reserve<T>(*this, min_capacity);
		}
	}

	void clear()
	{
		if (this->m_size != 0)
		{
			detail::_vector_clear<T>(*this);
		}
	}

	iterator erase(const_iterator const begin)
	{
		return erase(begin, begin + 1);
	}

	iterator erase(const_iterator const begin, const_iterator const end)
	{
		vsm_assert(begin >= this->template _get_ptr<T>());
		vsm_assert(begin <= end);
		vsm_assert(end <= this->template _get_ptr<T>() + this->m_size);

		if (begin != end)
		{
			T* const hole_ptr = const_cast<T*>(begin);
			T* const hole_end = const_cast<T*>(end);
			std::destroy(hole_ptr, hole_end);

			detail::_vector_erase<T>(*this, hole_ptr, hole_end);
		}

		return const_cast<T*>(begin);
	}

	iterator unstable_erase(const_iterator const begin)
	{
		return unstable_erase(begin, begin + 1);
	}

	iterator unstable_erase(const_iterator const begin, const_iterator const end)
	{
		vsm_assert(begin >= this->template _get_ptr<T>());
		vsm_assert(begin <= end);
		vsm_assert(end <= this->template _get_ptr<T>() + this->m_size);

		if (begin != end)
		{
			T* const hole_ptr = const_cast<T*>(begin);
			T* const hole_end = const_cast<T*>(end);
			std::destroy(hole_ptr, hole_end);

			detail::_vector_erase<T>(*this, hole_ptr, hole_end, /* stable: */ false);
		}

		return const_cast<T*>(begin);
	}

	template<std::convertible_to<T> U = T>
	vsm_always_inline iterator insert(const_iterator const pos, U&& value)
	{
		return emplace(pos, vsm_forward(value));
	}

	template<typename... Args>
	iterator emplace(const_iterator const pos, Args&&... args)
		requires std::constructible_from<T, Args...>
	{
		static_assert(is_nothrow_relocatable_v<T>);

		vsm_assert(pos >= this->template _get_ptr<T>());
		vsm_assert(pos <= this->template _get_ptr<T>() + this->m_size);

		T* const hole = detail::_vector_insert<T>(*this, const_cast<T*>(pos), 1);

		vsm_except_try
		{
			return iterator(::new (hole) T(vsm_forward(args)...));
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_constructible_v<T, Args...>)
			{
				detail::_vector_erase<T>(*this, hole, hole + 1);
			}

			vsm_except_rethrow;
		}
	}

	iterator insert_default(const_iterator const pos, size_t const count)
	{
		static_assert(is_nothrow_relocatable_v<T>);

		vsm_assert(pos >= this->template _get_ptr<T>());
		vsm_assert(pos <= this->template _get_ptr<T>() + this->m_size);

		T* const hole = detail::_vector_insert<T>(
			*this,
			const_cast<T*>(pos),
			count);

		vsm_except_try
		{
			std::uninitialized_default_construct_n(
				hole,
				count);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_default_constructible_v<T>)
			{
				detail::_vector_erase<T>(*this, hole, count);
			}

			vsm_except_rethrow;
		}
	}

	iterator insert(const_iterator const pos, size_t const count, T const& value)
	{
		static_assert(is_nothrow_relocatable_v<T>);

		vsm_assert(pos >= this->template _get_ptr<T>());
		vsm_assert(pos <= this->template _get_ptr<T>() + this->m_size);

		T* const hole = detail::_vector_insert<T>(
			*this,
			const_cast<T*>(pos),
			count);

		vsm_except_try
		{
			std::uninitialized_fill_n(
				hole,
				count,
				value);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_copy_constructible_v<T>)
			{
				detail::_vector_erase<T>(*this, hole, count);
			}

			vsm_except_rethrow;
		}
	}

	template<std::input_iterator Iterator>
	vsm_always_inline iterator insert(const_iterator const pos, Iterator const begin, Iterator const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		return insert_range(pos, begin, end);
	}

	template<std::input_iterator Iterator>
	iterator insert_n(const_iterator const pos, Iterator const begin, size_t const count)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		static_assert(is_nothrow_relocatable_v<T>);

		vsm_assert(pos >= this->template _get_ptr<T>());
		vsm_assert(pos <= this->template _get_ptr<T>() + this->m_size);

		T* const hole = detail::_vector_insert<T>(
			*this,
			const_cast<T*>(pos),
			count);

		vsm_except_try
		{
			std::uninitialized_copy_n(
				begin,
				count,
				hole);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_convertible_v<std::iter_reference_t<Iterator>, T>)
			{
				detail::_vector_erase<T>(*this, hole, count);
			}

			vsm_except_rethrow;
		}
	}

	template<std::ranges::input_range R>
	iterator insert_range(const_iterator const pos, R&& range)
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	{
		if constexpr (std::ranges::sized_range<R>)
		{
			return insert_n(
				pos,
				std::ranges::begin(range),
				std::ranges::size(range));
		}
		else
		{
			return insert_range(
				pos,
				std::ranges::begin(range),
				std::ranges::end(range));
		}
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	iterator insert_range(const_iterator const pos, Iterator const begin, Sentinel const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		if constexpr (std::sized_sentinel_for<Sentinel, Iterator>)
		{
			return insert_n(
				pos,
				begin,
				static_cast<size_t>(end - begin));
		}
		else
		{
			size_t const index = static_cast<size_t>(begin - this->template _get_ptr<T>());

			for (size_t i = index; begin != end; ++i, (void)++begin)
			{
				insert(pos, this->template _get_ptr<T>() + i, *begin);
			}

			return this->template _get_ptr<T>() + index;
		}
	}

	template<std::forward_iterator Iterator>
	iterator insert_range(const_iterator const pos, Iterator const begin, Iterator const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		return insert_n(
			pos,
			begin,
			static_cast<size_t>(std::distance(begin, end)));
	}

	template<std::convertible_to<T> U = T>
	vsm_always_inline T& push_back(U&& value)
	{
		return emplace_back(vsm_forward(value));
	}

	iterator push_back_n(size_t const count, T const& value)
		requires std::is_copy_constructible_v<T>
	{
		T* const hole = detail::_vector_push_back<T>(*this, count);

		vsm_except_try
		{
			std::uninitialized_fill_n(
				hole,
				count,
				value);
		}
			vsm_except_catch(...)
		{
			if constexpr (!std::is_nothrow_copy_constructible_v<T>)
			{
				detail::_vector_pop_back<T>(*this, count);
			}

			vsm_except_rethrow;
		}

		return iterator(hole);
	}

	iterator push_back_default(size_t const count = 1)
		requires std::is_default_constructible_v<T>
	{
		T* const hole = detail::_vector_push_back<T>(*this, count);

		vsm_except_try
		{
			std::uninitialized_default_construct_n(
				hole,
				count);
		}
			vsm_except_catch(...)
		{
			if constexpr (!std::is_nothrow_default_constructible_v<T>)
			{
				detail::_vector_pop_back<T>(*this, count);
			}

			vsm_except_rethrow;
		}

		return iterator(hole);
	}

	template<typename... Args>
	T& emplace_back(Args&&... args)
		requires std::constructible_from<T, Args...>
	{
		static_assert(is_nothrow_relocatable_v<T>);

		T* const hole = detail::_vector_push_back<T>(*this, 1);

		vsm_except_try
		{
			return *::new (hole) T(vsm_forward(args)...);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_constructible_v<T, Args...>)
			{
				detail::_vector_pop_back<T>(*this, 1);
			}

			vsm_except_rethrow;
		}
	}

	template<std::ranges::input_range R>
	iterator append_range(R&& range)
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	{
		if constexpr (std::ranges::sized_range<R>)
		{
			return append_n(std::ranges::begin(range), std::ranges::size(range));
		}
		else
		{
			return append_range(std::ranges::begin(range), std::ranges::end(range));
		}
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	iterator append_range(Iterator begin, Sentinel const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		if constexpr (std::sized_sentinel_for<Sentinel, Iterator>)
		{
			return append_n(
				begin,
				static_cast<size_t>(end - begin));
		}
		else
		{
			size_t const old_size = this->m_size;

			for (; begin != end; ++begin)
			{
				push_back(*begin);
			}

			return iterator(this->template _get_ptr<T>() + old_size);
		}
	}

	template<std::forward_iterator Iterator>
	iterator append_range(Iterator const begin, Iterator const end)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		return append_n(
			begin,
			static_cast<size_t>(std::distance(begin, end)));
	}

	template<std::input_iterator Iterator>
	iterator append_n(Iterator begin, size_t const count)
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
	{
		T* const hole = detail::_vector_push_back<T>(*this, count);

		vsm_except_try
		{
			std::uninitialized_copy_n(
				begin,
				count,
				hole);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_convertible_v<std::iter_reference_t<Iterator>, T>)
			{
				detail::_vector_pop_back<T>(*this, count);
			}

			vsm_except_rethrow;
		}

		return iterator(hole);
	}

	void pop_back()
	{
		vsm_assert(this->m_size != 0);
		std::destroy_at(this->template _get_ptr<T>() + (this->m_size - 1));
		detail::_vector_pop_back<T>(*this, 1);
	}

	void pop_back(size_t const count)
	{
		vsm_assert(this->m_size >= count);
		std::destroy_n(this->template _get_ptr<T>() + (this->m_size - count), count);
		detail::_vector_pop_back<T>(*this, count);
	}

	[[nodiscard]] T pop_back_value()
	{
		vsm_assert(this->m_size != 0);

		T* const hole = this->template _get_ptr<T>() + (this->m_size - 1);

		struct popper_back
		{
			detail::_vector& vector;
			T* const hole;
			bool pop_back = true;

			popper_back(detail::_vector& vector, T* const hole)
				: vector(vector)
				, hole(hole)
			{
			}

			popper_back(popper_back const&) = delete;
			popper_back& operator=(popper_back const&) = delete;

			~popper_back()
			{
				if (pop_back)
				{
					std::destroy_at(hole);
					detail::_vector_pop_back<T>(vector, 1);
				}
			}
		};

		[[maybe_unused]] popper_back popper(*this, hole);

		vsm_except_try
		{
			return vsm_move(*hole);
		}
		vsm_except_catch (...)
		{
			if constexpr (!std::is_nothrow_move_constructible_v<T>)
			{
				popper.pop_back = false;
			}

			vsm_except_rethrow;
		}
	}

	void resize(size_t const new_size)
		requires std::is_default_constructible_v<T>
	{
		static_assert(is_nothrow_relocatable_v<T>);

		size_t const old_size = this->m_size;

		if (T* const hole = detail::_vector_resize<T>(*this, new_size))
		{
			vsm_except_try
			{
				std::uninitialized_value_construct_n(
					hole,
					new_size - old_size);
			}
			vsm_except_catch (...)
			{
				if constexpr (!std::is_nothrow_default_constructible_v<T>)
				{
					detail::_vector_pop_back<T>(*this, new_size - old_size);
				}

				vsm_except_rethrow;
			}
		}
	}

	void resize_default(size_t const new_size)
		requires std::is_default_constructible_v<T>
	{
		static_assert(is_nothrow_relocatable_v<T>);

		size_t const old_size = this->m_size;

		if (T* const hole = detail::_vector_resize<T>(*this, new_size))
		{
			vsm_except_try
			{
				std::uninitialized_default_construct_n(
					hole,
					new_size - old_size);
			}
			vsm_except_catch (...)
			{
				if constexpr (!std::is_nothrow_default_constructible_v<T>)
				{
					detail::_vector_pop_back<T>(*this, new_size - old_size);
				}

				vsm_except_rethrow;
			}
		}
	}

	void resize(size_t const new_size, T const& value)
		requires std::is_default_constructible_v<T>
	{
		static_assert(is_nothrow_relocatable_v<T>);

		size_t const old_size = this->m_size;

		if (T* const hole = detail::_vector_resize<T>(*this, new_size))
		{
			vsm_except_try
			{
				std::uninitialized_fill_n(
					hole,
					new_size - old_size,
					value);
			}
			vsm_except_catch (...)
			{
				if constexpr (!std::is_nothrow_copy_constructible_v<T>)
				{
					detail::_vector_pop_back<T>(*this, new_size - old_size);
				}

				vsm_except_rethrow;
			}
		}
	}

	friend bool operator==(small_vector_base const& lhs, small_vector_base const& rhs)
	{
		return detail::_vector_equal(lhs, rhs);
	}

	friend auto operator<=>(small_vector_base const& lhs, small_vector_base const& rhs)
	{
		return detail::_vector_compare(lhs, rhs);
	}

private:
	using detail::_vector_base<Allocator>::_vector_base;

	small_vector_base(small_vector_base const&) = default;
	small_vector_base& operator=(small_vector_base const&) = default;

	~small_vector_base() = default;

	template<typename T2, typename A2, size_t C2>
	friend class detail::_vector_storage;

	template<typename T2, size_t C2, allocator A2>
	friend class vsm::small_vector;
};

template<typename T, typename A, typename U>
typename small_vector_base<T, A>::size_type erase(
	small_vector_base<T, A>& v,
	U const& value)
{
	auto it = std::remove(v.begin(), v.end(), value);
	typename small_vector_base<T, A>::size_type n = v.end() - it;
	v.pop_back(n);
	return n;
}

template<typename T, typename A, typename Predicate>
typename small_vector_base<T, A>::size_type erase_if(
	small_vector_base<T, A>& v,
	Predicate&& predicate)
{
	auto it = std::remove_if(v.begin(), v.end(), vsm_forward(predicate));
	typename small_vector_base<T, A>::size_type n = v.end() - it;
	v.pop_back(n);
	return n;
}

template<typename T, typename A, typename U>
typename small_vector_base<T, A>::size_type erase_unstable(
	small_vector_base<T, A>& v,
	U const& value)
{
	auto it = remove_unstable(v.begin(), v.end(), value);
	typename small_vector_base<T, A>::size_type n = v.end() - it;
	v.pop_back(n);
	return n;
}

template<typename T, typename A, typename Predicate>
typename small_vector_base<T, A>::size_type erase_if_unstable(
	small_vector_base<T, A>& v,
	Predicate&& predicate)
{
	auto it = remove_if_unstable(v.begin(), v.end(), vsm_forward(predicate));
	typename small_vector_base<T, A>::size_type n = v.end() - it;
	v.pop_back(n);
	return n;
}


template<typename T, size_t Capacity, allocator Allocator>
class small_vector : public detail::_vector_storage<T, Allocator, Capacity>
{
	using base = detail::_vector_storage<T, Allocator, Capacity>;

public:
	small_vector() noexcept(std::is_nothrow_default_constructible_v<Allocator>)
	{
		_initialize();
	}

	explicit small_vector(Allocator const& allocator)
		noexcept(std::is_nothrow_copy_constructible_v<Allocator>)
		: base(allocator)
	{
		_initialize();
	}

	small_vector(small_vector&& other) noexcept
		requires allocators::is_propagatable_v<Allocator>
		: base(other.m_allocator)
	{
		static_assert(is_nothrow_relocatable_v<T>);
		static_assert(std::is_nothrow_copy_constructible_v<Allocator>);

		detail::_vector_move<T>(*this, other);
		other._initialize();
	}

	small_vector& operator=(small_vector&& other) & noexcept
		requires allocators::is_propagatable_v<Allocator>
	{
		static_assert(is_nothrow_relocatable_v<T>);
		static_assert(std::is_nothrow_copy_constructible_v<Allocator>);

		if (vsm_likely(this != &other))
		{
			detail::_vector_move_assign<T>(*this, other);
			other._initialize();
		}

		return *this;
	}

	~small_vector()
	{
		detail::_vector_destroy<T>(*this);
	}

	void shrink_to_fit()
	{
		static_assert(is_nothrow_relocatable_v<T>);

		if (std::max(this->m_size, Capacity) < this->m_capacity)
		{
			detail::_vector_shrink_to_fit<T>(*this, Capacity);
		}
	}

	void swap(small_vector& other) noexcept
		requires allocators::is_propagatable_v<Allocator>
	{
		static_assert(std::is_nothrow_swappable_v<Allocator>);

		detail::_vector_swap<T, Capacity != 0>(*this, other);
	}

	[[nodiscard]] T* release()
		requires (Capacity == 0)
	{
		T* const ptr = this->_get_storage_ptr<T>();
		vsm_vector_annotate(remove, *this);
		detail::_vector_initialize<T>(*this, /* Capacity: */ 0);
		return ptr;
	}

private:
	void _initialize() noexcept
	{
		using base_type = detail::_vector_base<Allocator>;

		static_assert(offsetof(base_type, m_allocator) == 0);
		constexpr size_t storage_offset = offsetof(small_vector, m_allocator) + sizeof(base_type);
		static_assert(storage_offset == offsetof(small_vector, m_storage_ptr));

		if constexpr (Capacity != 0)
		{
			static_assert(storage_offset == offsetof(small_vector, m_storage));
		}

		detail::_vector_initialize<T>(*this, Capacity);
	}
};

template<typename T, typename Allocator = default_allocator>
using vector = small_vector<T, 0, Allocator>;

#undef vsm_vector_annotate

} // namespace vsm
