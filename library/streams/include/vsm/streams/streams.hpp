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

} // namespace streams
} // namespace vsm
