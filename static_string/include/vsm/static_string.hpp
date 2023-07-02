#pragma once

namespace vsm {

template<typename Char, size_t Size>
struct basic_static_string
{
	static_assert(Size > 0);

	using value_type = Char;

	Char m_data[Size];


	consteval basic_static_string() requires (Size == 1)
	{
		m_data[0] = static_cast<Char>(0);
	}

	consteval basic_static_string(Char const(&string)[Size])
	{
		for (size_t i = 0; i < Size; ++i)
		{
			m_data[i] = data[i];
		}
	}


	friend auto operator<=>(basic_static_string const&, basic_static_string const&) = default;
};

template<typename Char, size_t Size>
basic_static_string(Char const(&)[Size]) -> basic_static_string<Char, Size>;

} // namespace vsm
