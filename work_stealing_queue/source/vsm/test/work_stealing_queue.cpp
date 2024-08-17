bool ___debug;

#include <vsm/work_stealing_queue.hpp>

#include <catch2/catch_all.hpp>

#include <deque>

using namespace vsm;

#if 0
TEST_CASE("", "[work_stealing_queue]")
{
	work_stealing_queue<int> queue(/* min_initial_size: */ 2);
	work_stealing_thief<int> thief = queue.get_thief();

	auto const pop_or_steal = [&](int const front, int const back)
	{
		SECTION("Pop")
		{
			REQUIRE(queue.pop_one() == 1);
		}

		SECTION("Steal")
		{

		}
	};

	SECTION("")
	{
		queue.push_one(1);
		REQUIRE(queue.pop_one() == 1);
		REQUIRE(queue.pop_one() == std::nullopt);
		REQUIRE(thief.steal_one() == std::nullopt);
	}

	SECTION("")
	{
		queue.push_one(2);
		REQUIRE(thief.steal_one() == 2);
		REQUIRE(queue.pop_one() == std::nullopt);
		REQUIRE(thief.steal_one() == std::nullopt);
	}

	SECTION("")
	{
		queue.push_one(3);
		queue.push_one(4);

		SECTION("")
		{
			REQUIRE(queue.pop_one() == 4);
			REQUIRE(queue.pop_one() == 3);
		}

		SECTION("")
		{
			REQUIRE(queue.pop_one() == 4);
			REQUIRE(thief.steal_one() == 3);
		}

		SECTION("")
		{
			REQUIRE(thief.steal_one() == 3);
			REQUIRE(queue.pop_one() == 4);
		}

		SECTION("")
		{
			REQUIRE(thief.steal_one() == 3);
			REQUIRE(thief.steal_one() == 4);
		}

		REQUIRE(queue.pop_one() == std::nullopt);
		REQUIRE(thief.steal_one() == std::nullopt);
	}
}
#endif

TEST_CASE("", "[work_stealing_queue]")
{
	static constexpr int count = 10'000;

	work_stealing_queue<int> queue(/* min_initial_size: */ 2);
	work_stealing_thief<int> thief = queue.get_thief();

	std::deque<int> deque;

	for (int i = 0; i < count; ++i)
	{
		queue.push_one(i);
		deque.push_back(i);
	}

	std::uniform_int_distribution<int> d(0, 1);
	for (int i = 0; i < count; ++i)
	{
		if (d(Catch::sharedRng()))
		{
			REQUIRE(queue.take_one().value() == deque.back());
			deque.pop_back();
		}
		else
		{
			REQUIRE(thief.steal_one().value() == deque.front());
			deque.pop_front();
		}
	}
}
