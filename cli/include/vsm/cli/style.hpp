#pragma once

#include <vsm/flags.hpp>
#include <vsm/result.hpp>
#include <vsm/sio/write.hpp>

#include <cstdint>

namespace vsm::cli {

enum class color : uint8_t
{
	black = 30,
	red,
	green,
	yellow,
	blue,
	magenta,
	cyan,
	white,

	bright_black = 90,
	bright_red,
	bright_green,
	bright_yellow,
	bright_blue,
	bright_magenta,
	bright_cyan,
	bright_white,
};

enum class emphasis
{
	bold                                = 1 << 0,
	italic                              = 1 << 1,
	underline                           = 1 << 2,
	strikethrough                       = 1 << 3,
};
vsm_flag_enum(emphasis);

class alignas(4) style
{
	static constexpr uint8_t has_foreground                     = 1 << 0;
	static constexpr uint8_t has_background                     = 1 << 1;

	uint8_t m_mask;
	color m_foreground;
	color m_background;
	emphasis m_emphasis;

public:
	style() = default;

	constexpr style(emphasis const emphasis)
		: m_mask{}
		, m_foreground{}
		, m_background{}
		, m_emphasis(emphasis)
	{
	}

	constexpr style(color const foreground)
		: m_mask(has_foreground)
		, m_foreground(foreground)
		, m_background{}
		, m_emphasis{}
	{
	}

	constexpr style(color const foreground, emphasis const emphasis)
		: m_mask(has_foreground)
		, m_foreground(foreground)
		, m_background{}
		, m_emphasis(emphasis)
	{
	}

	constexpr style(color const foreground, color const background)
		: m_mask(has_foreground | has_background)
		, m_foreground(foreground)
		, m_background(background)
		, m_emphasis{}
	{
	}

	constexpr style(color const foreground, color const background, emphasis const emphasis)
		: m_mask(has_foreground | has_background)
		, m_foreground(foreground)
		, m_background(background)
		, m_emphasis(emphasis)
	{
	}

	bool operator==(style const&) const = default;

private:
	// Enough space for formatting any style.
	static constexpr max_style_characters = 3 + 4 * 4 + 2 * 5;

	static void format_to_buffer(style style, char* buffer);

	friend result<size_t> tag_invoke(sio::write_cpo const cpo, style const style, sio::sink auto& sink)
	{
		char buffer[max_style_characters];
		size_t const size = format_to_buffer(style, buffer);
		return cpo(std::string_view(buffer, size), sink);
	}
};

} // namespace vsm::cli
