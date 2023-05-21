#include <vsm/impl/syntax.hpp>

#include <vsm/impl/lexer.hpp>

#include <vsm/compilers/parser.hpp>

using namespace vsm;
using namespace vsm::compilers;
using namespace vsm::lexgen;

namespace {

struct lex_parser : parser
{
	enum class precedence
	{
		root,
		alternative,
		sequence,
	};

	recursive<expr*> parse_expr(precedence const precedence)
	{
		auto const is_expr_first = [](token_kind const kind)
		{
			switch (kind)
			{
			case tk::identifier:
			case tk::literal_alternative:
			case tk::literal_sequence:
			case tk::lparen:
				return true;
			}
			return false;
		};

		expr* expr = nullptr;

		switch (peek_token()->kind)
		{
		case tk::identifier:
			{
				auto expr_node = node<string_expr>(sk::reference_expr);
				expr_node->string_token = consume_token();
				expr = expr_node.leave();
			}
			break;
		
		case tk::literal_sequence:
			{
				auto expr_node = node<string_expr>(sk::literal_sequence_expr);
				expr_node->string_token = consume_token();
				expr = expr_node.leave();
			}
			break;
		
		case tk::literal_alternative:
			{
				auto expr_node = node<string_expr>(sk::literal_alternative_expr);
				expr_node->string_token = consume_token();
				expr = expr_node.leave();
			}
			break;
			
		case tk::lparen:
			{
				auto expr_node = node<parentheses_expr>(sk::parentheses);
				expr_node->lparen_token = consume_token();
				expr_node->expr = co_await parse_expr(precedence());
				expr_node->rparen_token = consume_token(tk::rparen);
				expr = expr_node.leave();
			}
			break;
		
		default:
			{
				//TODO:
				//diagnostics.report();
				vsm_assert(false);
			}
			break;
		}

		switch (peek_token()->kind)
		{
			{
				sk kind;
			
			case tk::question:
				kind = sk::zero_or_one_expr;
				goto suffix_expr;
			
			case tk::star:
				kind = sk::zero_or_many_expr;
				goto suffix_expr;
			
			case tk::plus:
				kind = sk::one_or_many_expr;
				goto suffix_expr;
			
			suffix_expr:
				auto expr_node = node<operator_1_expr>(kind);
				expr_node->operator_token = consume_token();
				expr_node->operand = expr;
				expr = expr_node.leave();
			}
			break;
	
		case tk::lbrace:
			{
				auto expr_node = node<min_max_expr>(sk::min_max_expr);
				expr_node->lbrace_token = consume_token();
				expr_node->min_token = consume_token(tk::integer);
				if (auto const comma_token = try_consume_token(tk::comma))
				{
					expr_node->comma_token = comma_token;
					expr_node->max_token = consume_token(tk::integer);
				}
				expr_node->rbrace_token = consume_token(tk::rbrace);
				expr = expr_node.leave();
			}
			break;
		}

		/**/ if (precedence < precedence::alternative)
		{
			while (peek_token()->kind == tk::pipe)
			{
				auto const* expr_node = node<operator_2_expr>(sk::alternative_expr);
				expr_node->operator_token = consume_token();
				expr_node->lhs_operand = expr;
				expr_node->rhs_operand = co_await parse_expr(precedence::alternative);
				expr = expr_node.leave();
			}
		}
		else if (precedence < precedence::sequence)
		{
			while (is_expr_first(peek_token()->kind))
			{
				auto expr_node = node<operator_2_expr>(sk::sequence_expr);
				expr_node->lhs_operand = expr;
				expr_node->rhs_operand = co_await parse_expr(precedence::sequence);
				expr = expr_node.leave();
			}
		}

		co_return expr;
	}

	rule* parse_rule()
	{
		auto rule_node = node<rule>();
		
		// Parse rule attributes.
		{
			auto attribute_list = list<attribute>();
	
			while (peek_token()->kind == tk::at)
			{
				auto attribute_node = node<attribute>();
				attribute_node->at_token = consume_token();
				attribute_node->name_token = consume_token(tk::identifier);
				attribute_list.push(attribute_node.leave());
			}

			rule_node->attribute = attribute_list.leave();
		}

		rule_node->name_token = consume_token(tk::identifier);
		rule_node->colon_token = consume_token(tk::colon);
		rule_node->expr = recurse(parse_expr(precedence()));
		rule_node->semicolon_token = consume_token(tk::semicolon);

		return rule_node.leave();
	}
};

} // namespace


