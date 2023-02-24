#pragma once

#include <vsm/concepts.hpp>
#include <vsm/result.hpp>

#include <span>
#include <cstddef>

namespace vsm::sio {

template<typename T>
concept source = requires(T& stream, std::span<std::byte> const buffer)
{
	{ stream.read_some(buffer) } -> std::same_as<result<size_t>>;
};

template<typename T>
concept sink = requires(T& stream, std::span<std::byte const> const buffer)
{
	{ stream.write_some(buffer) } -> std::same_as<result<size_t>>;
};


template<typename T>
concept direct_source = source<T> && requires (T& stream, size_t const size)
{
	{ stream.direct_read_acquire(size) } -> std::same_as<result<std::span<std::byte const>>>;
	{ stream.direct_read_release(size) } -> std::same_as<void>;
};

template<typename T>
concept direct_sink = sink<T> && requires (T& stream, size_t const size)
{
	{ stream.direct_write_acquire(size) } -> std::same_as<result<std::span<std::byte>>>;
	{ stream.direct_write_release(size) } -> std::same_as<void>;
};

} // namespace vsm::sio
