#include <vsm/flags.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

enum class e
{
	a               = 1 << 0,
	b               = 1 << 1,
	c               = 1 << 2,

	ab              = a | b,
	ac              = a | c,
	bc              = b | c,
	abc             = a | b | c,
};
vsm_flag_enum(e);

TEST_CASE("any_flags", "[flags]")
{
	CHECK(any_flags(e::ab, e::a));
	CHECK(any_flags(e::ab, e::b));
	CHECK(!any_flags(e::ab, e::c));
}

TEST_CASE("all_flags", "[flags]")
{
	CHECK(all_flags(e::ab, e::a));
	CHECK(all_flags(e::ab, e::b));
	CHECK(all_flags(e::ab, e::ab));
	CHECK(!all_flags(e::ab, e::c));
	CHECK(!all_flags(e::ab, e::ac));
	CHECK(!all_flags(e::ab, e::bc));
	CHECK(!all_flags(e::ab, e::abc));

	CHECK(all_flags(e::abc, e::a));
	CHECK(all_flags(e::abc, e::b));
	CHECK(all_flags(e::abc, e::c));
	CHECK(all_flags(e::abc, e::ac));
	CHECK(all_flags(e::abc, e::bc));
	CHECK(all_flags(e::abc, e::abc));
}

} // namespace
