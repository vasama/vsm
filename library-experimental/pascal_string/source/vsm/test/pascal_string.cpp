#include <vsm/pascal_string.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

TEST_CASE("pascal_string literal", "[pascal_string]")
{
	using namespace string_literals;

	pascal_string const s = "hello"_pascal;

	size_t const size = s.size();
	REQUIRE(size == 5);

	char const* const data = s.data();
	CHECK(memcmp(data, "hello", 5) == 0);
}
