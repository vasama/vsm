#pragma once

#include <vsm/utility.hpp>

#include <ranges>

namespace vsm {
namespace views {

template<typename Bound>
[[nodiscard]] auto indices(Bound&& bound)
{
	return std::views::iota(std::remove_cvref_t<Bound>(), vsm_forward(bound));
}

} // namespace views
} // namespace vsm
