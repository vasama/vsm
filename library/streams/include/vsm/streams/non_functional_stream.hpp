#pragma once

#include <vsm/streams/streams.hpp>

namespace vsm::streams {

class non_functional_stream_t
{
public:
	[[nodiscard]] vsm::result<size_t> write_some(
		std::span<std::byte const> const data) const noexcept
	{
		if (data.empty())
		{
			return {};
		}

		return vsm::unexpected(std::make_error_code(std::errc::function_not_supported));
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> direct_write_acquire(
		size_t const size) const noexcept
	{
		if (size == 0)
		{
			return {};
		}

		return vsm::unexpected(std::make_error_code(std::errc::function_not_supported));
	}

	void direct_write_release(size_t) const noexcept
	{
	}
};

//TODO: Make this constexpr... any_ref is failing with constant referents.
inline non_functional_stream_t non_functional_stream;

} // namespace vsm::streams
