#include <vsm/execution/run_queue.hpp>

#include <catch2/catch_all.hpp>

#include <exec/async_scope.hpp>

using namespace vsm;
using namespace vsm::execution;

namespace ex = std_execution;

TEST_CASE("run_queue", "[execution][run_queue]")
{
	run_queue queue;
	exec::async_scope scope;

	bool flag = false;
	scope.spawn(ex::on(
		queue.get_scheduler(),
		ex::just() | ex::then([&]()
		{
			flag = true;
		})));

	queue.run_all();
	REQUIRE(flag);
}
