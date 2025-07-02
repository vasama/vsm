#include <vsm/pointer_tag_pair.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct alignas(4) s {};

using pair_type = pointer_tag_pair<s, unsigned>;
static_assert(pair_type::bits_requested == 2);

static s array[2];

TEST_CASE("pointer_tag_pair", "[tag_ptr][pointer_tag_pair]")
{
	auto const [p, t] = pair_type(array, 3);
	REQUIRE(p == array);
	REQUIRE(t == 3);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_add_pointer", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(array + 0, 3));

	auto const pair_1 = atom.fetch_add_pointer(1, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == array + 0);
	REQUIRE(pair_1.tag() == 3);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == array + 1);
	REQUIRE(pair_2.tag() == 3);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_sub_pointer", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(array + 1, 3));

	auto const pair_1 = atom.fetch_sub_pointer(1, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == array + 1);
	REQUIRE(pair_1.tag() == 3);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == array + 0);
	REQUIRE(pair_2.tag() == 3);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_add_tag", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(nullptr, 3));

	auto const pair_1 = atom.fetch_add_tag(1, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == nullptr);
	REQUIRE(pair_1.tag() == 3);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == nullptr);
	REQUIRE(pair_2.tag() == 0);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_sub_tag", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(nullptr, 0));

	auto const pair_1 = atom.fetch_sub_tag(1, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == nullptr);
	REQUIRE(pair_1.tag() == 0);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == nullptr);
	REQUIRE(pair_2.tag() == 3);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_and_tag", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(nullptr, 3));

	auto const pair_1 = atom.fetch_and_tag(2, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == nullptr);
	REQUIRE(pair_1.tag() == 3);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == nullptr);
	REQUIRE(pair_2.tag() == 2);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_or_tag", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(nullptr, 1));

	auto const pair_1 = atom.fetch_or_tag(2, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == nullptr);
	REQUIRE(pair_1.tag() == 1);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == nullptr);
	REQUIRE(pair_2.tag() == 3);
}

TEST_CASE("atomic<pointer_tag_pair>::fetch_xor_tag", "[tag_ptr][pointer_tag_pair][atomic]")
{
	atomic<pair_type> atom(pair_type(nullptr, 3));

	auto const pair_1 = atom.fetch_xor_tag(2, std::memory_order_relaxed);
	REQUIRE(pair_1.pointer() == nullptr);
	REQUIRE(pair_1.tag() == 3);

	auto const pair_2 = atom.load(std::memory_order_relaxed);
	REQUIRE(pair_2.pointer() == nullptr);
	REQUIRE(pair_2.tag() == 1);
}

} // namespace
