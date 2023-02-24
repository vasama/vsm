#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>

#include <compare>
#include <string_view>

namespace vsm {
namespace detail {

template<typename Char>
inline constexpr Char const zstr_empty[1] = { static_cast<Char>(0) };

template<typename Char>
constexpr int _zstr_ncmp(Char const* lhs, size_t const lhs_n, Char const* rhs, size_t const rhs_n)
{
	constexpr Char z = static_cast<Char>(0);

	size_t n = std::min(lhs_n, rhs_n);

	while (n--)
	{
		Char const l = *lhs++;
		Char const r = *rhs++;

		if (l != r)
		{
			return static_cast<int_least32_t>(l) - static_cast<int_least32_t>(r);
		}

		bool const lz = l == z;
		bool const rz = r == z;

		if (lz != rz)
		{
			return static_cast<int>(rz) - static_cast<int>(lz);
		}
	}

	return static_cast<int>(rhs_n - lhs_n);
}

template<typename Char>
constexpr int zstr_ncmp(Char const* lhs, size_t const lhs_n, Char const* rhs, size_t const rhs_n)
{
	return _zstr_ncmp(lhs, lhs_n, rhs, rhs_n);
}

template<>
inline constexpr int zstr_ncmp<char>(char const* lhs, size_t const lhs_n, char const* rhs, size_t const rhs_n)
{
	vsm_if_consteval
	{
		return _zstr_ncmp(lhs, lhs_n, rhs, rhs_n);
	}
	else
	{
		size_t const n = std::min(lhs_n, rhs_n);
		int const cmp = std::strncmp(lhs, rhs, n);

		return cmp != 0
			? cmp
			: static_cast<int>(rhs_n - lhs_n);
	}
}

template<>
inline constexpr int zstr_ncmp<wchar_t>(wchar_t const* lhs, size_t const lhs_n, wchar_t const* rhs, size_t const rhs_n)
{
	vsm_if_consteval
	{
		return _zstr_ncmp(lhs, lhs_n, rhs, rhs_n);
	}
	else
	{
		size_t const n = std::min(lhs_n, rhs_n);
		int const cmp = std::wcsncmp(lhs, rhs, n);

		return cmp != 0
			? cmp
			: static_cast<int>(rhs_n - lhs_n);
	}
}

template<typename Char>
constexpr int zstr_cmp(Char const* lhs, Char const* rhs)
{
	return zstr_ncmp(lhs, static_cast<size_t>(-1), rhs, static_cast<size_t>(-1));
}

template<>
inline constexpr int zstr_cmp<char>(char const* const lhs, char const* const rhs)
{
	vsm_if_consteval
	{
		return _zstr_ncmp(lhs, static_cast<size_t>(-1), rhs, static_cast<size_t>(-1));
	}
	else
	{
		return std::strcmp(lhs, rhs);
	}
}

template<>
inline constexpr int zstr_cmp<wchar_t>(wchar_t const* const lhs, wchar_t const* const rhs)
{
	vsm_if_consteval
	{
		return _zstr_ncmp(lhs, static_cast<size_t>(-1), rhs, static_cast<size_t>(-1));
	}
	else
	{
		return std::wcscmp(lhs, rhs);
	}
}

} // namespace detail

template<character Char>
class basic_zstring_ptr
{
	Char const* m_ptr;

public:
	class sentinel {};

	class iterator
	{
		Char const* m_ptr;

	public:
		using difference_type = ptrdiff_t;
		using value_type = Char;
		using pointer = Char const*;
		using reference = Char const&;
		using iterator_concept = std::contiguous_iterator_tag;

		explicit constexpr iterator(Char const* const ptr) noexcept
			: m_ptr(ptr)
		{
		}

		[[nodiscard]] constexpr Char const& operator*() const
		{
			return *m_ptr;
		}

		[[nodiscard]] constexpr Char const* operator->() const
		{
			return m_ptr;
		}

