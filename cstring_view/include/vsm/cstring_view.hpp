#pragma once

#include <string_view>

namespace vsm {
namespace detail {

consteval char const* empty_string_literal(char)
{
	return "";
}

consteval wchar_t const* empty_string_literal(wchar_t)
{
	return L"";
}

consteval char8_t const* empty_string_literal(char8_t)
{
	return u8"";
}

consteval char16_t const* empty_string_literal(char16_t)
{
	return u"";
}

consteval char32_t const* empty_string_literal(char32_t)
{
	return U"";
}

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_cstring_view : std::basic_string_view<Char, Traits>
{
	using string_view_type = std::basic_string_view<Char, Traits>;

public:
	using traits_type = typename string_view_type::traits_type;
	using value_type = typename string_view_type::value_type;
	using pointer = typename string_view_type::pointer;
	using const_pointer = typename string_view_type::const_pointer;
	using reference = typename string_view_type::reference;
	using const_reference = typename string_view_type::const_reference;
	using iterator = typename string_view_type::iterator;
	using const_iterator = typename string_view_type::const_iterator;
	using reverse_iterator = typename string_view_type::reverse_iterator;
	using const_reverse_iterator = typename string_view_type::const_reverse_iterator;
	using size_type = typename string_view_type::size_type;
	using difference_type = typename string_view_type::difference_type;

	static constexpr size_type npos = string_view_type::npos;


	constexpr basic_cstring_view()
		: string_view_type(empty_string_literal(Char{}), 0)
	{
	}

	constexpr basic_cstring_view(Char const* c_str)
		: string_view_type(c_str)
	{
	}


	constexpr Char const* c_str() const noexcept
	{
		return string_view_type::data();
	}

	constexpr size_type max_size() const noexcept
	{
		return string_view_type::max_size() - 1;
	}

	constexpr basic_cstring_view substr() const noexcept
	{
		return *this;
	}

	constexpr basic_cstring_view substr(size_t const pos) const noexcept
	{
		return basic_cstring_view(string_view_type::substr(pos));
	}

	constexpr string_view_type substr(size_type const pos, size_type const count) const
	{
		return string_view_type::substr(pos, count);
	}


	using string_view_type::begin;
	using string_view_type::cbegin;
	using string_view_type::end;
	using string_view_type::cend;
	using string_view_type::rbegin;
	using string_view_type::crbegin;
	using string_view_type::rend;
	using string_view_type::crend;
	using string_view_type::operator[];
	using string_view_type::at;
	using string_view_type::front;
	using string_view_type::back;
	using string_view_type::data;
	using string_view_type::size;
	using string_view_type::length;
	using string_view_type::empty;
	using string_view_type::copy;
	using string_view_type::compare;
	using string_view_type::starts_with;
	using string_view_type::ends_with;
	using string_view_type::contains;
	using string_view_type::find;
	using string_view_type::rfind;
	using string_view_type::find_first_of;
	using string_view_type::find_last_of;
	using string_view_type::find_first_not_of;
	using string_view_type::find_last_not_of;


	bool operator==(basic_cstring_view const&) const = default;
	auto operator<=>(basic_cstring_view const&) const = default;

	friend bool operator==(basic_cstring_view const& lhs, string_view_type const& rhs) noexcept
	{
		return static_cast<string_view_type const&>(lhs) == rhs;
	}

	friend bool operator==(string_view_type const& lhs, basic_cstring_view const& rhs) noexcept
	{
		return lhs == static_cast<string_view_type const&>(rhs);
	}

	friend bool operator!=(basic_cstring_view const& lhs, string_view_type const& rhs) noexcept
	{
		return static_cast<string_view_type const&>(lhs) != rhs;
	}

	friend bool operator!=(string_view_type const& lhs, basic_cstring_view const& rhs) noexcept
	{
		return lhs != static_cast<string_view_type const&>(rhs);
	}

	friend auto operator<=>(basic_cstring_view const& lhs, string_view_type const& rhs) noexcept
	{
		return static_cast<string_view_type const&>(lhs) <=> rhs;
	}

	friend auto operator<=>(string_view_type const& lhs, basic_cstring_view const& rhs) noexcept
	{
		return lhs <=> static_cast<string_view_type const&>(rhs);
	}

private:
	explicit constexpr basic_cstring_view(string_view_type const string) noexcept
		: string_view_type(string)
	{
	}
};

} // namespace detail

using detail::basic_cstring_view;

using cstring_view = basic_cstring_view<char>;
using wcstring_view = basic_cstring_view<wchar_t>;

} // namespace vsm
