#pragma once

#include <vsm/sio/concepts.hpp>

namespace vsm::sio {

template<source Source>
class window_source
{
	[[no_unique_address]] Source m_source;
	uint64_t m_offset;
	uint64_t m_extent;

public:
	static result<window_source> make(any_cvref_of<Source> auto&& source, uint64_t const offset, uint64_t const extent)
	{
		window_source window(vsm_forward(source), offset, extent);
		vsm_try_void(window.seek(0));
		return window;
	}

	result<void> seek(uint64_t const offset)
	{
		if (offset > m_extent)
		{
			return vsm::error(sio::error::seek_out_of_range);
		}

		return m_source.seek(m_offset + offset);
	}

private:
	explicit window_source(any_cvref_of<Source> auto&& source, uint64_t const offset, uint64_t const extent)
		: m_source(vsm_forward(source))
		, m_offset(offset)
		, m_extent(extent)
	{
	}
};

template<source Source>
window_source<std::remove_cvref_t<Source>> window(Source&& source, uint64_t const offset, uint64_t const extent)
{
	return window_source<std::remove_cvref_t<Source>>::make(vsm_forward(source), offset, extent);
}

} // namespace vsm::sio
