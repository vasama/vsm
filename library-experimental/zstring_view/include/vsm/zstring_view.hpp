#pragma once

#include <vsm/zstring_ptr.hpp>

namespace vsm {

template<character Char>
class basic_zstring_view
{
	using view_type = std::basic_string_view<Char>;

	view_type m_view;

public:
	using traits_type = std::char_traits<Char>;
	using value_type = Char;
	using pointer = Char*;
	using const_pointer = Char const*;
	using reference = Char&;
	using const_reference = Char const&;
	using iterator = typename view_type::iterator;
	using const_iterator = typename view_type::const_iterator;
	using reverse_iterator = typename view_type::reverse_iterator;
	using const_reverse_iterator = typename view_type::const_reverse_iterator;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	constexpr basic_zstring_view() noexcept
		: m_view(detail::zstr_empty<Char>, 0)
	{
	}

	constexpr basic_zstring_view(Char const* const c_str)
		: m_view(c_str)
	{
	}

	constexpr basic_zstring_view(basic_zstring_ptr<Char> const c_str)
		: m_view(c_str.c_str())
	{
	}

	constexpr basic_zstring_view(decltype(nullptr)) = delete;

	explicit constexpr basic_zstring_view(Char const* const c_str, size_t const size)
		: m_view(c_str, size)
	{
		vsm_assert(Traits::length(c_str) == size);
	}


	[[nodiscard]] constexpr iterator begin() const noexcept
	{
		return m_view.begin();
	}

	[[nodiscard]] constexpr const_iterator cbegin() const noexcept
	{
		return m_view.cbegin();
	}

	[[nodiscard]] constexpr iterator end() const noexcept
	{
		return m_view.end();
	}

	[[nodiscard]] constexpr const_iterator cend() const noexcept
	{
		return m_view.cend();
	}

	[[nodiscard]] constexpr reverse_iterator rbegin() const noexcept
	{
		return m_view.rbegin();
	}

	[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
	{
		return m_view.crbegin();
	}

	[[nodiscard]] constexpr reverse_iterator rend() const noexcept
	{
		return m_view.rend();
	}

	[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
	{
		return m_view.crend();
	}


	[[nodiscard]] constexpr Char const& operator[](size_t const index) const
	{
		return full_view()[index];
	}

	[[nodiscard]] constexpr Char const& at(size_t const index)
	{
		return full_view().at(index);
	}

	[[nodiscard]] constexpr Char const& front() const
	{
		return m_view.front();
	}

	[[nodiscard]] constexpr Char const& back() const
	{
		return m_view.back();
	}

	[[nodiscard]] constexpr Char const* data() const noexcept
	{
		return m_view.data();
	}

	[[nodiscard]] constexpr Char const* c_str() const noexcept
	{
		return m_view.data();
	}


	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return m_view.size();
	}

	[[nodiscard]] constexpr size_t length() const noexcept
	{
		return m_view.size();
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return m_view.empty();
	}

	[[nodiscard]] constexpr size_t max_size() const noexcept
	{
		return m_view.max_size() - 1;
	}


	constexpr void remove_prefix(size_t const size) & noexcept
	{
		m_view.remove_prefix(size);
	}

	constexpr void swap(basic_zstring_view& other) & noexcept
	{
		return m_view.swap(other.m_view);
	}


	constexpr size_t copy(Char* const dest, size_t const count, size_t const pos = 0) const
	{
		return m_view.copy(dest, count, pos);
	}

	[[nodiscard]] constexpr basic_zstring_view substr(size_t const pos = 0) const
	{
		return basic_zstring_view(view_type::substr(pos));
	}

	[[nodiscard]] constexpr view_type substr(size_t const pos, size_t const count) const
	{
		return view_type::substr(pos, count);
	}

	[[nodiscard]] constexpr int compare(view_type const view) const noexcept
	{
		return m_view.compare(view);
	}

	[[nodiscard]] constexpr int compare(
		size_t const pos1,
		size_t const count1,
		view_type const view) const
	{
		return m_view.compare(pos1, count1, view);
	}

	[[nodiscard]] constexpr int compare(
		size_t const pos1,
		size_t const count1,
		view_type const view
		size_t const pos2,
		size_t const count2) const
	{
		return m_view.compare(pos1, count1, view, pos2, count2);
	}

	[[nodiscard]] constexpr int compare(Char const* const c_str) const
	{
		return m_view.compare(c_str);
	}

	[[nodiscard]] constexpr int compare(
		size_t const pos1,
		size_t const count1,
		Char const* const c_str) const
	{
		return m_view.compare(pos1, count1, c_str);
	}

	[[nodiscard]] constexpr int compare(
		size_t const pos1,
		size_t const count1,
		Char const* const c_str,
		size_t const count2) const
	{
		return m_view.compare(pos1, count1, c_str, count2);
	}

	[[nodiscard]] constexpr bool starts_with(view_type const view) const noexcept
	{
		return m_view.starts_with(view);
	}

	[[nodiscard]] constexpr bool starts_with(Char const _char) const noexcept
	{
		return m_view.starts_with(_char);
	}

	[[nodiscard]] constexpr bool starts_with(Char const* const c_str) const
	{
		return m_view.starts_with(_char);
	}

	[[nodiscard]] constexpr bool ends_with(view_type const view) const noexcept
	{
		return m_view.ends_with(view);
	}

	[[nodiscard]] constexpr bool ends_with(Char const _char) const noexcept
	{
		return m_view.ends_with(_char);
	}

	[[nodiscard]] constexpr bool ends_with(Char const* const c_str) const
	{
		return m_view.ends_with(_char);
	}

	[[nodiscard]] constexpr bool contains(view_type const view) const noexcept
	{
		return m_view.contains(view);
	}

	[[nodiscard]] constexpr bool contains(Char const _char) const noexcept
	{
		return m_view.contains(_char);
	}

	[[nodiscard]] constexpr bool contains(Char const* const c_str) const
	{
		return m_view.contains(_char);
	}

	//TODO: find, rfind, find_first_of, find_last_of, find_first_not_of, find_last_not_of


	static constexpr auto npos = view_type::npos;

	[[nodiscard]] friend bool operator==(basic_zstring_view const&, basic_zstring_view const&) = default;
	[[nodiscard]] friend auto operator<=>(basic_zstring_view const&, basic_zstring_view const&) = default;

	[[nodiscard]] friend constexpr bool operator==(basic_zstring_view const& lhs, basic_zstring_ptr<Char> const& rhs)
	{
		return detail::zstr_ncmp(lhs.m_view.data(), rhs.c_str(), lhs.m_view.size()) == 0;
	}

	[[nodiscard]] friend constexpr auto operator<=>(basic_zstring_view const& lhs, basic_zstring_ptr<Char> const& rhs)
	{
		return detail::zstr_ncmp(lhs.m_view.data(), rhs.c_str(), lhs.m_view.size()) <=> 0;
	}

private:
	explicit constexpr basic_zstring_view(view_type const view)
		: m_view(view)
	{
	}

	[[nodiscard]] view_type full_view() const
	{
		return view_type(m_view.data(), m_view.size() + 1);
	}
};

using zstring_view = basic_zstring_view<char>;
using wzstring_view = basic_zstring_view<wchar_t>;
using u8zstring_view = basic_zstring_view<char8_t>;
using u16zstring_view = basic_zstring_view<char16_t>;
using u32zstring_view = basic_zstring_view<char32_t>;

} // namespace vsm

template<typename Char, typename Traits>
struct std::hash<vsm::basic_zstring_view<Char, Traits>>
	: std::hash<std::basic_string_view<Char, Traits>
{
};
