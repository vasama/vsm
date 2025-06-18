#pragma once

#include <vsm/concepts.hpp>
#include <vsm/platform.h>
#include <vsm/utility.hpp>

#include <span>

namespace vsm::detail {

template<typename Span, typename T>
class _span
{
	using span_type = std::span<T>;

protected:
	span_type m_span;

public:
	using element_type                  = T const;
	using value_type                    = T;
	using size_type                     = std::size_t;
	using difference_type               = std::ptrdiff_t;
	using pointer                       = T const*;
	using const_pointer                 = T const*;
	using reference                     = T const&;
	using const_reference               = T const&;
	using iterator                      = typename span_type::iterator;
	using const_iterator                = typename span_type::const_iterator;
	using reverse_iterator              = typename span_type::reverse_iterator;
	using const_reverse_iterator        = typename span_type::const_reverse_iterator;

	template<no_cvref_of<Span>... Args>
		requires std::constructible_from<span_type, Args...>
	explicit(!implicitly_constructible_from<span_type, Args...>)
	vsm_always_inline _span(Args&&... args)
		: m_span(vsm_forward(args)...)
	{
	}

	_span(_span const&) = default;
	_span& operator=(_span const&) = default;

	[[nodiscard]] vsm_always_inline iterator begin() const
	{
		return m_span.begin();
	}

	[[nodiscard]] vsm_always_inline iterator end() const
	{
		return m_span.end();
	}

	[[nodiscard]] vsm_always_inline iterator cbegin() const
	{
		return m_span.begin();
	}

	[[nodiscard]] vsm_always_inline iterator cend() const
	{
		return m_span.end();
	}

	[[nodiscard]] vsm_always_inline reverse_iterator rbegin() const
	{
		return m_span.rbegin();
	}

	[[nodiscard]] vsm_always_inline reverse_iterator rend() const
	{
		return m_span.rend();
	}

	[[nodiscard]] vsm_always_inline reverse_iterator crbegin() const
	{
		return m_span.rbegin();
	}

	[[nodiscard]] vsm_always_inline reverse_iterator crend() const
	{
		return m_span.rend();
	}


	[[nodiscard]] vsm_always_inline T const& front() const
	{
		return m_span.front();
	}

	[[nodiscard]] vsm_always_inline T const& back() const
	{
		return m_span.back();
	}

#if __cpp_lib_span >= 202311L
	[[nodiscard]] vsm_always_inline T const& at(size_type const offset) const
	{
		return m_span.at(offset);
	}
#endif

	[[nodiscard]] vsm_always_inline T const& operator[](size_type const offset) const
	{
		return m_span[offset];
	}

	[[nodiscard]] vsm_always_inline T const* data() const
	{
		return m_span.data();
	}


	[[nodiscard]] vsm_always_inline size_type size() const
	{
		return m_span.size();
	}

	[[nodiscard]] vsm_always_inline size_type size_bytes() const
	{
		return m_span.size_bytes();
	}

	[[nodiscard]] vsm_always_inline bool empty() const
	{
		return m_span.empty();
	}


	[[nodiscard]] vsm_always_inline constexpr Span first(size_type const count) const
	{
		return Span(span_type::first(count));
	}

	[[nodiscard]] vsm_always_inline constexpr Span last(size_type const count) const
	{
		return Span(span_type::last(count));
	}

	[[nodiscard]] vsm_always_inline constexpr Span subview(size_type const offset) const
	{
		return Span(span_type::subspan(offset));
	}

	[[nodiscard]] vsm_always_inline constexpr Span subview(
		size_type const offset,
		size_type const count) const
	{
		return Span(span_type::subspan(offset, count));
	}
};

} // namespace vsm::detail
