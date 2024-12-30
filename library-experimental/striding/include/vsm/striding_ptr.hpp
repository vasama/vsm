#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>

#include <compare>
#include <iterator>
#include <memory>

#include <cstdint>

namespace vsm {

#define vsm_detail_add(ptr, offset) \
	reinterpret_cast<T*>(reinterpret_cast<byte_type*>(ptr) + (offset))

#define vsm_detail_sub(ptr, offset) \
	reinterpret_cast<T*>(reinterpret_cast<byte_type*>(ptr) - (offset))

template<non_ref T>
class striding_ptr
{
	using byte_type = vsm::copy_cv_t<T, unsigned char>;

	T* m_ptr;
	ptrdiff_t m_stride;

public:
	striding_ptr() = default;

	striding_ptr(decltype(nullptr))
		: m_ptr(nullptr)
	{
	}

	template<typename U = T>
		requires std::convertible_to<U*, T*>
	striding_ptr(U* const ptr)
		: m_ptr(ptr)
		, m_stride(sizeof(U))
	{
	}

	template<typename U = T>
		requires std::convertible_to<U*, T*>
	explicit striding_ptr(U* const ptr, ptrdiff_t const stride)
		: m_ptr(ptr)
		, m_stride(stride)
	{
	}

	template<vsm::not_same_as<T> U>
		requires std::convertible_to<U*, T*>
	striding_ptr(striding_ptr<U> const ptr)
		: m_ptr(ptr.m_ptr)
		, m_stride(ptr.m_stride)
	{
	}


	[[nodiscard]] T* base() const
	{
		return m_ptr;
	}

	[[nodiscard]] ptrdiff_t stride() const
	{
		return m_stride;
	}

	[[nodiscard]] bool is_contiguous() const
	{
		return m_stride == sizeof(T);
	}


	[[nodiscard]] T& operator*() const
	{
		vsm_assert(m_ptr != nullptr);
		return *std::launder(m_ptr);
	}

	[[nodiscard]] T* operator->() const
	{
		vsm_assert(m_ptr != nullptr);
		return std::launder(m_ptr);
	}

	[[nodiscard]] T& operator[](ptrdiff_t const offset) const
	{
		vsm_assert(m_ptr != nullptr);
		return *std::launder(vsm_detail_add(m_ptr, offset * m_stride));
	}


	[[nodiscard]] friend striding_ptr operator+(striding_ptr const& ptr, ptrdiff_t const offset)
	{
		vsm_assert(ptr.m_ptr != nullptr);
		return striding_ptr(vsm_detail_add(ptr.m_ptr, offset * ptr.m_stride), ptr.m_stride);
	}

	[[nodiscard]] friend striding_ptr operator+(ptrdiff_t const offset, striding_ptr const& ptr)
	{
		vsm_assert(ptr.m_ptr != nullptr);
		return striding_ptr(vsm_detail_add(ptr.m_ptr, offset * ptr.m_stride), ptr.m_stride);
	}

	[[nodiscard]] friend striding_ptr operator-(striding_ptr const& ptr, ptrdiff_t const offset)
	{
		vsm_assert(ptr.m_ptr != nullptr);
		return striding_ptr(vsm_detail_sub(ptr.m_ptr, offset * ptr.m_stride), ptr.m_stride);
	}

	[[nodiscard]] friend ptrdiff_t operator-(striding_ptr const& lhs, striding_ptr const& rhs)
	{
		vsm_assert(lhs.m_ptr != nullptr);
		vsm_assert(rhs.m_ptr != nullptr);
		vsm_assert(lhs.m_stride == rhs.m_stride);

		ptrdiff_t const offset =
			reinterpret_cast<byte_type*>(lhs.m_ptr) -
			reinterpret_cast<byte_type*>(rhs.m_ptr);
		vsm_assert(offset % lhs.m_stride == 0);

		return offset / lhs.m_stride;
	}

	striding_ptr& operator++() &
	{
		vsm_assert(m_ptr != nullptr);
		m_ptr = vsm_detail_add(m_ptr, m_stride);
		return *this;
	}

	[[nodiscard]] striding_ptr operator++(int) &
	{
		vsm_assert(m_ptr != nullptr);
		striding_ptr const r = *this;
		m_ptr = vsm_detail_add(m_ptr, m_stride);
		return r;
	}

	striding_ptr& operator--() &
	{
		vsm_assert(m_ptr != nullptr);
		m_ptr = vsm_detail_sub(m_ptr, m_stride);
		return *this;
	}

	[[nodiscard]] striding_ptr operator--(int) &
	{
		vsm_assert(m_ptr != nullptr);
		striding_ptr const r = *this;
		m_ptr = vsm_detail_sub(m_ptr, m_stride);
		return r;
	}

	striding_ptr& operator+=(ptrdiff_t const offset) &
	{
		vsm_assert(m_ptr != nullptr);
		m_ptr = vsm_detail_add(m_ptr, offset * m_stride);
		return *this;
	}

	striding_ptr& operator-=(ptrdiff_t const offset) &
	{
		vsm_assert(m_ptr != nullptr);
		m_ptr = vsm_detail_sub(m_ptr, offset * m_stride);
		return *this;
	}


	[[nodiscard]] friend bool operator==(striding_ptr const& ptr, decltype(nullptr))
	{
		return ptr.m_ptr == nullptr;
	}

	[[nodiscard]] friend bool operator!=(striding_ptr const& ptr, decltype(nullptr))
	{
		return ptr.m_ptr != nullptr;
	}

	[[nodiscard]] friend bool operator==(striding_ptr const& lhs, striding_ptr const& rhs)
	{
		vsm_assert(lhs.m_stride == rhs.m_stride);
		return lhs.m_ptr == rhs.m_ptr;
	}

	[[nodiscard]] friend auto operator<=>(striding_ptr const& lhs, striding_ptr const& rhs)
	{
		vsm_assert(lhs.m_stride == rhs.m_stride);
		return std::compare_three_way()(lhs.m_ptr, rhs.m_ptr);
	}

private:
	template<non_ref U>
	friend class striding_ptr;
};

#undef vsm_detail_add
#undef vsm_detail_sub

} // namespace vsm

template<typename T>
struct std::iterator_traits<vsm::striding_ptr<T>>
{
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using iterator_category = random_access_iterator_tag;
};
