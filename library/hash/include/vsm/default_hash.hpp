#pragma once

#include <vsm/xxhash.hpp>

namespace vsm {

using default_hash = xxhash;

template<typename Policy>
using basic_default_hasher = basic_hasher<default_hash, Policy>;

using default_hasher = basic_default_hasher<default_hash_policy>;

} // namespace vsm
