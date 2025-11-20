#pragma once

#include <cstddef>

namespace vsm {

template<size_t Priority>
struct priority_tag : priority_tag<Priority - 1>
{
};

template<>
struct priority_tag<0>
{
};

} // namespace vsm