		constexpr iterator& operator++() &
		{
			++m_ptr;
			return *this;
		}

		[[nodiscard]] constexpr iterator operator++(int) &
		{
			auto r = *this;
			++m_ptr;
			return r;
		}

		constexpr iterator& operator+=(ptrdiff_t const offset) &
		{
			m_ptr += offset;
			return *this;
		}

		constexpr iterator& operator-=(ptrdiff_t const offset) &
		{
			m_ptr -= offset;
			return *this;
		}

		[[nodiscard]] friend constexpr iterator operator+(iterator const self, ptrdiff_t const offset)
		{
			return iterator(self.m_ptr + offset);
		}

		[[nodiscard]] friend constexpr iterator operator+(ptrdiff_t const offset, iterator const self)
		{
			return iterator(self.m_ptr + offset);
		}

		[[nodiscard]] friend constexpr iterator operator-(iterator const self, ptrdiff_t const offset)
		{
			return iterator(self.m_ptr - offset);
		}

		[[nodiscard]] friend constexpr bool operator==(iterator const self, sentinel)
		{
			return *self.m_ptr == static_cast<Char>(0);
		}
	};

	using traits_type = std::char_traits<Char>;
	using value_type = Char;
	using pointer = Char*;
	using const_pointer = Char const*;
	using reference = Char&;
	using const_reference = Char const&;
	using const_iterator = iterator;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	constexpr basic_zstring_ptr() noexcept
		: m_ptr(detail::zstr_empty<Char>)
	{
	}

	constexpr basic_zstring_ptr(Char const* const c_str)
		: m_ptr(c_str)
	{
	}

	constexpr basic_zstring_ptr(decltype(nullptr)) = delete;


	[[nodiscard]] constexpr iterator begin() const noexcept
	{
		return iterator(m_ptr);
	}

	[[nodiscard]] constexpr iterator cbegin() const noexcept
	{
		return iterator(m_ptr);
	}

	[[nodiscard]] constexpr sentinel end() const noexcept
	{
		return sentinel();
	}

	[[nodiscard]] constexpr sentinel cend() const noexcept
	{
		return sentinel();
	}


	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return *m_ptr == static_cast<Char>(0);
	}

	[[nodiscard]] constexpr Char const* data() const noexcept
	{
		return m_ptr;
	}

	[[nodiscard]] constexpr Char const* c_str() const noexcept
	{
		return m_ptr;
	}


	[[nodiscard]] friend constexpr bool operator==(basic_zstring_ptr const& lhs, basic_zstring_ptr const& rhs)
	{
		return detail::zstr_cmp(lhs.m_ptr, rhs.m_ptr) == 0;
	}

	[[nodiscard]] friend constexpr auto operator<=>(basic_zstring_ptr const& lhs, basic_zstring_ptr const& rhs)
	{
		return detail::zstr_cmp(lhs.m_ptr, rhs.m_ptr) <=> 0;
	}

	[[nodiscard]] friend constexpr bool operator==(basic_zstring_ptr const& lhs, basic_string_view<Char> const& rhs)
	{
		return detail::zstr_ncmp(lhs.m_ptr, static_cast<size_t>(-1), rhs.data(), rhs.size()) == 0;
	}

	[[nodiscard]] friend constexpr auto operator<=>(basic_zstring_ptr const& lhs, basic_string_view<Char> const& rhs)
	{
		return detail::zstr_ncmp(lhs.m_ptr, static_cast<size_t>(-1), rhs.data(), rhs.size()) <=> 0;
	}
};

using zstring_ptr = basic_zstring_ptr<char>;
using wzstring_ptr = basic_zstring_ptr<wchar_t>;
using u8zstring_ptr = basic_zstring_ptr<char8_t>;
using u16zstring_ptr = basic_zstring_ptr<char16_t>;
using u32zstring_ptr = basic_zstring_ptr<char32_t>;

} // namespace vsm
