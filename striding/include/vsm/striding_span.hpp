#pragma once

#include <vsm/striding_ptr.hpp>

#include <ranges>

namespace vsm {

template<non_ref T>
class striding_span
{
	using byte_type = vsm::copy_cv_t<T, unsigned char>;

	T* m_ptr;
	ptrdiff_t m_stride;
	size_t m_size;

public:
	striding_span() = default;

	template<typename U>
		requires std::convertible_to<U*, T*>
	striding_span(U* const ptr, size_t const size)
		: m_ptr(ptr)
		, m_stride(sizeof(U))
		, m_size(size)
	{
	}

	template<typename U>
		requires std::convertible_to<U*, T*>
	striding_span(striding_ptr<U> const ptr, size_t const size)
		: m_ptr(ptr.base())
		, m_stride(ptr.stride())
		, m_size(size)
	{
	}

	template<not_same_as<T> U>
		requires std::convertible_to<U*, T*>
	striding_span(striding_span<U> const& span)
		: m_ptr(span.m_ptr)
		, m_stride(span.m_stride)
		, m_size(span.m_size)
	{
	}

	template<typename U, size_t Size>
		requires std::convertible_to<U*, T*>
	striding_span(U(&array)[Size])
		: m_ptr(array)
		, m_stride(sizeof(U))
		, m_size(Size)
	{
	}

	template<no_cvref_of<striding_span> R>
		requires std::ranges::contiguous_range<R>
	striding_span(R&& range)
		: m_ptr(std::to_address(std::ranges::begin(range)))
		, m_stride(sizeof(std::ranges::range_value_t<R>))
		, m_size(std::ranges::size(range))
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

	[[nodiscard]] size_t size() const
	{
		return m_size;
	}

	[[nodiscard]] T& operator[](size_t const offset) const
	{
		return striding_ptr<T>(m_ptr, m_stride)[static_cast<ptrdiff_t>(offset)];
	}

	[[nodiscard]] striding_ptr<T> begin() const
	{
		return striding_ptr<T>(m_ptr, m_stride);
	}

	[[nodiscard]] striding_ptr<T> end() const
	{
		return striding_ptr<T>(m_ptr, m_stride) + m_size;
	}

private:
	template<non_ref U>
	friend class striding_span;
};

} // namespace vsm
