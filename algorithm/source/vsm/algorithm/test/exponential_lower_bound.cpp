#include <vsm/algorithm/exponential_lower_bound.hpp>

#include <catch2/catch_all.hpp>

#include <random>

using namespace vsm;

namespace {

static std::pair<std::vector<int>, int> generate_mass_data()
{
	auto& engine = Catch::sharedRng();

	std::pair<std::vector<int>, int> pair;

	pair.first.resize(10'000);
	{
		std::uniform_int_distribution<int> d(
			0,
			std::numeric_limits<int>::max());

		std::generate(
			pair.first.begin(),
			pair.first.end(),
			[&]() { return d(engine); });

		std::ranges::sort(pair.first);
	}

	// Initialize pair.second:
	{
		std::uniform_int_distribution<size_t> d(
			0,
			pair.first.size() - 1);

		pair.second = pair.first[d(engine)];
	}

	return pair;
}

static std::pair<std::vector<int>, int> generate_data()
{
	if (GENERATE(0, 1))
	{
		return { {}, 0 };
	}

	if (GENERATE(0, 1))
	{
		return { { 1, 2, 3 }, GENERATE(0, 1, 2, 3) };
	}

	return generate_mass_data();
}

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

	auto const std_lower_bound = std::lower_bound(
		haystack.begin(),
		haystack.end(),
		needle);

	auto hint = haystack.begin();

	if (!haystack.empty())
	{
		SECTION("hint = end")
		{
			hint = haystack.end();
		}

		SECTION("hint = last")
		{
			hint = haystack.end() - 1;
		}

		if (std_lower_bound != haystack.end())
		{
			SECTION("hint = target")
			{
				hint = std_lower_bound;
			}
		}
	}

	CAPTURE(haystack.size());
	CAPTURE(needle);
	CAPTURE(hint - haystack.begin());

	auto const lower_bound = exponential_lower_bound_near(
		haystack.begin(),
		hint,
		haystack.end(),
		needle);

	REQUIRE(lower_bound == std_lower_bound);
}

} // namespace
