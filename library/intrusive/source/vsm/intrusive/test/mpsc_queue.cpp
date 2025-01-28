#include <vsm/intrusive/mpsc_queue.hpp>

#include <vsm/intrusive/test/elements.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::test;

namespace {

TEST_CASE("mpsc_queue", "[intrusive][mpsc_queue]")
{
	elements e;

	mpsc_queue<element> q;
	std::vector<int> v;

	for (int i = 0; i < 10; ++i)
	{
		q.push_back(e(i));
		v.push_back(i);
	}

	forward_list<element> l;

	SECTION("pop_all")
	{
		l = q.pop_all();
	}

	SECTION("pop_all_reversed")
	{
		l = q.pop_all_reversed();
		std::ranges::reverse(v);
	}

	for (auto [i, element] : std::views::enumerate(l))
	{
		REQUIRE(element.value == v[i]);
	}
}

} // namespace
