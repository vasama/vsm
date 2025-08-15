#pragma once

#include <vsm/any_ref.hpp>
#include <vsm/streams/detail/any.hpp>

namespace vsm::streams {

class any_sink_ref
	: vsm::any_ref<
		detail::any_sink_write_some,
		detail::any_sink_write,
		detail::any_sink_flush>
{
public:
	using any_ref::any_ref;

	[[nodiscard]] vsm::result<size_t> write_some(
		std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write_some>(data);
	}

	[[nodiscard]] vsm::result<size_t> write(std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write>(data);
	}

	[[nodiscard]] vsm::result<void> flush() const noexcept
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

	[[nodiscard]] vsm::result<size_t> write_some(
		std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write_some>(data);
	}

	[[nodiscard]] vsm::result<size_t> write(std::span<std::byte const> const data) const noexcept
	{
		return any_ref::invoke<detail::any_sink_write>(data);
	}

	[[nodiscard]] vsm::result<void> flush() const noexcept
	{
		return any_ref::invoke<detail::any_sink_flush>();
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> direct_write_acquire(
		size_t const size) const noexcept
	{
		return any_ref::invoke<detail::any_sink_direct_write_acquire>(size);
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> direct_write_refresh(
		size_t const written_size,
		size_t const min_new_size) const noexcept
	{
		return any_ref::invoke<detail::any_sink_direct_write_refresh>(written_size, min_new_size);
	}

	[[nodiscard]] void direct_write_release(size_t const size) const noexcept
	{
		any_ref::invoke<detail::any_sink_direct_write_release>(size);
	}
};

} // namespace vsm::streams
