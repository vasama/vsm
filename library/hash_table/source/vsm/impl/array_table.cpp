#include <vsm/detail/array_table.hpp>

using namespace vsm;
using namespace vsm::detail;

template _array_table_bucket<uint_least16_t> const detail::_array_table_empty_bucket<uint_least16_t>;
template _array_table_bucket<uint_least32_t> const detail::_array_table_empty_bucket<uint_least32_t>;
template _array_table_bucket<size_t> const detail::_array_table_empty_bucket<size_t>;
