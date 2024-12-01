#include <vsm/bit_ptr.hpp>

#include <catch2/catch_all.hpp>

#include <random>

using namespace vsm;

TEST_CASE("bit_ptr unsigned arithmetic", "[bit][bit_ptr]")
{
	uint32_t word = 0;

	auto const p = bit_ptr<uint32_t>(&word, 0);
	auto const p_32 = p + 32u;

	CHECK(p_32 - p == 32);
	CHECK(p_32 - 32u == p);

	CHECK(13u + p + 19u == p_32);
	CHECK(p_32 - 13u - 19u == p);
}

TEST_CASE("bit_ptr signed arithmetic", "[bit][bit_ptr]")
{
	uint32_t word = 0;

	auto const p = bit_ptr<uint32_t>(&word, 0);
	auto const p_32 = p + 32;

	CHECK(p_32 - p == 32);
	CHECK(p_32 - 32 == p);

	CHECK(13 + p + 19 == p_32);
	CHECK(p_32 - 13 - 19 == p);
}

TEST_CASE("bit_ref assignment", "[bit][bit_ref]")
{
	uint32_t word = 0;

	bit_ref<uint32_t>(word, 0) = true;
	CHECK(word == 1u);
	word = 0;

	bit_ref<uint32_t>(word, 31) = true;
	CHECK(word == 0x80000000u);
	word = 0;

	bit_ref<uint32_t>(word, 0) = true;
	bit_ref<uint32_t>(word, 1) = true;
	CHECK(word == 3u);
	word = 0;

	{
		bit_ref<uint32_t> const ref(word, 0);
		ref = true;
		CHECK(word == 1u);
	}
	word = 0;
}

TEST_CASE("bit_ptr iteration", "[bit][bit_ptr]")
{
	uint32_t word = 0;

	Catch::uniform_integer_distribution<uint32_t> distribution(
		0,
		std::numeric_limits<uint32_t>::max());

	uint32_t const random_word = distribution(Catch::sharedRng());

	auto const beg = bit_ptr(&word, 0);
	auto const end = bit_ptr(&word + 1, 0);
	REQUIRE(end - beg == 32);

	bit_ptrdiff_t i = 0;
	for (auto pos = beg; pos != end; ++pos, ++i)
	{
		REQUIRE(i < 32);
		REQUIRE(i == pos - beg);

		if (random_word & 1u << i)
		{
			*pos = true;
		}
	}
	CHECK(word == random_word);
}
