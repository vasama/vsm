#include <vsm/impl/lexer.hpp>

#include <cstdint>

using namespace vsm;
using namespace vsm::lexgen;

static bool is_integer_char(char const c)
{
	return c >= '0' && c <= '9';
}

static bool is_identifier_begin_char(char const c)
{
	return
		c == '_' ||
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
}

static bool is_identifier_char(char const c)
{
	return is_identifier_begin_char(c) || is_integer_char(c);
}

static bool skip_delimited(char const** const p_beg, char const* const end, char const delimiter)
{
	char const* beg = *p_beg;
	bool result = false;
	
	while (beg != end)
	{
		char const c = *beg++;
		if (c == delimiter)
		{
			result = true;
			break;
		}
		
		if (c == '\\')
		{
			if (beg == end)
			{
				break;
			}
			++beg;
		}
	}

	*p_beg = beg;
	return result;
}

size_t lexer::lex(std::span<raw_token> const buffer)
{
	using tk = token_kind;

	char const* const beg = m_source_beg;
	char const* pos = m_source_pos;
	char const* const end = m_source_end;

	raw_token* out_pos = buffer.data();
	raw_token* const out_end = out_pos + buffer.size();

	auto const create_token = [&](tk const kind, char const* const token_beg, char const* const token_end)
	{
		*out_pos++ = raw_token
		{
			.kind = kind,
			.end_offset = static_cast<source_offset>(token_end - beg),
		};
		pos = token_end;
	};

	char const* error_beg = nullptr;
	auto const commit_error = [&](char const* const end) -> bool
	{
		if (error_beg != nullptr)
		{
			create_token(tk::error_trivia, error_beg, end);
			error_beg = nullptr;
			return true;
		}
		return false;
	};

	for (const char* p = pos; out_pos != out_end;)
	{
		if (p == end)
		{
			commit_error(end);
			break;
		}
		
		char const* const token_beg = pos;

		tk kind;
		switch (char c = *p++)
		{
		case ' ':
		case '\t':
		case '\r':
			while (p != end)
			{
				switch (*p)
				{
				case ' ':
				case '\t':
				case '\r':
					++p;
					continue;
				}
			}
			[[fallthrough]]

		case '\n':
			kind = tk::whitespace_trivia;
			break;

		case '#':
			while (p != end && *p++ != '\n')
			{
			}
			kind = tk::line_comment_trivia;
			break;

		case '@':
			kind = tk::at;
			break;

		case ':':
			kind = tk::colon;
			break;

		case ';':
			kind = tk::semicolon;
			break;

		case ',':
			kind = tk::comma;
			break;

		case '(':
			kind = tk::lparen;
			break;

		case ')':
			kind = tk::rparen;
			break;

		case '{':
			kind = tk::lbrace;
			break;

		case '}':
			kind = tk::rbrace;
			break;

		case '?':
			kind = tk::question;
			break;

		case '*':
			kind = tk::star;
			break;

		case '+':
			kind = tk::plus;
			break;

		case '|':
			kind = tk::pipe;
			break;

		case '\'':
			if (!skip_delimited(&p, end, '\''))
			{
				goto error_token;
			}
			kind = tk::literal_sequence;
			break;

		case '[':
			if (!skip_delimited(&p, end, ']'))
			{
				goto error_token;
			}
			kind = tk::literal_alternative;
			break;
			
		default:
			if (is_integer_char(c))
			{
				while (p != end && is_integer_char(*p))
				{
					++p;
				}
				kind = tk::integer;
				break;
			}
			
			if (is_identifier_begin_char(c))
			{
				while (p != end && is_identifier_char(*p))
				{
					++p;
				}
				kind = tk::identifier;
				break;
			}

		error_token:
			if (error_beg == nullptr)
			{
				error_beg = token_beg;
			}
			continue;
		}
	
		if (commit_error(token_beg) && out_pos == out_end)
		{
			break;
		}
		
		create_token(kind, token_beg, p);
	}

	return out_pos - buffer.data();
}
