#pragma once

#include <vsm/array.hpp>
#include <vsm/standard.hpp>

#include <bit>
#include <string_view>

#include <cstdint>

namespace vsm {
namespace detail::pascal_string_ {

using size_type = uint32_t;

template<typename Char>
inline constexpr size_t size_buffer_size = sizeof(size_type) / sizeof(Char);

template<typename Char>
using size_buffer = array<Char, size_buffer_size<Char>>;

template<typename Char>
constexpr size_type get_size(Char const* const buffer)
{
	size_buffer<Char> s;
	for (size_t i = 0; i < s.size(); ++i)
	{
		s[i] = buffer[i];
	}
	return std::bit_cast<size_type>(s);
}

template<typename Char, size_t Size>
struct static_string
{
	static_assert(Size > 0);

	using value_type = Char;

	using size_buffer = pascal_string_::size_buffer<Char>;
	static constexpr size_t data_offset = std::tuple_size_v<size_buffer>;
	
	Char buffer alignas(size_type)[data_offset + Size];

	consteval static_string() requires (Size == 1)
	{
		set_size(0);
		buffer[data_offset] = static_cast<Char>(0);
	}

	consteval static_string(Char const(&string)[Size])
	{
		set_size(Size - 1);
		set_data(string);
	}

	consteval void set_size(size_type const size)
	{
		size_buffer const s = std::bit_cast<size_buffer>(size);

		for (size_t i = 0; i < s.size(); ++i)
		{
			buffer[i] = s[i];
		}
	}

	consteval void set_data(Char const* const data)
	{
		Char* const s = buffer + data_offset;

		for (size_t i = 0; i < Size; ++i)
		{
			s[i] = data[i];
		}
	}
};

template<typename Char, size_t Size>
static_string(Char const(&)[Size]) -> static_string<Char, Size>;

template<typename Char>
inline constexpr static_string<Char, 0> empty;

} // namespace detail::pascal_string_

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_pascal_string
{
	using size_type = detail::pascal_string_::size_type;
	
	using size_buffer = detail::pascal_string_::size_buffer<Char>;
	
	template<size_t Size>
	using static_string = detail::pascal_string_::static_string<Char, Size>;

	Char const* m_buffer;

public:
	constexpr basic_pascal_string()
		: m_buffer(detail::pascal_string_::empty<Char>.buffer)
	{
	}

	template<size_t Size>
	explicit constexpr basic_pascal_string(static_string<Size> const& string)
		: m_buffer(string.buffer)
	{
	}

	constexpr size_t size() const
	{
		vsm_if_consteval
		{
			return detail::pascal_string_::get_size(m_buffer);
		}
		else
		{
			return *reinterpret_cast<size_type const*>(m_buffer);
		}
	}

	constexpr Char const* data() const
	{
		return m_buffer + detail::pascal_string_::size_buffer_size<Char>;
	}

	constexpr Char const* begin() const
	{
		return m_buffer + detail::pascal_string_::size_buffer_size<Char>;
	}

	constexpr Char const* end() const
	{
		return m_buffer + detail::pascal_string_::size_buffer_size<Char> + size();
	}

	operator std::basic_string_view<Char, Traits>() const
	{
		return std::basic_string_view<Char, Traits>(data(), size());
	}
};

template<detail::pascal_string_::static_string String>
consteval basic_pascal_string<typename decltype(String)::value_type> operator""_pascal()
{
	return basic_pascal_string<typename decltype(String)::value_type>(String);
}

using pascal_string = basic_pascal_string<char>;

} // namespace vsm
