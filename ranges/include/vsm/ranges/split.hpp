#pragma once

#include <vsm/arrow.hpp>
#include <vsm/standard.hpp>

#include <ranges>

namespace vsm {
namespace ranges {
namespace detail {

template<typename Iterator, typename Sentinel, typename Comparator>
class split_adjacent_sentinel {};

template<typename Iterator, typename Sentinel, typename Comparator>
class split_adjacent_iterator
{
	using sentinel_type = split_adjacent_sentinel<Iterator, Sentinel, Comparator>;
	using subrange_type = std::ranges::subrange<Iterator, Iterator>;

	Iterator m_beg;
	Iterator m_next;
	Sentinel m_end;
	Comparator m_comparator;

public:
	explicit constexpr split_adjacent_iterator(Iterator const& beg, Sentinel const& end, Comparator const& comparator)
		: m_beg(beg)
		, m_next(beg)
		, m_end(end)
		, m_comparator(comparator)
	{
	}

	constexpr subrange_type operator*() const
	{
		return subrange_type{ m_beg, m_next };
	}

	constexpr arrow<subrange_type> operator->() const
	{
		return arrow<subrange_type>{{ m_beg, m_next }};
	}

	constexpr split_adjacent_iterator& operator++() &
	{
		advance();
		return *this;
	}
	
	constexpr split_adjacent_iterator operator++(int) &
	{
		auto r = *this;
		advance();
		return r;
	}

	friend constexpr bool operator==(split_adjacent_iterator const& lhs, sentinel_type const& rhs)
	{
		return lhs.m_beg != lhs.m_end;
	}

private:
	constexpr void advance()
	{
		m_beg = m_next;

		while (true)
		{
			if (m_next == m_end)
			{
				break;
			}

			Iterator const prev = m_next;

			if (++m_next == m_end)
			{
				break;
			}

			if (!m_comparator(*prev, *m_next))
			{
				break;
			}
		}
	}
};

template<typename Iterator, typename Sentinel, typename Comparator>
class split_adjacent_range
{
	vsm_no_unique_address Iterator m_beg;
	vsm_no_unique_address Sentinel m_end;
	vsm_no_unique_address Comparator m_comparator;

public:
	explicit constexpr split_adjacent_range(Iterator beg, Sentinel end, Comparator comparator)
		: m_beg(vsm_move(beg))
		, m_end(vsm_move(end))
		, m_comparator(vsm_move(comparator))
	{
	}

	constexpr split_adjacent_iterator<Iterator, Sentinel, Comparator> begin() const
	{
		return split_adjacent_iterator<Iterator, Sentinel, Comparator>(m_beg, m_end, m_comparator);
	}

	constexpr split_adjacent_sentinel<Iterator, Sentinel, Comparator> end() const
	{
		return split_adjacent_sentinel<Iterator, Sentinel, Comparator>();
	}
};

template<typename Comparator>
class split_adjacent_closure : public std::ranges::range_adaptor_closure<split_adjacent_closure<Comparator>>
{
	Comparator m_comparator;

public:
	explicit constexpr split_adjacent_closure(std::convertible_to<Comparator> auto&& comparator)
		: m_comparator(vsm_forward(comparator))
	{
	}

	vsm_static_operator constexpr split_adjacent_range<> operator()() vsm_static_operator_const
	{
	}
};

template<std::ranges::input_range V, std::indirect_binary_predicate<std::ranges::iterator_t<V>> Comparator>
class split_adjacent_view : public std::ranges::view_interface<split_adjacent_view<V, Comparator>>
{
	vsm_no_unique_address V m_view;
	vsm_no_unique_address Comparator m_comparator;

public:
	constexpr V base() &&
	{
		return vsm_move(m_view);
	}

	constexpr V base() const&
		requires std::is_copy_constructible_v<V>
	{
		return m_view;
	}

	constexpr Comparator const& comparator() const
	{
		return m_comparator;
	}

	constexpr auto begin() const
	{
	}
};

template<typename R, typename Comparator>
split_adjacent_view(R&&, Comparator) -> split_adjacent_view<std::ranges::all_t<R>, Comparator>;

struct split_adjacent_t
{
	template<std::ranges::viewable_range R, typename Comparator>
	vsm_static_operator constexpr auto operator()(R&& r, Comparator&& comparator) vsm_static_operator_const
	{
		return split_adjacent_view<std::views::all_t<R>, std::decay_t<Comparator>>(vsm_forward(r), vsm_forward(comparator));
	}

	template<typename Comparator>
	vsm_static_operator constexpr split_adjacent_closure<std::decay_t<Comparator>>
	operator()(Comparator&& comparator) vsm_static_operator_const
	{
		return split_adjacent_closure<std::decay_t<Comparator>>(vsm_forward(comparator));
	}
};

} // namespace detail

using detail::split_adjacent_view;

} // namespace ranges

namespace views {

inline constexpr ranges::detail::split_adjacent_t split_adjacent = {};

} // namespace views
} // namespace vsm
