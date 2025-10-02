#pragma once

#include <vsm/streams/streams.hpp>
#include <vsm/utility.hpp>

#include <span>

namespace vsm::streams {

template<sink Sink, typename Buffer>
class basic_buffered_sink
{
	Sink m_sink;

	size_t m_buffer_offset = 0;
	Buffer m_buffer;

public:
	template<std::convertible_to<Sink> S, typename... Args>
		requires std::constructible_from<Buffer, Args...>
	explicit basic_buffered_sink(S&& sink, Args&&... args)
		: m_sink(vsm_forward(sink))
		, m_buffer(vsm_forward(args)...)
	{
	}

	basic_buffered_sink(basic_buffered_sink&&);
	basic_buffered_sink& operator=(basic_buffered_sink&&) &;

	~basic_buffered_sink()
	{
		(void)flush();
	}

	[[nodiscard]] vsm::result<size_t> write_some(std::span<std::byte const> const data) noexcept
	{
		if (m_buffer_offset == m_buffer.size())
		{
			vsm_try_void(_flush());
		}

		if (data.size() >= m_buffer.size())
		{
			if (m_buffer_offset != 0)
			{
				vsm_try_void(_flush());
			}

			return m_sink.write_some(data);
		}

		size_t const max_transfer_size = m_buffer.size() - m_buffer_offset;
		size_t const transfer_size = std::min(max_transfer_size, data.size());

		std::memcpy(m_buffer.data() + m_buffer_offset, data.data(), transfer_size);
		m_buffer_offset += transfer_size;

		return transfer_size;
	}

	[[nodiscard]] vsm::result<void> flush() noexcept
	{
		if (m_buffer_offset != 0)
		{
			return _flush();
		}
		else
		{
			return {};
		}
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> direct_write_acquire(
		size_t const min_size) noexcept
	{
		if (min_size > m_buffer.size() - m_buffer_offset)
		{
			if (min_size > m_buffer.size())
			{
				return vsm::unexpected(std::make_error_code(std::errc::no_buffer_space));
			}

			vsm_try_void(_flush());
		}

		return std::span<std::byte>(m_buffer).subspan(m_buffer_offset);
	}

	void direct_write_release(size_t const written_size) noexcept
	{
		vsm_assert(written_size <= m_buffer.size() - m_buffer_offset);
		m_buffer_offset += written_size;
	}

private:
	vsm::result<void> _flush()
	{
		auto const data = std::span<std::byte>(m_buffer).subspan(0, m_buffer_offset);

		size_t flush_offset = 0;
		while (flush_offset != data.size())
		{
			if (auto const r = m_sink.write_some(data.subspan(flush_offset)))
			{
				flush_offset += *r;
			}
			else
			{
				std::memmove(data.data(), data.data() + flush_offset, data.size() - flush_offset);
				m_buffer_offset -= flush_offset;
				return vsm::unexpected(r.error());
			}
		}

		m_buffer_offset = 0;
		return streams::flush(m_sink);
	}
};

} // namespace vsm::streams
