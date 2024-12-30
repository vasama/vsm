#include <vsm/ascii.hpp>

#include <vsm/assert.h>
#include <vsm/platform.h>
#include <vsm/preprocessor.h>

#include <bit>

using namespace vsm;
using namespace vsm::ascii;

static constexpr unsigned char max = 'z' - 'a';

static constexpr unsigned char sub_to_lower = 'A';
static constexpr unsigned char add_to_lower = static_cast<unsigned char>('a' - 'A');

static constexpr unsigned char sub_to_upper = 'a';
static constexpr unsigned char add_to_upper = static_cast<unsigned char>('A' - 'a');


static unsigned char ascii_transform_1(
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

static void ascii_to_lower(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg)
{
	ascii_transform_n(
		src_beg,
		src_end,
		out_beg,
		sub_to_lower,
		add_to_lower,
		max);
}

static void ascii_to_upper(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg)
{
	ascii_transform_n(
		src_beg,
		src_end,
		out_beg,
		sub_to_upper,
		add_to_upper,
		max);
}

void ascii::to_lower(std::span<char> const data)
{
	char* const beg = data.data();
	char* const end = beg + data.size();
	ascii_to_lower(beg, end, beg);
}

void ascii::to_upper(std::span<char> const data)
{
	char* const beg = data.data();
	char* const end = beg + data.size();
	ascii_to_upper(beg, end, beg);
}

void ascii::to_lower(std::string_view const string, std::span<char> const buffer)
{
	vsm_assert(buffer.size() >= string.size());
	char const* const src_beg = string.data();
	char const* const src_end = src_beg + string.size();
	ascii_to_lower(src_beg, src_end, buffer.data());
}

void ascii::to_upper(std::string_view const string, std::span<char> const buffer)
{
	vsm_assert(buffer.size() >= string.size());
	char const* const src_beg = string.data();
	char const* const src_end = src_beg + string.size();
	ascii_to_upper(src_beg, src_end, buffer.data());
}

bool ascii::equal(std::string_view const lhs, std::string_view const rhs)
{
	if (lhs.size() != rhs.size())
	{
		return false;
	}

	int const cmp = ascii_compare(
		lhs.data(),
		rhs.data(),
		lhs.size());

	return cmp == 0;
}

std::strong_ordering ascii::compare(std::string_view const lhs, std::string_view const rhs)
{
	size_t const min_size = std::min(lhs.size(), rhs.size());

	int const cmp = ascii_compare(
		lhs.data(),
		rhs.data(),
		min_size);

	return cmp == 0
		? std::strong_ordering::equal
		: lhs.size() <=> rhs.size();
}
