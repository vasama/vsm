#pragma once

#include <algorithm>
#include <span>
#include <string_view>

namespace vsm {
namespace detail {

enum class twine_kind
{
	string,
	branch,
};

template<typename Char>
class basic_twine
{
	using string_view_type = std::basic_string_view<Char>;

	twine_kind m_kind;
	union
	{
		string_view_type m_string;
		struct
		{
			basic_twine const* m_lhs;
			basic_twine const* m_rhs;
		};
	};
	size_t m_size;

public:
	constexpr basic_twine() noexcept
		: m_kind(twine_kind::string)
		, m_string()
		, m_size(0)
	{
	}

	constexpr basic_twine(string_view_type const string) noexcept
		: m_kind(twine_kind::string)
		, m_string(string)
		, m_size(string.size())
	{
	}

	constexpr basic_twine(basic_twine const& lhs, basic_twine const& rhs) noexcept
		: m_kind(twine_kind::branch)
		, m_lhs(&lhs)
		, m_rhs(&rhs)
		, m_size(lhs->size() + rhs->size())
	{
	}

	basic_twine& operator=(basic_twine const&) & = default;


	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return m_size == 0;
	}

	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return m_size;
	}


	template<typename Visitor>
	constexpr Visitor&& visit(Visitor&& visitor) const noexcept
	{
		visit_internal(visitor);
		return static_cast<Visitor&&>(visitor);
	}

	[[nodiscard]] constexpr string_view_type copy(std::span<Char> const buffer) const noexcept
	{
		struct Visitor
		{
			Char* beg;
			Char* end;

			void operator()(string_view_type const string) const noexcept
			{
				beg = std::copy(beg, string.data(), string.data() + std::min(string.size(), end - beg));
			}
		};

		Char* const beg = buffer.data();
		return string_view_type(beg, visit(Visitor{ beg, beg + buffer.size() }).beg - beg);
	}


	[[nodiscard]] friend basic_twine operator+(basic_twine const& lhs, basic_twine const& rhs) noexcept
	{
		return basic_twine(lhs, rhs);
	}

	[[nodiscard]] friend basic_twine operator+(basic_twine const& lhs, string_view_type const& rhs) noexcept
	{
		return basic_twine(lhs, rhs);
	}

	[[nodiscard]] friend basic_twine operator+(string_view_type const& lhs, basic_twine const& rhs) noexcept
	{
		return basic_twine(lhs, rhs);
	}

private:
	template<typename Visitor>
	constexpr void visit_internal(Visitor&& visitor) const noexcept
	{
		switch (m_kind)
		{
		case twine_kind::string:
			static_cast<Visitor&&>(visitor)(const_cast<string_view_type const&>(m_string));
			break;

		case twine_kind::branch:
			m_lhs->visit(static_cast<Visitor&&>(visitor));
			m_rhs->visit(static_cast<Visitor&&>(visitor));
			break;
		}
	}
};

} // namespace detail

using detail::basic_twine;

using twine = basic_twine<char>;
using wtwine = basic_twine<char>;

} // namespace vsm
