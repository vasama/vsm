#pragma once

#include <vsm/xxhash.hpp>

namespace vsm {

using default_hash = xxhash;
using default_hasher = basic_hasher<default_hash>;

} // namespace vsm
