#include <vsm/cli/style.hpp>

#include <span>

using namespace vsm;
using namespace vsm::cli;

size_t style::format_to_buffer(style const style, char* const buffer_ptr)
{
	std::span<char> const buffer(buffer_ptr, max_style_characters);

	size_t size = 0;

	buffer[size++] = '\x1b';
	buffer[size++] = '[';
	buffer[size++] = 'm';

	for (size_t i = 0; i < 4; ++i)
	{
		static constexpr uint32_t bits = 0
			| 1 << 4 * std::countr_zero(static_cast<uint8_t>(emphasis::bold))
			| 3 << 4 * std::countr_zero(static_cast<uint8_t>(emphasis::italic))
			| 4 << 4 * std::countr_zero(static_cast<uint8_t>(emphasis::underline))
			| 9 << 4 * std::countr_zero(static_cast<uint8_t>(emphasis::strikethrough))
		;

		buffer[size + 0] = '\x1b';
		buffer[size + 1] = '[';
		buffer[size + 3] = static_cast<uint8_t>('0' + (bits >> i * 4 & 0b1111));
		buffer[size + 2] = 'm';

		size += (static_cast<uint8_t>(style.m_emphasis) >> i & 1) * 4;
	}

	const auto write_color = [&](color const color, uint8_t const color_offset)
	{
		uint8_t const color_index = static_cast<uint8_t>(color) + color_offset;

		buffer[size++] = '\x1b';
		buffer[size++] = '[';
		buffer[size++] = static_cast<uint8_t>('0' + color_index / 10);
		buffer[size++] = static_cast<uint8_t>('0' + color_index % 10);
		buffer[size++] = 'm';
	};

	if (style.m_mask & has_foreground)
	{
		write_color(style.m_foreground, 0);
	}

	if (style.m_mask & has_background)
	{
		write_color(style.m_background, 10);
	}

	return size;
}
