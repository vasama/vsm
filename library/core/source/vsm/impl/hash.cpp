#include <vsm/hash.hpp>

using namespace vsm;
using namespace vsm::detail;

constinit void const* const detail::aslr_seed = static_cast<void const*>(&aslr_seed);
