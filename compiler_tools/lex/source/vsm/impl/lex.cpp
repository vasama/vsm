#include <vsm/lex.hpp>

#include <vsm/platform.h>

using namespace vsm;
using namespace vsm::lex;

size_t lexer::lex(token_integer_type* const token_buffer, offset_integer_type* const offset_buffer, size_t const buffer_size)
{
	// First transition gets special handling.
	if (m_state == initial_state)
	{
		uint8_t const input = *m_source_pos++;
		uint32_t const input_class = m_tables->input_class[input];
		m_state = m_tables->transitions[input_class] & 0xFFFF;
	}

	size_t token_count = lex_internal(token_buffer, offset_buffer, buffer_size);

	// Handle end of file.
	if (m_source_pos == m_source_end)
	{
		token_buffer[token_count] = m_tables->end_of_file[m_state];
		offset_buffer[token_count] = static_cast<offset_integer_type>(m_source_end - m_source_beg);
		++token_count;
	}

	return token_count;
}

size_t lexer::lex_internal(token_integer_type* const token_buffer_, offset_integer_type* const offset_buffer_, size_t const buffer_size)
{
	token_integer_type* const vsm_restrict token_buffer = token_buffer_;
	offset_integer_type* const vsm_restrict offset_buffer = offset_buffer_;

	uint32_t const* const vsm_restrict input_class_table = m_tables->input_class;
	uint32_t const* const vsm_restrict transitions_table = m_tables->transitions;

	uint32_t state = m_state;

	char const* vsm_restrict source = m_source_beg;
	size_t source_index = m_source_pos - m_source_beg;
	size_t const source_size = m_source_end - m_source_beg;

	size_t token_count = 0;
	while ((source_index < source_size) && (token_count < buffer_size))
	{
		uint8_t const input = source[source_index];
		uint32_t const input_class = input_class_table[input];
		uint32_t const transition = transitions_table[input_class + state];

		token_buffer[token_count] = (transition >> 16) & 0x7FFF;
		offset_buffer[token_count] = static_cast<offset_integer_type>(source_index);

		state = transition & 0xFFFF;
		token_count += transition >> 31;
		source_index += 1;
	}

	m_state = state;
	m_source_pos = source + source_index;

	return token_count;
}
