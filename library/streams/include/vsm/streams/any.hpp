#pragma once

#include <vsm/any.hpp>
#include <vsm/streams/detail/any.hpp>

namespace vsm::streams {

struct any_sink
	: vsm::any<
		detail::any_sink_write_some,
		detail::any_sink_write,
		detail::any_sink_flush>
{
	using basic_any::basic_any;

	[[nodiscard]] vsm::result<size_t> write_some(std::span<std::byte const> const data) noexcept
	{
		return basic_any::invoke<detail::any_sink_write_some>(data);
	}

	[[nodiscard]] vsm::result<size_t> write(std::span<std::byte const> const data) noexcept
	{
		return basic_any::invoke<detail::any_sink_write>(data);
	}

	[[nodiscard]] vsm::result<void> flush() noexcept
	{
		return basic_any::invoke<detail::any_sink_flush>();
	}
};

struct any_direct_sink
	: vsm::any<
		detail::any_sink_write_some,
		detail::any_sink_write,
		detail::any_sink_flush,
		detail::any_sink_direct_write_acquire,
		detail::any_sink_direct_write_refresh,
		detail::any_sink_direct_write_release>
{
	using basic_any::basic_any;

	[[nodiscard]] vsm::result<size_t> write_some(std::span<std::byte const> const data) noexcept
	{
		return basic_any::invoke<detail::any_sink_write_some>(data);
	}

	[[nodiscard]] vsm::result<size_t> write(std::span<std::byte const> const data) noexcept
	{
		return basic_any::invoke<detail::any_sink_write>(data);
	}

	[[nodiscard]] vsm::result<void> flush() noexcept
	{
		return basic_any::invoke<detail::any_sink_flush>();
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> direct_write_acquire(size_t const size) noexcept
	{
		return basic_any::invoke<detail::any_sink_direct_write_acquire>(size);
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> direct_write_refresh(
		size_t const written_size,
		size_t const min_new_size) noexcept
	{
		return basic_any::invoke<detail::any_sink_direct_write_refresh>(written_size, min_new_size);
	}

	[[nodiscard]] void direct_write_release(size_t const size) noexcept
	{
		basic_any::invoke<detail::any_sink_direct_write_release>(size);
	}
};

} // namespace vsm::streams
