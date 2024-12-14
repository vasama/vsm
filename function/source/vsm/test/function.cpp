#include <vsm/function.hpp>

#include <vsm/test/allocator.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

using function_type = function<int(int)>;

struct adder
{
	int x;

	explicit adder(int const x)
		: x(x)
	{
	}

	int operator()(int y) const
	{
		return x + y;
	}
};

struct alignas(256) big_adder : adder
{
	using adder::adder;
};

TEST_CASE("function", "[function]")
{
	test::allocation_scope const scope;
	{
		function<int(int)> f;
		size_t expected_allocation_count = 0;

		SECTION("static storage")
		{
			f = adder(7);
		}

		SECTION("dynamic storage")
		{
			f = decltype(f)(test::allocator(), big_adder(7));
			expected_allocation_count = 1;
		}

		REQUIRE(f(42) == 49);
		REQUIRE(scope.get_allocation_count() == expected_allocation_count);
	}
	REQUIRE(scope.get_allocation_count() == 0);
}

} // namespace
