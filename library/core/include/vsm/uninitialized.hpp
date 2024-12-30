#pragma once

namespace vsm {

struct uninitialized_t
{
	explicit uninitialized_t() = default;
};
inline constexpr uninitialized_t uninitialized{};

} // namespace vsm
