#include <vsm/shared_vector.hpp>

#include <vsm/testing/allocator.hpp>
#include <vsm/testing/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct object : test::counted
{
	int value;

	object(int const value)
		: value(value)
	{
	}

	friend bool operator==(object const& lhs, object const& rhs)
	{
		return lhs.value == rhs.value;
	}
};

static auto values(auto const& range)
{
	return range | std::views::transform([](auto const& x) { return x.value; });
}


using unique_type = unique_vector<object, test::allocator>;
using shared_type = shared_vector<object, test::allocator>;


static shared_type copy_or_borrow(std::span<object const> const span)
{
	return GENERATE(1, 0)
		? shared_type(std::from_range, span)
		: shared_type::borrow(span);
}

[[maybe_unused]]
static shared_type copy_or_share(shared_type const& vector)
{
	if (GENERATE(1, 0))
	{
		return shared_type(std::from_range, std::span<object const>(vector));
	}
	else
	{
		return vector;
	}
}


TEST_CASE("shared_vector construction", "[shared_vector]")
{
	object array[] = { 1, 2, 3 };

	test::scoped_count count;
	test::allocation_scope alloc;
	{
		shared_type const shared = copy_or_borrow(array);
		CHECK(std::ranges::equal(array, shared));
	}
	CHECK(count.empty());
}

TEST_CASE("shared_vector mutation", "[shared_vector]")
{
	object array[] = { 1, 2, 3 };

	test::scoped_count count;
	test::allocation_scope alloc;
	{
		shared_type shared = copy_or_borrow(array);
		shared_type const shared_copy = copy_or_share(shared);

		unique_type unique = vsm_move(shared);
		REQUIRE(std::ranges::equal(unique, shared_copy));

		unique.emplace_back(4);
		REQUIRE(std::ranges::equal(values(unique), std::array{ 1, 2, 3, 4 }));

		shared = vsm_move(unique);
		REQUIRE(std::ranges::equal(values(shared), std::array{ 1, 2, 3, 4 }));
	}
	CHECK(count.empty());
}

} // namespace
