#pragma once

#include <vsm/assert.h>

#include <limits>

#include <cstddef>

namespace vsm {

template<typename Char, size_t Size>
struct basic_literal_string
{
	static constexpr size_t size = Size;

	Char data[Size] = {};

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
		vsm_assert(2 <= radix && radix <= 36);

		return radix < 10
			? _consume_integer<_parse_digit_numeric>(out_value, radix)
			: _consume_integer<_parse_digit_alpha_numeric>(out_value, radix);
	}

private:
	[[nodiscard]] static constexpr int _parse_digit_numeric(Char const character, int const radix)
	{
		if (Char('0') <= character && character < Char('0') + radix)
		{
			return character - Char('0');
		}

		return static_cast<int>(-1);
	}

	[[nodiscard]] static constexpr int _parse_digit_alpha_numeric(Char const character, int const radix)
	{
		if (Char('0') <= character && character <= Char('9'))
		{
			return character - Char('0');
		}

		if (Char('a') <= character && character < Char('a') + radix)
		{
			return character - Char('a') + 10;
		}

		if (Char('A') <= character && character < Char('A') + radix)
		{
			return character - Char('A') + 10;
		}

		return static_cast<int>(-1);
	}

	template<auto ParseDigit, std::unsigned_integral Integer>
	[[nodiscard]] constexpr bool _consume_integer(Integer& out_value, int const radix)
	{
		constexpr Integer max = std::numeric_limits<Integer>::max();
		Integer const max_mul = max / radix;

		Char const* beg = this->beg;

		Integer value = 0;
		for (; beg != end; ++beg)
		{
			int const digit = ParseDigit(*beg, radix);

			if (digit < 0)
			{
				break;
			}

			if (value > max_mul)
			{
				return false;
			}
			value *= static_cast<Integer>(radix);

			if (value > max - digit)
			{
				return false;
			}
			value += digit;
		}

		if (beg == this->beg)
		{
			return false;
		}

		this->beg = beg;
		out_value = value;

		return true;
	}
};

using literal_parser = basic_literal_parser<char>;

} // namespace vsm
