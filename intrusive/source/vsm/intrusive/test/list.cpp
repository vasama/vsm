#include <vsm/intrusive/list.hpp>

#include <vsm/intrusive/test/elements.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::test;

namespace {

using list = vsm::intrusive::list<element>;

TEST_CASE("list::push_back", "[intrusive][list]")
{
	list list;
	elements e;

	list.push_back(e(1));
	CHECK(list.size() == 1);
	CHECK(list.front()->value == 1);
	CHECK(list.back()->value == 1);

	list.push_back(e(2));
	CHECK(list.size() == 2);
	CHECK(list.front()->value == 1);
	CHECK(list.back()->value == 2);

	list.push_back(e(3));
	CHECK(list.size() == 3);
	CHECK(list.front()->value == 1);
	CHECK(list.back()->value == 3);
}

TEST_CASE("list::push_front", "[intrusive][list]")
{
	list list;
	elements e;

	list.push_front(e(1));
	CHECK(list.size() == 1);
	CHECK(list.front()->value == 1);
	CHECK(list.back()->value == 1);

	list.push_front(e(2));
	CHECK(list.size() == 2);
	CHECK(list.front()->value == 2);
	CHECK(list.back()->value == 1);

	list.push_front(e(3));
	CHECK(list.size() == 3);
	CHECK(list.front()->value == 3);
	CHECK(list.back()->value == 1);
}

TEST_CASE("list iteration.", "[intrusive][list]")
{
	list list;
	elements e;

	int const count = GENERATE(0, 100);

	for (int i = 0; i < count; ++i)
	{
		list.push_back(e(i));
	}

	auto beg = std::ranges::begin(list);
	auto const end = std::ranges::end(list);

	for (int i = 0; i < count; ++i, ++beg)
	{
		REQUIRE(beg != end);
		CHECK(beg->value == i);
	}

	CHECK(beg == end);
}

TEST_CASE("list::remove" "[intrusive][list]")
{
	list list;
	elements e;

	element* elements[5];
	for (int i = 0; i < 5; ++i)
	{
		list.push_back(elements[i] = e(i));
	}

	int const remove = GENERATE(0, 2, 4);
	list.remove(elements[remove]);

	int expected[4];
	for (int i = 0, j = 0; i < 5; ++i)
	{
		if (i != remove)
		{
			expected[j++] = i;
		}
	}

	CHECK(std::ranges::equal(expected, test::values(list)));
}

TEST_CASE("list::remove during iteration.", "[intrusive][list]")
{
	list list;
	elements e;

	list.push_back(e(1));
	list.push_back(e(2));
	list.push_back(e(3));

	auto beg = std::ranges::begin(list);
	auto const end = std::ranges::end(list);

	REQUIRE(beg != end);
	CHECK(beg->value == 1);
	{
		auto const next = std::next(beg);
		REQUIRE(next != end);
		CHECK(next->value == 2);
		list.remove(&*next);
	}
	REQUIRE(beg != end);
	CHECK(beg->value == 1);
	{
		auto const next = std::next(beg);
		REQUIRE(next != end);
		CHECK(next->value == 3);
		list.remove(&*next);
	}
	REQUIRE(beg != end);
	CHECK(beg->value == 1);

	CHECK(++beg == end);
}

} // namespace
