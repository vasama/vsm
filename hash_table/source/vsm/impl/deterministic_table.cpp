#include <vsm/detail/deterministic_table.hpp>

using namespace vsm::detail::deterministic_table;

template<typename I>
bucket<I> const bucket<I>::empty_bucket =
{
	.index = std::numeric_limits<I>::max(),
};

template bucket<uint_least16_t> const bucket<uint_least16_t>::empty_bucket;
template bucket<uint_least32_t> const bucket<uint_least32_t>::empty_bucket;
template bucket<size_t> const bucket<size_t>::empty_bucket;
