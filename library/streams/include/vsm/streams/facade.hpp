#pragma once

#include <vsm/result.hpp>

#include <span>

#include <cstddef>

namespace vsm::streams {

template<typename DirectSink>
class direct_sink_facade
{
public:
	[[nodiscard]] vsm::result<size_t> write_some(std::span<std::byte const> const data) noexcept
	{
		DirectSink& self = static_cast<DirectSink&>(*this);

		size_t transferred = 0;
		while (transferred < data.size())
		{
			if (auto const r = self.direct_write_acquire(1))
			{
				size_t const transfer_size = std::min(
					r->size(),
					data.size() - transferred);

				std::memcpy(r->data(), data.data() + transferred, transfer_size);
				self.direct_write_release(transfer_size);
				transferred += transfer_size;
			}
			else
			{
				if (transferred != 0)
				{
					break;
				}

				return vsm::unexpected(r.error());
			}
		}

		return transferred;
	}
};

} // namespace vsm::streams
