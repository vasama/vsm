#pragma once

#include <string_view>
#include <span>
#include <utility>

#include <cstdint>

namespace vsm::lexgen {

using source_offset = uint32_t;

enum class token_kind : uint8_t
{
	error_trivia,
	whitespace_trivia,
	line_comment_trivia,

	identifier,
	integer,

	literal_sequence,
	literal_alternative,

	at,
	colon,
	semicolon,
	comma,
	lparen,
	rparen,
	lbrace,
	rbrace,
	question,
	star,
	plus,
	pipe,

	end_of_file,
};

struct raw_token
{
	token_kind kind;
	source_offset end_offset;
};

class lexer
{
	char const* m_source_beg;
	char const* m_source_pos;
	char const* m_source_end;

public:
	explicit lexer(std::string_view const source)
		: m_source_beg(source.data())
		, m_source_pos(m_source_beg)
		, m_source_end(m_source_beg + source.size())
	{
	}


	size_t lex(std::span<raw_token> buffer);

#if 0
	std::string_view get_string(source_offset const beg, source_offset const end) const
	{
		return std::string_view(
			m_source_beg + std::to_underlying(beg),
			m_source_beg + std::to_underlying(end));
	}
#endif
};

} // namespace vsm::lexgen
