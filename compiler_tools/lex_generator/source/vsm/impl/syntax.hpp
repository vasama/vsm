#pragma once

#include <vsm/compilers/syntax.hpp>

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

	attribute,
	rule,
};

struct expr : compilers::syntax
{
protected:
	expr() = default;
	~expr() = default;
};

struct parentheses_expr final : expr
{
	compilers::source_element_ptr<token> lparen;
	compilers::source_element_ptr<token> rparen;
	compilers::source_element_ptr<expr> expr;
};

struct literal_expr : expr
{
	compilers::source_element_ptr<expr> literal;
};

struct operator1_expr final : expr
{
	compilers::source_element_ptr<token> operator_;
	compilers::source_element_ptr<expr> operand;
};

struct operator2_expr final : expr
{
	compilers::source_element_ptr<token> operator_;
	compilers::source_element_ptr<expr> lhs_operand;
	compilers::source_element_ptr<expr> rhs_operand;
};

struct min_max_expr final : expr
{
	compilers::source_element_ptr<expr> operand;
	compilers::source_element_ptr<token> lbrace;
	compilers::source_element_ptr<token> rbrace;
	compilers::source_element_ptr<token> min;
	compilers::source_element_ptr<token> comma;
	compilers::source_element_ptr<token> max;
};


struct attribute final : compilers::syntax
{
	compilers::source_element_ptr<token> at;
	compilers::source_element_ptr<token> name;
};

struct rule final : compilers::syntax
{
	compilers::source_element_list<attribute> attributes;
	compilers::source_element_ptr<token> name;
	compilers::source_element_ptr<token> colon;
	compilers::source_element_ptr<expr> expr;
	compilers::source_element_ptr<token> semicolon;
};

struct syntax_tree final : compilers::syntax_tree
{
	compilers::source_element_list<rule> rules;
};

} // namespace vsm::lexgen
