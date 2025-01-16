#include <vsm/intrusive/list.hpp>

#include <vsm/intrusive/test/elements.hpp>

#include <catch2/catch_all.hpp>

#include <ranges>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::test;

namespace {

using list_type = vsm::intrusive::list<element>;

static_assert(std::bidirectional_iterator<list_type::iterator>);
static_assert(std::bidirectional_iterator<list_type::const_iterator>);
static_assert(std::ranges::bidirectional_range<list_type>);

static bool check_values(list_type const& list, std::initializer_list<int> const values)
{
	return std::ranges::equal(
		list | std::views::transform([](element const& e) { return e.value; }),
		values);
}

TEST_CASE("list::push_back", "[intrusive][list]")
{
	list_type list;
	elements e;

	list.push_back(e(1));
	CHECK(list.size() == 1);
	CHECK(list.front().value == 1);
	CHECK(list.back().value == 1);

	list.push_back(e(2));
	CHECK(list.size() == 2);
	CHECK(list.front().value == 1);
	CHECK(list.back().value == 2);

	list.push_back(e(3));
	CHECK(list.size() == 3);
	CHECK(list.front().value == 1);
	CHECK(list.back().value == 3);
}

TEST_CASE("list::push_front", "[intrusive][list]")
{
	list_type list;
	elements e;

	list.push_front(e(1));
	CHECK(list.size() == 1);
	CHECK(list.front().value == 1);
	CHECK(list.back().value == 1);

	list.push_front(e(2));
	CHECK(list.size() == 2);
	CHECK(list.front().value == 2);
	CHECK(list.back().value == 1);

	list.push_front(e(3));
	CHECK(list.size() == 3);
	CHECK(list.front().value == 3);
	CHECK(list.back().value == 1);
}

TEST_CASE("list::insert_before and insert_after", "[intrusive][list]")
{
	list_type list;
	elements e;

	auto& e1 = e(1);
	list.push_back(e1);

	auto& e3 = e(3);
	list.push_back(e3);

	SECTION("insert_before e3")
	{
		list.insert_before(e3, e(2));
	}

	SECTION("insert_before e3 iterator")
	{
		list.insert_before(list.make_iterator(e3), e(2));
	}

	SECTION("insert_after e1")
	{
		list.insert_after(e1, e(2));
	}

	SECTION("insert_after e1 iterator")
	{
		list.insert_after(list.make_iterator(e1), e(2));
	}

	CHECK(check_values(list, { 1, 2, 3 }));
}

TEST_CASE("list iteration.", "[intrusive][list]")
{
	list_type list;
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
	list_type list;
	elements e;

	element* elements[5];
	for (int i = 0; i < 5; ++i)
	{
		element& new_element = e(i);
		list.push_back(new_element);
		elements[i] = &new_element;
	}

	int const remove = GENERATE(0, 2, 4);
	list.remove(*elements[remove]);

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
	list_type list;
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
		list.remove(*next);
	}
	REQUIRE(beg != end);
	CHECK(beg->value == 1);
	{
		auto const next = std::next(beg);
		REQUIRE(next != end);
		CHECK(next->value == 3);
		list.remove(*next);
	}
	REQUIRE(beg != end);
	CHECK(beg->value == 1);

	CHECK(++beg == end);
}

TEST_CASE("list element with private link", "[intrusive][list]")
{
	class private_element : list_link
	{
		friend vsm::intrusive::access;
	};

	private_element e;
	vsm::intrusive::list<private_element> list;
	list.push_back(e);
}

} // namespace
