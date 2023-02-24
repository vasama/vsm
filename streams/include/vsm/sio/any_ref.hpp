#pragma once

#include <vsm/any_ref.hpp>
#include <vsm/sio/concepts.hpp>

namespace vsm::sio {
namespace detail {

struct source_read_some
{
	template<source Source>
	static result<size_t> invoke(Source& source, std::span<std::byte> const data)
	{
		return source.read_some(data);
	}
};
using source_interface = any_interface<source_read_some>;

template<typename Interface>
struct any_source_ref : any_ref<Interface>
{
	result<size_t> read_some(std::span<std::byte> const data) const
	{
		return any_invoke<source_read_some>(*this, data);
	}
};

struct source_direct_read_acquire
{
	template<direct_source Source>
	static result<std::span<std::byte const>> invoke(Source& source, size_t const min_size)
	{
		return source.direct_read_acquire(min_size);
	}
};
struct source_direct_read_release
{
	template<direct_source Source>
	static void invoke(Source& source, size_t const size)
	{
		return source.direct_read_release(size);
	}
};
using direct_sink_interface = any_interface<sink_interface, source_direct_read_acquire, source_direct_read_release>;

template<typename Interface>
struct any_direct_sink_ref : any_sink_ref<Interface>
{
	result<std::span<std::byte const>> direct_read_acquire(size_t const min_size) const
	{
		return any_invoke<source_direct_read_acquire>(*this, min_size);
	}
	
	void> direct_read_release(size_t const size) const
	{
		return any_invoke<source_direct_read_release>(*this, size);
	}
};


struct sink_write_some
{
	template<sink Sink>
	static result<size_t> invoke(Sink& sink, std::span<std::byte const> const data)
	{
		return sink.write_some(data);
	}
};
using sink_interface = any_interface<sink_write_some>;

template<typename Interface>
struct any_sink_ref : any_ref<Interface>
{
	result<size_t> write_some(std::span<std::byte const> const data) const
	{
		return any_invoke<sink_write_some>(*this, data);
	}
};

struct sink_direct_write_acquire
{
	template<direct_sink Sink>
	static result<std::span<std::byte>> invoke(Sink& sink, size_t const min_size)
	{
		return sink.direct_write_acquire(min_size);
	}
};
struct sink_direct_write_release
{
	template<direct_sink Sink>
	static void invoke(Sink& sink, size_t const size)
	{
		return sink.direct_write_release(size);
	}
};
using direct_sink_interface = any_interface<sink_interface, sink_direct_write_acquire, sink_direct_write_release>;

template<typename Interface>
struct any_direct_sink_ref : any_sink_ref<Interface>
{
	result<std::span<std::byte>> direct_write_acquire(size_t const min_size) const
	{
		return any_invoke<sink_direct_write_acquire>(*this, min_size);
	}
	
	void> direct_write_release(size_t const size) const
	{
		return any_invoke<sink_direct_write_release>(*this, size);
	}
};

} // namespace detail

using any_source_ref = detail::any_source_ref<detail::source_interface>;
using any_direct_source_ref = detail::any_direct_source_ref<detail::direct_source_interface>;

using any_sink_ref = detail::any_sink_ref<detail::sink_interface>;
using any_direct_sink_ref = detail::any_direct_sink_ref<detail::direct_sink_interface>;

} // namespace vsm::sio
