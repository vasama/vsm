#pragma once

#include <vsm/main.h>
#include <vsm/platform.h>
#include <vsm/result.hpp>

vsm::result<int> vsm_nothrow_main(int argc, char const* const* argv);
