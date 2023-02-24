#pragma once

#include <vsm/propagate_const.hpp>

#include <concepts>
#include <string_view>
#include <span>

#include <cstdint>

namespace vsm::lexgen {

enum class syntax_kind : uint8_t
{
	error_trivia,
	whitespace_trivia,
	line_comment_trivia,


	error_token,

	identifier_token,
	integer_token,

	literal_sequence_token,
	literal_alternative_token,

	at_token,
	colon_token,
	semicolon_token,
	comma_token,
	lparen_token,
	rparen_token,
	lbrace_token,
	rbrace_token,
	question_token,
	star_token,
	plus_token,
	pipe_token,

	end_of_file_token,


	reference_expr,
	literal_sequence_expr,
	literal_alternative_expr,
	sequence_expr,
	alternative_expr,
	zero_or_one_expr,
	zero_or_many_expr,
	one_or_many_expr,
	min_max_expr,

	attr,
	rule,
};

struct source_element
{
	syntax_kind kind;
	std::string_view content;

	source_element(source_element const&) = default;
	source_element& operator=(source_element const&) = default;

protected:
	source_element() = default;
	~source_element() = default;
};

template<std::derived_from<source_element> SourceElement>
using source_element_ptr = vsm::propagate_const<SourceElement*>;

template<std::derived_from<source_element> SourceElement>
using source_element_list = vsm::propagate_const<std::span<vsm::propagate_const<SourceElement*>>>;


struct trivia : source_element
{
};

struct syntax_element : source_element
{
	source_element_ptr<trivia> leading_trivia;
	source_element_ptr<trivia> trailing_trivia;

protected:
	syntax_element() = default;
	~syntax_element() = default;
};


struct token final : syntax_element
{
};

struct syntax : syntax_element
{

protected:
	syntax() = default;
	~syntax() = default;
};


struct expr : syntax
{
protected:
	expr() = default;
	~expr() = default;
};

struct parentheses_expr : expr
{
	source_element_ptr<token> lparen_token;
	source_element_ptr<token> rparen_token;
	source_element_ptr<expr> expr;
};

struct string_expr : expr
{
	source_element_ptr<expr> string_token;
};

struct operator_1_expr : expr
{
	source_element_ptr<token> operator_token;
	source_element_ptr<expr> operand;
};

struct operator_2_expr : expr
{
	source_element_ptr<token> operator_token;
	source_element_ptr<expr> lhs_operand;
	source_element_ptr<expr> rhs_operand;
};

struct min_max_expr : expr
{
	source_element_ptr<token> lbrace_token;
	source_element_ptr<token> rbrace_token;
	source_element_ptr<token> min_token;
	source_element_ptr<token> comma_token;
	source_element_ptr<token> max_token;
};


struct attr : syntax
{
	source_element_ptr<token> at_token;
	source_element_ptr<token> name_token;
};

struct rule : syntax
{
	source_element_list<attr> attr;
	source_element_ptr<token> name_token;
	source_element_ptr<token> colon_token;
	source_element_ptr<expr> expr;
	source_element_ptr<token> semicolon_token;
};

struct syntax_tree
{
	source_element_list<rule> rules;
};

} // namespace vsm::lexgen
