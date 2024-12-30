#include <vsm/xxhash.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct pair
{
	int i;
	std::string_view s;

	template<typename State>
	friend void tag_invoke(decltype(hash_append), State& state, pair const& p)
	{
		hash_append(state, p.i);
		hash_append(state, p.s);
	}
};

static size_t hash_manually(pair const& p)
{
	vsm_detail_xxhash(state_t) state;
	vsm_detail_xxhash(reset)(&state, get_aslr_seed());

	vsm_detail_xxhash(update)(&state, &p.i, sizeof(p.i));
	vsm_detail_xxhash(update)(&state, p.s.data(), p.s.size());

	return vsm_detail_xxhash(digest)(&state);
}

TEST_CASE("xxhash", "[hash]")
{
	pair const p = { 42, "hello" };
	size_t const h1 = basic_hasher<xxhash>()(p);
	size_t const h2 = hash_manually(p);
	REQUIRE(h1 == h2);
}

} // namespace
