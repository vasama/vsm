#include <vsm/swiss_set.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

TEST_CASE("swiss_table full tombs", "[hash_table][swiss_table]")
{
	vsm::swiss_set<
		size_t,
		default_key_selector,
		default_allocator,
		/* Hasher: */ std::identity> set;

	for (size_t i = 0; i < 16; ++i)
	{
		size_t const k = i << 7;
		REQUIRE(set.insert(k).inserted);
		REQUIRE(set.erase(k));
	}
}
