#include <vsm/hash_map.hpp>

#include <catch2/catch_all.hpp>

#include <string_view>

using namespace vsm;

namespace {

void check_element(auto const& map, int const k, int const v)
{
	auto const e = map.find(k);
	REQUIRE(e != nullptr);
	CHECK(e->key == k);
	CHECK(e->value == v);
}

template<typename Allocator, size_t Capacity>
struct map_template
{
	template<typename K, typename V>
	using type = small_hash_map<K, V, Capacity>;
};

using map_types = std::tuple<
	map_template<default_allocator, 0>,
	map_template<default_allocator, 15>
>;

using std_map_type = std::unordered_map<int, int>;

TEMPLATE_LIST_TEST_CASE("hash_map default constructor", "[hash_table][hash_map]", map_types)
{
	using key_type = int;
	using map_type = typename TestType::template type<key_type, key_type>;

	map_type map;
	CHECK(map.empty());
	CHECK(map.size() == 0);
}

TEMPLATE_LIST_TEST_CASE("hash_map insert & find", "[hash_table][hash_map]", map_types)
{
	using key_type = uint8_t;
	using map_type = typename TestType::template type<key_type, int>;

	int const count = GENERATE(0, 10, 16, 20, 32);

	map_type vsm_map;
	std_map_type std_map;

	auto& rng = Catch::sharedRng();
	for (int i = 0; i < count; ++i)
	{
	retry_with_new_key:
		key_type const k = static_cast<key_type>(rng());

		if (!std_map.try_emplace(k, static_cast<int>(i)).second)
		{
			goto retry_with_new_key;
		}

		CHECK(vsm_map.insert(k, i).inserted);
	}
	REQUIRE(vsm_map.size() == count);

	for (int i = 0, max = std::numeric_limits<key_type>::max(); i <= max; ++i)
	{
		key_type const k = static_cast<key_type>(i);
	
		auto std_iterator = std_map.find(k);
		bool const key_exists = std_iterator != std_map.end();
		auto const* const vsm_element = vsm_map.find(k);

		REQUIRE((vsm_element != nullptr) == key_exists);

		if (key_exists)
		{
			CHECK(vsm_element->key == k);
			CHECK(vsm_element->value == std_iterator->second);
		}
	}
}

TEST_CASE("hash_map string_view", "[hash_table][hash_map]")
{
	hash_map<std::string_view, int> m;
	CHECK(m.insert("blah", 42).inserted);
}

} // namespace
