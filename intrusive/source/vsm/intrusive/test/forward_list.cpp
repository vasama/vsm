#include <vsm/intrusive/forward_list.hpp>

#include <vsm/intrusive/test/elements.hpp>

#include <vsm/utility.hpp>

#include <catch2/catch_all.hpp>

#include <initializer_list>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::test;

vsm_clang_diagnostic(ignored "-Wself-move")

namespace {

using list_type = vsm::intrusive::forward_list<element>;

static_assert(std::forward_iterator<list_type::iterator>);
static_assert(std::forward_iterator<list_type::const_iterator>);
static_assert(std::ranges::forward_range<list_type>);

static void check_content(list_type const& l, std::initializer_list<int> const expect)
{
	CHECK(l.empty() == (expect.size() == 0));

	auto it = l.begin();
	auto const end = l.end();

	for (int const e : expect)
	{
		REQUIRE(it != end);
		CHECK(it++->value == e);
	}
}

TEST_CASE("forward_list::push_front", "[intrusive][forward_list]")
{
	list_type l;
	elements e;

	l.push_front(e(1));
	check_content(l, { 1 });
	CHECK(l.front().value == 1);

	l.push_front(e(2));
	check_content(l, { 2, 1 });
	CHECK(l.front().value == 2);

	l.push_front(e(3));
	check_content(l, { 3, 2, 1 });
	CHECK(l.front().value == 3);
}

TEST_CASE("forward_list::push_back", "[intrusive][forward_list]")
{
	list_type l;
	elements e;

	l.push_back(e(1));
	check_content(l, { 1 });
	CHECK(l.front().value == 1);

	l.push_back(e(2));
	check_content(l, { 1, 2 });
	CHECK(l.front().value == 1);

	l.push_back(e(3));
	check_content(l, { 1, 2, 3 });
	CHECK(l.front().value == 1);
}

TEST_CASE("forward_list::pop_front", "[intrusive][forward_list]")
{
	list_type l;
	elements e;

	l.push_front(e(1));
	l.push_front(e(2));
	l.push_front(e(3));

	CHECK(l.pop_front().value == 3);
	check_content(l, { 2, 1 });
	CHECK(l.front().value == 2);

	CHECK(l.pop_front().value == 2);
	check_content(l, { 1 });
	CHECK(l.front().value == 1);

	CHECK(l.pop_front().value == 1);
	check_content(l, {});
}

TEST_CASE("forward_list::splice_front", "[intrusive][forward_list]")
{

}

TEST_CASE("forward_list move constructor", "[intrusive][forward_list]")
{
	elements e;

	list_type l1;
	l1.push_back(e(1));

	list_type l2 = std::move(l1);

	REQUIRE(l1.empty());
	REQUIRE(l2.pop_front().value == 1);

}

TEST_CASE("forward_list move assignment", "[intrusive][forward_list]")
{
	elements e;

	list_type l1;
	l1.push_back(e(1));

	list_type l2;
	l2 = std::move(l1);

	REQUIRE(l1.empty());
	REQUIRE(l2.pop_front().value == 1);
}

TEST_CASE("forward_list self assignment", "[intrusive][forward_list]")
{
	elements e;

	list_type l1;
	l1.push_back(e(1));

	l1 = vsm_move(l1);

	REQUIRE(l1.pop_front().value == 1);
}

} // namespace
