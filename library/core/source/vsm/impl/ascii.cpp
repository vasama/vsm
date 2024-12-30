#include <vsm/ascii.hpp>

#include <vsm/assert.h>
#include <vsm/platform.h>
#include <vsm/preprocessor.h>

#include <bit>

using namespace vsm;

namespace {

static constexpr unsigned char max = 'z' - 'a';

static constexpr unsigned char sub_to_lower = 'A';
static constexpr unsigned char add_to_lower = static_cast<unsigned char>('a' - 'A');

static constexpr unsigned char sub_to_upper = 'a';
static constexpr unsigned char add_to_upper = static_cast<unsigned char>('A' - 'a');


static unsigned char _ascii_transform_1(
	unsigned char const src,
	unsigned char const sub,
	unsigned char const add,
	unsigned char const max)
{
	return src - sub > max
		? src
		: src + add;
}

#if __has_include(vsm_pp_include(vsm/impl/ascii/vsm_arch.ipp))
#	include vsm_pp_include(vsm/impl/ascii/vsm_arch.ipp)
#else
#	include <vsm/impl/ascii/generic.ipp>
#endif

} // namespace

void detail::_ascii_to_lower_fast(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg) noexcept
{
	_ascii_transform_n(
		src_beg,
		src_end,
		out_beg,
		sub_to_lower,
		add_to_lower,
		max);
}

void detail::_ascii_to_upper_fast(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg) noexcept
{
	_ascii_transform_n(
		src_beg,
		src_end,
		out_beg,
		sub_to_upper,
		add_to_upper,
		max);
}

std::strong_ordering detail::_ascii_ci_cmp_fast(
	char const* const lhs_beg,
	char const* const lhs_end,
	char const* const rhs_beg,
	char const* const rhs_end) noexcept
{
	size_t const lhs_size = static_cast<size_t>(lhs_end - lhs_beg);
	size_t const rhs_size = static_cast<size_t>(rhs_end - rhs_beg);
	size_t const min_size = std::min(lhs_size, rhs_size);

	int const cmp = _ascii_compare(
		lhs_beg,
		rhs_beg,
		min_size);

	return cmp == 0 ? std::strong_ordering::equal : lhs_size <=> rhs_size;
}
