#include <vsm/any_allocator.hpp>

#include <vsm/testing/allocator.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

TEST_CASE("any_allocator", "[any][allocator]")
{
	test::allocation_scope const scope;
	auto const allocator = any_allocator(test::allocator());

	auto allocation = vsm::allocate_at_least(allocator, 100);
	REQUIRE(allocation.storage != nullptr);
	REQUIRE(allocation.size >= 100);
	REQUIRE(scope.get_allocation_count() == 1);

	memset(allocation.storage, 1, allocation.size);
	if (size_t const new_size = allocator.resize(allocation, 200, 300))
	{
		memset(allocation.storage, 2, new_size);
		allocation.size = new_size;
	}

	allocator.deallocate(allocation);
	REQUIRE(scope.get_allocation_count() == 0);
}

} // namespace
