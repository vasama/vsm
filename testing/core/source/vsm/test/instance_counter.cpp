#include <vsm/testing/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

using test::scoped_count;

struct counted : test::counted {};

TEST_CASE("instance_counter without scope", "[testing][instance_counter]")
{
	REQUIRE(test::root_count == 0);
	{
		counted const c;
		REQUIRE(test::root_count == 1);
	}
	REQUIRE(test::root_count == 0);
}

TEST_CASE("instance_counter with scope", "[testing][instance_counter]")
{
	scoped_count const s1;
	{
		counted const c1;
		REQUIRE(s1.count() == 1);

		{
			scoped_count const s2;
			{
				counted const c2;
				REQUIRE(s1.count() == 1);
				REQUIRE(s2.count() == 1);
			}
			REQUIRE(s1.count() == 0);
		}
		REQUIRE(s1.count() == 1);
	}
	REQUIRE(s1.count() == 0);
}

} // namespace
