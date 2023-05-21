#pragma once

#include <vsm/compilers/cast.hpp>

#include <vsm/propagate_const.hpp>

#include <concepts>
#include <string_view>
#include <span>

#include <cstdint>

namespace vsm::compilers {

enum class source_offset : size_t {};

struct source_span
{
	source_offset beg;
	source_offset end;
};


enum class source_element_kind : uint32_t {};

class source_element
{
public:
	template<std::derived_from<source_element> E>
	struct traits
	{
		static constexpr source_element_kind min_kind = E::min_kind;
		static constexpr source_element_kind max_kind = E::max_kind;
	};

	static constexpr source_element_kind min_kind = source_element_kind(0x00000000);
	static constexpr source_element_kind max_kind = source_element_kind(0xffffffff);

	source_element_kind kind;

	source_element(source_element const&) = default;
	source_element& operator=(source_element const&) = default;

protected:
	source_element() = default;
	~source_element() = default;

private:
	template<std::derived_from<source_element> To>
	friend bool tag_invoke(is_a_cpo, tag<To>, source_element const& e)
	{
		return traits<To>::min_kind <= e.kind && e.kind <= traits<To>::max_kind;
	}
};

template<std::derived_from<source_element> SourceElement>
using source_element_ptr = vsm::propagate_const<SourceElement*>;

template<std::derived_from<source_element> SourceElement>
using source_element_list = vsm::propagate_const<std::span<vsm::propagate_const<SourceElement*>>>;

template<std::derived_from<source_element> SourceElement>
using source_element_const_list = std::span<vsm::propagate_const<SourceElement*> const>;


class lexeme : public source_element
{
public:
	static constexpr source_element_kind min_kind = source_element_kind(0x00000000);
	static constexpr source_element_kind max_kind = source_element_kind(0x7fffffff);

	source_span span;

protected:
	lexeme() = default;
	~lexeme() = default;
};

class trivia final : public lexeme
{
public:
	static constexpr source_element_kind min_kind = source_element_kind(0x00000000);
	static constexpr source_element_kind max_kind = source_element_kind(0x3fffffff);
};

class token final : public lexeme
{
public:
	static constexpr source_element_kind min_kind = source_element_kind(0x40000000);
	static constexpr source_element_kind max_kind = source_element_kind(0x7fffffff);

	source_element_list<trivia> leading_trivia;
	source_element_list<trivia> trailing_trivia;
};

class syntax : public source_element
{
public:
	static constexpr source_element_kind min_kind = source_element_kind(0x80000000);
	static constexpr source_element_kind max_kind = source_element_kind(0xffffffff);

	vsm::propagate_const<std::span<token>> tokens;

	source_span get_source_span() const
	{
		return
		{
			.beg = tokens.get().front().span.beg,
			.end = tokens.get().back().span.end,
		};
	}

	source_element_const_list<trivia> get_leading_trivia() const
	{
		return tokens.get().front().leading_trivia.get();
	}

	source_element_const_list<trivia> get_trailing_trivia() const
	{
		return tokens.get().back().trailing_trivia.get();
	}

protected:
	syntax() = default;
	~syntax() = default;
};

class syntax_tree
{
public:
	syntax_tree(syntax_tree const&) = delete;
	syntax_tree& operator=(syntax_tree const&) = delete;

protected:
	syntax_tree() = default;
	~syntax_tree() = default;
};

} // namespace vsm::compilers
