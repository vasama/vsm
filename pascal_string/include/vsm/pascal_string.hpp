#pragma once

#include <vsm/standard.hpp>

#include <bit>
#include <string_view>

#include <cstdint>

namespace vsm {
namespace detail {

using _pstr_size_t = uint32_t;

template<typename Char>
inline constexpr size_t _pstr_size_size = sizeof(_pstr_size_t) / sizeof(Char);

template<typename Char>
struct _pstr_size_data
{
	Char data[_pstr_size_size<Char>];
};

template<typename Char>
constexpr _pstr_size_t _pstr_size(Char const* const buffer)
{
	_pstr_size_data<Char> size;
	for (size_t i = 0; i < _pstr_size_size<Char>; ++i)
	{
		size.data[i] = buffer[i];
	}
	return std::bit_cast<_pstr_size_t>(size);
}

template<typename Char, size_t Size>
struct _pstr
{
	static_assert(Size > 0);

	using value_type = Char;

	alignas(_pstr_size_t) Char buffer[_pstr_size_size<Char> + Size];

	consteval _pstr() requires (Size == 1)
	{
		set_size(0);
		buffer[_pstr_size_size<Char>] = static_cast<Char>(0);
	}

	consteval _pstr(Char const(&string)[Size])
	{
		set_size(Size - 1);
		set_data(string);
	}

	consteval void set_size(_pstr_size_t const size)
	{
		auto const s = std::bit_cast<_pstr_size_data<Char>>(size);

		for (size_t i = 0; i < _pstr_size_size<Char>; ++i)
		{
			buffer[i] = s.data[i];
		}
	}

	consteval void set_data(Char const* const data)
	{
		Char* const s = buffer + _pstr_size_size<Char>;

		for (size_t i = 0; i < Size; ++i)
		{
			s[i] = data[i];
		}
	}
};

template<typename Char, size_t Size>
_pstr(Char const(&)[Size]) -> _pstr<Char, Size>;

template<typename Char>
inline constexpr _pstr<Char, 0> _pstr_empty;

} // namespace detail

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_pascal_string
{
	Char const* m_buffer;

public:
	using size_type = detail::_pstr_size_t;
	using difference_type = std::make_signed_t<size_type>;
	using iterator = Char const*;
	using const_iterator = Char const*;

	constexpr basic_pascal_string()
		: m_buffer(detail::_pstr_empty<Char>.buffer)
	{
	}

	template<size_t Size>
	explicit constexpr basic_pascal_string(detail::_pstr<Char, Size> const& string)
		: m_buffer(string.buffer)
	{
	}

	[[nodiscard]] constexpr size_t size() const
	{
		vsm_if_consteval
		{
			return detail::_pstr_size(m_buffer);
		}
		else
		{
			detail::_pstr_size_t size;
			memcpy(&size, m_buffer, sizeof(size));
			return size;
		}
	}

	[[nodiscard]] constexpr Char const* data() const
	{
		return m_buffer + detail::_pstr_size_size<Char>;
	}

	[[nodiscard]] constexpr Char const* begin() const
	{
		return m_buffer + detail::_pstr_size_size<Char>;
	}

	[[nodiscard]] constexpr Char const* end() const
	{
		return m_buffer + detail::_pstr_size_size<Char> + size();
	}
};

namespace string_literals {

template<detail::_pstr String>
[[nodiscard]] consteval basic_pascal_string<typename decltype(String)::value_type> operator""_pascal()
{
	return basic_pascal_string<typename decltype(String)::value_type>(String);
}

} // namespace string_literals

using pascal_string = basic_pascal_string<char>;
using wpascal_string = basic_pascal_string<wchar_t>;
using u8pascal_string = basic_pascal_string<char8_t>;
using u16pascal_string = basic_pascal_string<char16_t>;
using u32pascal_string = basic_pascal_string<char32_t>;

} // namespace vsm
