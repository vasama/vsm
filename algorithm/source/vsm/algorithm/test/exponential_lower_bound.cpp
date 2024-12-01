#include <vsm/algorithm/exponential_lower_bound.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

static std::pair<std::vector<int>, int> generate_data();

TEST_CASE("exponential_lower_bound", "[algorithm]")
{
	auto const [haystack, needle] = generate_data();

	auto const lower_bound = exponential_lower_bound(
		haystack.begin(),
		haystack.end(),
		needle);

	auto const std_lower_bound = std::lower_bound(
		haystack.begin(),
		haystack.end(),
		needle);

	REQUIRE(lower_bound == std_lower_bound);
}

TEST_CASE("exponential_lower_bound_near", "[algorithm]")
{
	auto const [haystack, needle] = generate_data();

	auto hint = haystack.begin();

	SECTION("hint = end")
	{
		hint = haystack.end();
	}

	//SECTION("hint = random")
	//{
	//}

	auto const lower_bound = exponential_lower_bound_near(
		haystack.begin(),
		hint,
		haystack.end(),
		needle);

	auto const std_lower_bound = std::lower_bound(
		haystack.begin(),
		haystack.end(),
		needle);

	REQUIRE(lower_bound == std_lower_bound);
}

} // namespace
