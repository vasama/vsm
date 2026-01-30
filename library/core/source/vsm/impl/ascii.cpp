#include <vsm/ascii.hpp>

#include <vsm/assert.h>
#include <vsm/platform.h>
#include <vsm/preprocessor.h>

#include <bit>

using namespace vsm;

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

#if __has_include(vsm_pp_include(vsm/impl/ascii/vsm_arch.cpp))
#	include vsm_pp_include(vsm/impl/ascii/vsm_arch.cpp)
#else
#	include <vsm/impl/ascii/generic.cpp>
#endif

static void _ascii_to_lower(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg)
{
	_ascii_transform_n(
		src_beg,
		src_end,
		out_beg,
		sub_to_lower,
		add_to_lower,
		max);
}

static void _ascii_to_upper(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg)
{
	_ascii_transform_n(
		src_beg,
		src_end,
		out_beg,
		sub_to_upper,
		add_to_upper,
		max);
}

template<>
void vsm::ascii_to_lower<char>(std::span<char> const data) noexcept
{
	char* const beg = data.data();
	char* const end = beg + data.size();
	_ascii_to_lower(beg, end, beg);
}

template<>
void vsm::ascii_to_upper<char>(std::span<char> const data) noexcept
{
	char* const beg = data.data();
	char* const end = beg + data.size();
	_ascii_to_upper(beg, end, beg);
}

template<>
void vsm::ascii_to_lower<char>(std::string_view const string, std::span<char> const buffer) noexcept
{
	vsm_assert(buffer.size() >= string.size());
	char const* const src_beg = string.data();
	char const* const src_end = src_beg + string.size();
	_ascii_to_lower(src_beg, src_end, buffer.data());
}

template<>
void vsm::ascii_to_upper<char>(std::string_view const string, std::span<char> const buffer) noexcept
{
	vsm_assert(buffer.size() >= string.size());
	char const* const src_beg = string.data();
	char const* const src_end = src_beg + string.size();
	_ascii_to_upper(src_beg, src_end, buffer.data());
}

template<>
bool vsm::ascii_case_insensitive_equal<char>(std::string_view const lhs, std::string_view const rhs) noexcept
{
	if (lhs.size() != rhs.size())
	{
		return false;
	}

	int const cmp = _ascii_compare(
		lhs.data(),
		rhs.data(),
		lhs.size());

	return cmp == 0;
}

template<>
std::strong_ordering vsm::ascii_case_insensitive_compare<char>(
	std::string_view const lhs,
	std::string_view const rhs) noexcept
{
	size_t const min_size = std::min(lhs.size(), rhs.size());

	int const cmp = _ascii_compare(
		lhs.data(),
		rhs.data(),
		min_size);

	return cmp == 0
		? std::strong_ordering::equal
		: lhs.size() <=> rhs.size();
}

template void vsm::ascii_to_lower<char>(std::span<char> string) noexcept;
template void vsm::ascii_to_lower<char>(
	std::basic_string_view<char> src,
	std::span<char> dst) noexcept;

template void vsm::ascii_to_upper<char>(std::span<char> string) noexcept;
template void vsm::ascii_to_upper<char>(
	std::basic_string_view<char> src,
	std::span<char> dst) noexcept;

template bool vsm::ascii_case_insensitive_equal<char>(
	std::string_view lhs,
	std::string_view rhs) noexcept;

template std::strong_ordering vsm::ascii_case_insensitive_compare<char>(
	std::string_view lhs,
	std::string_view rhs) noexcept;
