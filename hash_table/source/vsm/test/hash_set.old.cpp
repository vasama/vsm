#include <vsm/hash_set.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

TEST_CASE("hash_table bug 2", "[hash_table]")
{
	vsm::hash_set<size_t, vsm::default_key_selector, std::identity> set;

	for (size_t i = 0; i < 16; ++i)
	{
		size_t const k = i << 7;
		REQUIRE(set.insert(k).inserted);
		REQUIRE(set.remove(k));
	}
}
