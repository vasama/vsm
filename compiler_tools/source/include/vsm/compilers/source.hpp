#pragma once

#include <string_view>

namespace vsm::compilers {

enum class source_offset : size_t {};

struct source_position
{
	size_t line;
	size_t columns;
};

class source
{
	std::string_view m_text;

public:
	std::string_view get_text() const
	{
		return m_text;
	}

	virtual source_position get_position(source_offset offset) const = 0;
};

enum class source_reference : size_t
{
	null = 0
};

class source_location
{
	source_reference m_source;
	source_offset m_source_offset;
};

class source_bundle
{
public:
	
};

} // namespace vsm::compilers
