#pragma once

#include <vsm/sio/concepts.hpp>

namespace vsm::sio {

template<sink Sink>
result<size_t> read(Sink& sink, std::span<std::byte> const data)
{
	size_t size = 0;

	while (size < data.size())
	{
		auto const r = sink.read_some(data);
	}
	
	return size;
}

} // namespace vsm::sio
