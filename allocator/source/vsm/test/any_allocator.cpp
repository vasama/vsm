#include <vsm/any_allocator.hpp>

#include <vsm/test/allocator.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

TEST_CASE("any_allocator", "[any][allocator]")
{
	test::allocation_scope scope;
	auto const allocator = any_allocator(test::allocator());

	auto allocation = allocator.allocate(100);
	REQUIRE(allocation.buffer != nullptr);
	REQUIRE(allocation.size >= 100);
	REQUIRE(scope.get_allocation_count() == 1);

	memset(allocation.buffer, 1, allocation.size);
	if (size_t const new_size = allocator.resize(allocation, 200))
	{
		memset(allocation.buffer, 2, new_size);
		allocation.size = new_size;
	}

	allocator.deallocate(allocation);
	REQUIRE(scope.get_allocation_count() == 0);
}

} // namespace
