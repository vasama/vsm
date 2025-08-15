#pragma once

#include <vsm/streams/streams.hpp>

namespace vsm::streams::detail {

struct any_sink_write_some
{
	using signature_type = vsm::result<size_t>(std::span<std::byte const>) noexcept;

	template<sink Sink>
	static vsm::result<size_t> invoke(Sink& sink, std::span<std::byte const> const data) noexcept
	{
		return sink.write_some(data);
	}
};

struct any_sink_write
{
	using signature_type = vsm::result<size_t>(std::span<std::byte const>) noexcept;

	template<sink Sink>
	static vsm::result<size_t> invoke(Sink& sink, std::span<std::byte const> const data) noexcept
	{
		return streams::write(sink, data);
	}
};

struct any_sink_flush
{
	using signature_type = vsm::result<void>() noexcept;

	template<sink Sink>
	static vsm::result<void> invoke(Sink& sink) noexcept
	{
		return streams::flush(sink);
	}
};

struct any_sink_direct_write_acquire
{
	using signature_type = vsm::result<std::span<std::byte>>(size_t) noexcept;

	template<direct_sink Sink>
	static vsm::result<std::span<std::byte>> invoke(Sink& sink, size_t const size) noexcept
	{
		return sink.direct_write_acquire(size);
	}
};

struct any_sink_direct_write_refresh
{
	using signature_type = vsm::result<std::span<std::byte>>(size_t, size_t) noexcept;

	template<direct_sink Sink>
	static vsm::result<std::span<std::byte>> invoke(
		Sink& sink,
		size_t const written_size,
		size_t const min_new_size) noexcept
	{
		return streams::direct_write_refresh(sink, written_size, min_new_size);
	}
};

struct any_sink_direct_write_release
{
	using signature_type = void(size_t) noexcept;

	template<direct_sink Sink>
	static void invoke(Sink& sink, size_t const size) noexcept
	{
		return sink.direct_write_release(size);
	}
};


struct any_source_read_some
{
	using signature_type = vsm::result<size_t>(std::span<std::byte>) noexcept;

	template<source Source>
	static vsm::result<size_t> invoke(Source& source, std::span<std::byte> const data) noexcept
	{
		return source.read_some(data);
	}
};

struct any_source_direct_read_acquire
{
	using signature_type = vsm::result<std::span<std::byte const>>(size_t) noexcept;

	template<direct_source Source>
	static vsm::result<std::span<std::byte const>> invoke(
		Source& source,
		size_t const size) noexcept
	{
		return source.direct_read_acquire(size);
	}
};

struct any_source_direct_read_release
{
	using signature_type = vsm::result<void>(size_t) noexcept;

	template<direct_source Source>
	static vsm::result<void> invoke(Source& source, size_t const size) noexcept
	{
		return source.direct_read_release(size);
	}
};

} // namespace vsm::streams::detail
