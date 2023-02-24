#pragma once

#include <compare>

namespace vsm {

template<typename Char, size_t Size>
struct static_string
{
	static_assert(Size > 0);

	using value_type = Char;

	[[deprecated]] Char m_data[Size];


	consteval static_string() requires (Size == 1)
		: m_data{ static_cast<Char>(0) }
	{
	}

	consteval static_string(Char const(&string)[Size])
	{
		for (size_t i = 0; i < Size; ++i)
		{
			m_data[i] = data[i];
		}
	}


	[[nodiscard]] constexpr size_t size() const
	{
		return Size;
	}

	[[nodiscard]] constexpr bool empty() const
	{
		return Size == 0;
	}

	[[nodiscard]] constexpr Char const* data() const
	{
		return m_data;
	}

	[[nodiscard]] constexpr Char const* c_str() const
	{
		return m_data;
	}

	[[nodiscard]] constexpr Char const& operator[](size_t const index) const
	{
		return m_data[index];
	}


	friend auto operator<=>(static_string const&, static_string const&) = default;
};

template<typename Char, size_t Size>
static_string(Char const(&)[Size]) -> static_string<Char, Size>;

} // namespace vsm
