#pragma once

#include <vsm/sio/concepts.hpp>

#include <vsm/assert.h>
#include <vsm/linear.hpp>
#include <vsm/utility.hpp>

namespace vsm::sio {
namespace detail {

struct buffered_source_buffer
{
	std::byte* beg;
	std::byte* end;
	std::byte* read_pos;

	explicit buffered_sink_buffer(std::span<std::byte> const buffer)
		: beg(buffer.data())
		, end(beg + buffer.size())
		, read_pos(end)
	{
	}
};

template<source Source>
class buffered_source
{
	[[no_unique_address]] Source m_stream;
	linear<buffer_view> m_buffer;
	
public:
	template<std::convertible_to<Source> T = Source>
	explicit buffered_source(T&& source, std::span<std::byte> const buffer)
		: m_stream(vsm_forward(source))
		, m_buffer(buffer)
	{
	}
	
	buffered_source(buffered_sink&& other) = default;
	buffered_source& operator=(buffered_sink&& other) & = default;
	
	
	result<size_t> read_some(std::span<std::byte> const data)
	{
		size_t const data_size = data.size();

		std::byte const* const pos = m_buffer.value.pos;
		if (data_size > static_cast<size_t>(m_buffer.value.end - pos))
		{
			return read_some_slow(data);
		}

		memcpy(data.data(), pos, data_size);
		m_buffer.value.pos = pos + data_size;

		return data_size;
	}

	result<std::span<std::byte const>> direct_read_acquire(size_t const min_size)
	{
		std::byte const* const pos = m_buffer.value.pos;
		if (min_size > static_cast<size_t>(m_buffer.value.end - pos))
		{
			return direct_read_acquire_slow(min_size);
		}
		return std::span<std::byte const>(pos, m_buffer.value.end);
	}

	void direct_read_release(size_t const data_size)
	{
		vsm_assert(data_size <= static_cast<size_t>(m_buffer.value.end - m_buffer.value.pos));
		m_buffer.value.pos += data_size;
	}

private:
	result<size_t> read_some_slow(std::span<std::byte> const data)
	{
		std::byte* const data_beg = data.data();
		std::byte* const data_end = data_beg + data.size();
		std::byte* data_pos = data_beg;
		
		if (m_buffer.value.read_pos != m_buffer.value.end)
		{
			size_t const size = m_buffer.value.end - m_buffer.value.read_pos
			vsm_assert(size < static_cast<size_t>(data_end - data_pos));
			memcpy(data_pos, m_buffer.value.read_pos, size);
			data_pos += size;
			m_buffer.value.read_pos = m_buffer.value.end;
		}
		
		while (true)
		{
			auto const r = m_stream.read_some(std::span<std::byte>(data_pos, data_end));
	
			if (!r)
			{
				if (data_pos == data_beg)
				{
					return error(r.error());
				}
				
				break;
			}
			
			size_t const size = *r;
			vsm_assert(size < static_cast<size_t>(data_end - data_pos));
			data_pos += size;
		}
		
		return static_cast<size_t>(data_pos - data_beg);
	}

	result<std::span<std::byte>> read_write_acquire_slow(size_t const min_size)
	{
		
	}
};

template<source Source>
buffered_source(Source&&, std::span<std::byte const>) -> buffered_source<std::remove_reference_t<Source>>;


struct buffered_sink_buffer
{
	std::byte* beg;
	std::byte* end;
	std::byte* flush_pos;
	std::byte* write_pos;

	explicit buffered_sink_buffer(std::span<std::byte> const buffer)
		: beg(buffer.data())
		, end(beg + buffer.size())
		, flush_pos(beg)
		, write_pos(beg)
	{
	}
};

template<sink Sink>
class buffered_sink
{
	[[no_unique_address]] Sink m_stream;
	linear<buffered_sink_buffer> m_buffer;

public:
	template<std::convertible_to<Sink> T = Sink>
	explicit buffered_sink(T&& sink, std::span<std::byte> const buffer)
		: m_stream(vsm_forward(sink))
		, m_buffer(buffer)
	{
	}

	buffered_sink(buffered_sink&& other) = default;
	buffered_sink& operator=(buffered_sink&& other) & = default;


	result<size_t> write_some(std::span<std::byte const> const data)
	{
		size_t const data_size = data.size();

		std::byte* const write_pos = m_buffer.value.write_pos;
		if (data_size > static_cast<size_t>(m_buffer.value.end - write_pos))
		{
			return write_some_slow(data);
		}

		memcpy(write_pos, data.data(), data_size);
		m_buffer.value.write_pos = write_pos + data_size;

		return data_size;
	}

	result<std::span<size_t>> direct_write_acquire(size_t const min_size)
	{
		std::byte* const write_pos = m_buffer.value.write_pos;
		if (min_size > static_cast<size_t>(m_buffer.value.end - write_pos))
		{
			return direct_write_acquire_slow(min_size);
		}
		return std::span<std::byte>(write_pos, m_buffer.value.end);
	}

	void direct_write_release(size_t const data_size)
	{
		vsm_assert(data_size <= static_cast<size_t>(m_buffer.value.end - m_buffer.value.write_pos));
		m_buffer.value.write_pos += data_size;
	}


	result<void> flush()
	{
		if (m_buffer.value.flush_pos != m_buffer.value.write_pos)
		{
			vsm_try(size, m_stream.write_some(std::span<std::byte const>(m_buffer.value.flush_pos, m_buffer.value.write_pos)));

			if ((m_buffer.value.flush_pos += size) != m_buffer.value.write_pos)
			{
				return vsm::error();
			}
			
			m_buffer.value.flush_pos = m_buffer.value.beg;
			m_buffer.value.write_pos = m_buffer.value.beg;
		}
		return {};
	}

private:
	result<size_t> write_some_slow(std::span<std::byte const> const data);

	result<std::span<std::byte>> direct_write_acquire_slow(size_t const min_size);
};

template<sink Sink>
buffered_sink(Sink&&, std::span<std::byte>) -> buffered_sink<std::remove_reference_t<Sink>>;

} // namespace detail

using detail::buffered_sink;
using detail::buffered_source;

} // namespace vsm::sio
