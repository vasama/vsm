#include <vsm/function.hpp>

#include <catch2/catch_all.hpp>

#include <array>

using namespace vsm;

using function_type = function<int(int)>;

TEST_CASE("static storage")
{
	auto lambda = [](int const x) -> int
	{
		return x + 1;
	};

	REQUIRE(function<int(int)>(lambda)(42) == 43);
}

TEST_CASE("dynamic storage")
{
	auto lambda = [data = std::array{ 1, 2, 3, 4, 5, 6, 7, 8 }](int const x) -> int
	{
		int r = 0;
		for (int const y : data)
		{
			r += x * y;
		}
		return r;
	};

	REQUIRE(function<int(int)>(lambda)(2) == 72);
}
