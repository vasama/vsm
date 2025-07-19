#pragma once

#include <vsm/any_ref.hpp>
#include <vsm/streams/streams.hpp>

namespace vsm::streams {
namespace detail {

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

} // namespace detail

class any_sink_ref
	: vsm::any_ref<
		detail::any_sink_write_some,
		detail::any_sink_write,
		detail::any_sink_flush>
{
public:
	using any_ref::any_ref;

	[[nodiscard]] constexpr vsm::result<size_t> write_some(
		std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write_some>(data);
	}

	[[nodiscard]] constexpr vsm::result<size_t> write(
		std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write>(data);
	}

	[[nodiscard]] constexpr vsm::result<void> flush() const noexcept
	{
		return any_ref::invoke<detail::any_sink_flush>();
	}
};

class any_direct_sink_ref
	: vsm::any_ref<
		detail::any_sink_write_some,
		detail::any_sink_write,
		detail::any_sink_flush,
		detail::any_sink_direct_write_acquire,
		detail::any_sink_direct_write_refresh,
		detail::any_sink_direct_write_release>
{
public:
	using any_ref::any_ref;

	[[nodiscard]] constexpr vsm::result<size_t> write_some(
		std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write_some>(data);
	}

	[[nodiscard]] constexpr vsm::result<size_t> write(
		std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write>(data);
	}

	[[nodiscard]] constexpr vsm::result<void> flush() const noexcept
	{
		return any_ref::invoke<detail::any_sink_flush>();
	}

	[[nodiscard]] constexpr vsm::result<std::span<std::byte>> direct_write_acquire(
		size_t const size) const noexcept
	{
		return any_ref::invoke<detail::any_sink_direct_write_acquire>(size);
	}

	[[nodiscard]] constexpr vsm::result<std::span<std::byte>> direct_write_refresh(
		size_t const written_size,
		size_t const min_new_size) const noexcept
	{
		return any_ref::invoke<detail::any_sink_direct_write_refresh>(written_size, min_new_size);
	}

	[[nodiscard]] constexpr void direct_write_release(size_t const size) const noexcept
	{
		any_ref::invoke<detail::any_sink_direct_write_release>(size);
	}
};

} // namespace vsm::streams
