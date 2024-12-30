#include <vsm/swiss_map.hpp>

#include <catch2/catch_all.hpp>

#include <string_view>
#include <unordered_map>

using namespace vsm;

namespace {

void check_element(auto const& map, size_t const k, size_t const v)
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
	using type = small_swiss_map<K, V, Capacity>;
};

using map_types = std::tuple<
	map_template<default_allocator, 0>,
	map_template<default_allocator, 15>
>;

using std_map_type = std::unordered_map<size_t, size_t>;

TEMPLATE_LIST_TEST_CASE("swiss_map default constructor", "[hash_table][swiss_table][swiss_map]", map_types)
{
	using key_type = size_t;
	using map_type = typename TestType::template type<key_type, key_type>;

	map_type const map;
	CHECK(map.empty());
	CHECK(map.size() == 0);
}

TEST_CASE("swiss_map string_view", "[hash_table][swiss_table][swiss_map]")
{
	swiss_map<std::string_view, size_t> m;
	CHECK(m.insert("blah", 42u).inserted);
	CHECK(m.at("blah") == 42u);
}

TEMPLATE_LIST_TEST_CASE("swiss_map insert & find", "[hash_table][swiss_table][swiss_map]", map_types)
{
	using key_type = uint8_t;
	using map_type = typename TestType::template type<key_type, size_t>;

	size_t const count = GENERATE(as<size_t>(), 0, 10, 16, 20, 32);

	map_type vsm_map;
	std_map_type std_map;

	auto& rng = Catch::sharedRng();
	for (size_t i = 0; i < count; ++i)
	{
	retry_with_new_key:
		key_type const k = static_cast<key_type>(rng());

		if (!std_map.try_emplace(k, i).second)
		{
			goto retry_with_new_key;
		}

		CHECK(vsm_map.insert(k, i).inserted);
	}
	REQUIRE(vsm_map.size() == count);

	for (size_t i = 0, max = std::numeric_limits<key_type>::max(); i <= max; ++i)
	{
		key_type const k = static_cast<key_type>(i);
	
		auto std_iterator = std_map.find(k);
		bool const key_exists = std_iterator != std_map.end();
		auto const* const vsm_element = vsm_map.find_ptr(k);

		REQUIRE((vsm_element != nullptr) == key_exists);

		if (key_exists)
		{
			CHECK(vsm_element->key == k);
			CHECK(vsm_element->value == std_iterator->second);
		}
	}
}

} // namespace
