#pragma once

#include <cstddef>

namespace vsm {

template<typename Char, size_t Size>
struct basic_literal_string
{
	static constexpr size_t size = Size;
	Char data[Size];

	consteval basic_literal_string(Char const(&string)[Size + 1])
	{
		for (size_t i = 0; i < Size; ++i)
		{
			data[i] = string[i];
		}
	}
};

template<typename Char>
struct basic_literal_string<Char, 0>
{
	static constexpr size_t size = 0;
	static constexpr Char const* data = nullptr;

	consteval basic_literal_string(Char const(&string)[1])
	{
	}
};

template<typename Char, size_t Size>
basic_literal_string(Char const(&)[Size]) -> basic_literal_string<Char, Size - 1>;


template<size_t Size>
struct literal_string : basic_literal_string<char, Size>
{
	using basic_literal_string<char, Size>::basic_literal_string;
};

template<size_t Size>
literal_string(char const(&)[Size]) -> literal_string<Size - 1>;


template<typename Char>
struct basic_literal_parser
{
	Char const* beg;
	Char const* end;

	explicit constexpr basic_literal_parser(Char const* const data, size_t const size)
		: beg(data)
		, end(data + size)
	{
	}

	[[nodiscard]] constexpr bool consume(Char const character)
	{
		if (beg != end && *beg == character)
		{
			++beg;
			return true;
		}
		return false;
	}

	template<std::unsigned_integral Integer>
	[[nodiscard]] constexpr bool consume_integer(Integer& out_value, int const radix = 10)
	{
		switch (radix)
		{
		case +2: return _consume_integer<+2, _parse_bin_digit>(out_value);
		case +8: return _consume_integer<+8, _parse_oct_digit>(out_value);
		case 10: return _consume_integer<10, _parse_dec_digit>(out_value);
		case 16: return _consume_integer<16, _parse_hex_digit>(out_value);
		default: return false;
		}
	}

private:
	[[nodiscard]] static constexpr int _parse_bin_digit(Char const character)
	{
		if (character == Char('0'))
		{
			return 0;
		}

		if (character == Char('1'))
		{
			return 1;
		}

		return static_cast<int>(-1);
	}

	[[nodiscard]] static constexpr int _parse_oct_digit(Char const character)
	{
		if (Char('0') <= character && character <= Char('7'))
		{
			return character - Char('0');
		}

		return static_cast<int>(-1);
	}

	[[nodiscard]] static constexpr int _parse_dec_digit(Char const character)
	{
		if (Char('0') <= character && character <= Char('9'))
		{
			return character - Char('0');
		}

		return static_cast<int>(-1);
	}

	[[nodiscard]] static constexpr int _parse_hex_digit(Char const character)
	{
		if (Char('0') <= character && character <= Char('9'))
		{
			return character - Char('0');
		}

		if (Char('a') <= character && character <= Char('f'))
		{
			return character - Char('a');
		}

		if (Char('A') <= character && character <= Char('F'))
		{
			return character - Char('A');
		}

		return static_cast<int>(-1);
	}

	template<int Radix, auto ParseDigit, std::unsigned_integral Integer>
	[[nodiscard]] constexpr bool _consume_integer(Integer& out_value)
	{
		Char const* const beg_pos = beg;

		Integer value = 0;
		for (; beg != end; ++beg)
		{
			int const digit = ParseDigit(*beg);

			if (digit < 0)
			{
				break;
			}

			value *= static_cast<Integer>(Radix);
			value += digit;
		}

		if (beg_pos == beg)
		{
			return false;
		}

		out_value = value;
		return true;
	}
};

using literal_parser = basic_literal_parser<char>;

} // namespace vsm
