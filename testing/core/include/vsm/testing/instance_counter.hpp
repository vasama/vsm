#pragma once

#include <compare>
#include <type_traits>

#include <cstddef>

namespace vsm::test {
namespace detail {

using instance_count_type = std::make_signed_t<size_t>;

template<typename T>
inline instance_count_type g_root_instance_count;

template<typename T>
inline instance_count_type* g_current_instance_count = &g_root_instance_count<T>;


struct instance_count_default_t;

extern template instance_count_type g_root_instance_count<instance_count_default_t>;
extern template instance_count_type* g_current_instance_count<instance_count_default_t>;

} // namespace detail

template<typename T>
class basic_counted
{
public:
	friend auto operator<=>(basic_counted const&, basic_counted const&) = default;

protected:
	basic_counted()
	{
		++*detail::g_current_instance_count<T>;
	}

	basic_counted(basic_counted const&) noexcept
	{
		++*detail::g_current_instance_count<T>;
	}

	basic_counted& operator=(basic_counted const&) & noexcept
	{
		return *this;
	}

	~basic_counted()
	{
		--*detail::g_current_instance_count<T>;
	}
};

template<typename T>
class basic_scoped_count
{
	detail::instance_count_type* m_prev;
	detail::instance_count_type m_count;

public:
	basic_scoped_count()
		: m_prev(detail::g_current_instance_count<T>)
		, m_count(0)
	{
		detail::g_current_instance_count<T> = &m_count;
	}

	basic_scoped_count(basic_scoped_count const&) = delete;
	basic_scoped_count& operator=(basic_scoped_count const&) = delete;

	~basic_scoped_count()
	{
		detail::g_current_instance_count<T> = m_prev;
	}

	[[nodiscard]] detail::instance_count_type count() const
	{
		return m_count;
	}

	[[nodiscard]] bool empty() const
	{
		return m_count == 0;
	}
};

template<typename T>
inline constexpr auto const& root_count_for = detail::g_root_instance_count<T>;


using counted = basic_counted<detail::instance_count_default_t>;
using scoped_count = basic_scoped_count<detail::instance_count_default_t>;

inline constexpr auto const& root_count = detail::g_root_instance_count<detail::instance_count_default_t>;

} // namespace vsm::test
