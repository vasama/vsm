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
	q.push_back(e(1));

	forward_list<element> l = q.pop_all();
	REQUIRE(l.pop_front()->value == 1);
}

} // namespace
