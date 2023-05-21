#pragma once

#include <string_view>

#include <cstdint>

namespace vsm::lex {

using token_integer_type = uint16_t;
using offset_integer_type = size_t;

struct lexer_tables
{
	uint32_t const* input_class;
	uint32_t const* transitions;
	uint16_t const* end_of_file;
};

class lexer
{
	static constexpr uint32_t initial_state = static_cast<uint32_t>(-1);

	lexer_tables const* m_tables;

	char const* m_source_beg;
	char const* m_source_pos;
	char const* m_source_end;

	uint32_t m_state = initial_state;

public:
	explicit lexer(lexer_tables const& tables, std::string_view const source)
		: m_tables(&tables)
		, m_source_beg(source.data())
		, m_source_pos(m_source_beg)
		, m_source_end(m_source_beg + source.size())
	{
	}

	size_t lex(token_integer_type* token_buffer, offset_integer_type* offset_buffer, size_t buffer_size);

private:
	size_t lex_internal(token_integer_type* token_buffer, offset_integer_type* offset_buffer, size_t buffer_size);
};

} // namespace vsm::lex
