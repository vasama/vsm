#include <vsm/function_view.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

using view_type = function_view<int(int)>;

TEST_CASE("function_view borrows callable object", "[function_view]")
{
	int y = 7;

	auto const callable = [&](int const x) -> int
	{
		return x * y;
	};

	detail::function_view_impl<0, 0, int, int> ff = callable;

	view_type const f = callable;

	if (GENERATE(0, 1))
	{
		y = -y;
	}

	CHECK(f(42) == 42 * y);
}

TEST_CASE("function_view binds to function", "[function_view]")
{
	int(* const function)(int) = [](int const x) -> int
	{
		return x * 7;
	};
	
	SECTION("reference to function")
	{
		CHECK(view_type(*function)(42) == 42 * 7);
	}
	
	SECTION("pointer to function")
	{
		CHECK(view_type(function)(42) == 42 * 7);
	}
}
