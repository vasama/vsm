#pragma once

#include <vsm/result.hpp>

#include <span>

#include <cstddef>

namespace vsm {
namespace streams {

template<typename Sink>
concept sink = requires (Sink&& sink, std::span<std::byte const> const data)
{
	{ sink.write_some(data) } -> std::same_as<vsm::result<size_t>>;
};

template<typename Source>
concept source = requires (Source& source, std::span<std::byte> const data)
{
	{ source.read_some(data) } -> std::same_as<vsm::result<size_t>>;
};


template<sink Sink>
[[nodiscard]] constexpr vsm::result<void> flush(Sink&& sink) noexcept
{
	if constexpr (requires { sink.flush(); })
	{
		static_assert(requires
		{
			{ sink.flush() } noexcept -> std::same_as<vsm::result<void>>;
		});

		return sink.flush();
	}
	else
	{
		return {};
	}
}

template<sink Sink>
[[nodiscard]] constexpr vsm::result<size_t> write(
	Sink&& sink,
	std::span<std::byte const> const data) noexcept
{
	if constexpr (requires { sink.write(data); })
	{
		static_assert(requires
		{
			{ sink.write(data) } noexcept -> std::same_as<vsm::result<size_t>>;
		});

		return sink.write(data);
	}
	else
	{
		size_t size = 0;

		while (size < data.size())
		{
			vsm_try(transferred, sink.write_some(data.subspan(size)));
			size += transferred;
		}

		return size;
	}
}


template<typename Sink>
concept direct_sink =
	sink<Sink> &&
	requires (Sink& sink, size_t const size)
	{
		{ sink.direct_write_acquire(size) }
			-> std::same_as<vsm::result<std::span<std::byte>>>;

		{ sink.direct_write_release(size) } -> std::same_as<void>;
	};

template<typename Source>
concept direct_source =
	source<Source> &&
	requires (Source& source, size_t const size)
	{
		{ source.direct_write_acquire(size) }
			-> std::same_as<vsm::result<std::span<std::byte const>>>;

		{ source.direct_write_release(size) } -> std::same_as<void>;
	};

template<direct_sink Sink>
vsm::result<std::span<std::byte>> direct_write_refresh(
	Sink& sink,
	size_t const written_size,
	size_t const min_new_size)
{
	if constexpr (requires { sink.direct_write_refresh(written_size, min_new_size); })
	{
		static_assert(requires
		{
			{ sink.direct_write_refresh(written_size, min_new_size) } noexcept
				-> std::same_as<vsm::result<std::span<std::byte>>>;
		});

		return sink.direct_write_refresh(written_size, min_new_size);
	}
	else
	{
		sink.direct_write_release(written_size);
		return sink.direct_write_acquire(min_new_size);
	}
}


namespace detail {

class fill_chunk_buffer
{
	static constexpr size_t large_size = 256;
	static constexpr size_t small_size = 16;

	std::byte m_buffer[large_size];

public:
	[[nodiscard]] std::span<std::byte const> initialize(std::byte const data, size_t const size)
	{
		if (size >= large_size)
		{
			std::memset(m_buffer, static_cast<uint8_t>(data), large_size);
			return std::span<std::byte const>(m_buffer, large_size);
		}
		else
		{
			std::memset(m_buffer, static_cast<uint8_t>(data), small_size);
			return std::span<std::byte const>(m_buffer, small_size);
		}
	}
};

} // namespace detail

template<sink Sink>
vsm::result<size_t> fill_some(Sink& sink, std::byte const data, size_t const size)
{
	size_t written_size = 0;

	if constexpr (direct_sink<Sink>)
	{
		vsm_try(buffer, sink.direct_write_acquire(1));

		size_t direct_write_size = 0;
		while (true)
		{
			direct_write_size = std::min(size, buffer.size());
			std::memset(buffer.data(), static_cast<uint8_t>(data), direct_write_size);
			written_size += direct_write_size;

			if (written_size == size)
			{
				break;
			}

			if (auto const r = streams::direct_write_refresh(sink, direct_write_size, 1))
			{
				buffer = *r;
			}
			else
			{
				break;
			}
		}

		sink.direct_write_release(direct_write_size);
	}
	else
	{
		detail::fill_chunk_buffer chunk_buffer;
		auto const chunk = chunk_buffer.initialize(data, size);

		while (written_size <= size)
		{
			if (auto const r = sink.write_some(chunk))
			{
				written_size += *r;
			}
			else if (written_size == 0)
			{
				return vsm::unexpected(r.error());
			}
			else
			{
				break;
			}
		}
	}

	return written_size;
}

} // namespace streams
} // namespace vsm
