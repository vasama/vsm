#include <vsm/default_hash.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

template<typename T>
struct wrapper
{
	T x;

	template<typename State>
	friend void tag_invoke(decltype(hash_append), State& state, wrapper const& x)
	{
		hash_append(state, x.x);
	}
};

struct my_type
{
	std::string_view string;
};

struct my_hash_policy
{
	template<typename State>
	friend void tag_invoke(decltype(hash_append), State& state, my_type const& x)
	{
		hash_append(state, x.string);
	}
};

using my_hasher = basic_default_hasher<my_hash_policy>;

TEST_CASE("default_hash with custom policy", "[hash]")
{
	size_t const h1 = my_hasher()(wrapper<my_type>{"hello"});
	size_t const h2 = default_hasher()(std::string_view("hello"));
	REQUIRE(h1 == h2);
}

} // namespace
