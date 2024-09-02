#include <vsm/function_view.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

using view_type = function_view<int(int)>;

struct struct_type
{
	int x;

	//TODO: Convert this back to explicit this function. It's static right now only as a workaround
	//      for this Clang bug: https://github.com/llvm/llvm-project/issues/106660
	static int function(struct_type const& self, int const y)
	{
		return self.x + y;
	}
};

TEST_CASE("function_view borrows callable object", "[function_view]")
{
	int y = 7;

	auto const callable = [&](int const x) -> int
	{
		return x * y;
	};

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

TEST_CASE("function_view binds to explicit this function", "[function_view]")
{
	struct_type const object{ 1 };
	view_type const f(nontype<&struct_type::function>, object);
	REQUIRE(f(42) == 43);
}

} // namespace
