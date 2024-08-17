#include <vsm/test/instance_counter.hpp>

using namespace vsm::test;
using namespace vsm::test::detail;

template instance_count_type detail::g_root_instance_count<instance_count_default_t>;
template instance_count_type* detail::g_current_instance_count<instance_count_default_t>;
