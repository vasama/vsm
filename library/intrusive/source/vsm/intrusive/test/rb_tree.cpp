#include <vsm/intrusive/rb_tree.hpp>

#include <vsm/intrusive/test/elements.hpp>

#include <catch2/catch_all.hpp>

#include <set>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::test;

namespace {

struct key_selector
{
	int operator()(element const& node) const
	{
		return node.value;
	}
};

using tree_type = rb_tree<element, key_selector>;

static_assert(std::bidirectional_iterator<tree_type::iterator>);
static_assert(std::bidirectional_iterator<tree_type::const_iterator>);
static_assert(std::ranges::bidirectional_range<tree_type>);

struct two_trees
{
	using vsm_tree_type = rb_tree<element, key_selector>;
	using const_iterator = vsm_tree_type::const_iterator;

	std::list<element> list;
	std::set<int> std_tree;
	vsm_tree_type vsm_tree;

	auto insert(int const value)
	{
		auto const it = list.insert(list.end(), element(value));

		auto const vsm_result = vsm_tree.insert(*it);
		auto const std_result = std_tree.insert(value);
		REQUIRE(vsm_result.inserted == std_result.second);

		if (!vsm_result.inserted)
		{
			list.erase(it);
		}

		return vsm_result;
	}

	bool erase(int const value)
	{
		auto const vsm_it = vsm_tree.find(value);
		auto const std_it = std_tree.find(value);

		REQUIRE((vsm_it != vsm_tree.end()) == (std_it != std_tree.end()));

		if (vsm_it == vsm_tree.end())
		{
			return false;
		}

		vsm_tree.erase(vsm_it);
		std_tree.erase(std_it);

		return true;
	}

	void erase(const_iterator const pos)
	{
		std_tree.erase(pos->value);
		vsm_tree.erase(pos);
	}

	[[nodiscard]] auto values() const
	{
		return test::values(vsm_tree);
	}

	[[nodiscard]] bool equal() const
	{
		return std::ranges::equal(std_tree, values());
	}
};


TEST_CASE("rb_tree::insert", "[intrusive][rb_tree]")
{
	elements e;

	tree_type tree;

	REQUIRE(tree.insert(e(1)).inserted);
	REQUIRE(tree.insert(e(2)).inserted);
	REQUIRE(tree.insert(e(3)).inserted);

	REQUIRE(!tree.insert(e(1)).inserted);
	REQUIRE(!tree.insert(e(2)).inserted);
	REQUIRE(!tree.insert(e(3)).inserted);

	REQUIRE(tree.insert(e(-1)).inserted);
	REQUIRE(tree.insert(e(-2)).inserted);
	REQUIRE(tree.insert(e(-3)).inserted);

	REQUIRE(!tree.insert(e(-1)).inserted);
	REQUIRE(!tree.insert(e(-2)).inserted);
	REQUIRE(!tree.insert(e(-3)).inserted);

	REQUIRE(tree.size() == 6);
}

TEST_CASE("rb_tree::erase", "[intrusive][rb_tree]")
{
	// NOLINTBEGIN(readability-braces-around-statements)

	two_trees trees;

	SECTION("One element tree")
	{
		trees.insert(0);
		REQUIRE(trees.erase(0));
		REQUIRE(trees.equal());
	}

	SECTION("Perfectly balanced, height 3-4")
	{
		/*           4
		 *          / \
		 *         /   \
		 *        /     \
		 *       /       \
		 *      2         6
		 *     / \       / \
		 *    /   \     /   \
		 *   1     3   5     7
		 */

		trees.insert(40);
		trees.insert(20);
		trees.insert(60);
		trees.insert(10);
		trees.insert(30);
		trees.insert(50);
		trees.insert(70);

		SECTION("Remove a leaf")
		{
			REQUIRE(trees.erase(GENERATE(1, 3, 5, 7) * 10));
			REQUIRE(trees.equal());
		}

		SECTION("Remove a branch")
		{
			int const branch = GENERATE(2, 6) * 10;

			// Try with 0, 1 and 2 children removed.
			if (GENERATE(false, true)) trees.erase(branch - 10);
			if (GENERATE(false, true)) trees.erase(branch + 10);

			REQUIRE(trees.erase(branch));
			REQUIRE(trees.equal());
		}

		SECTION("Remove the root")
		{
			int branch1 = 20;
			int branch2 = 60;

			if (GENERATE(false, true)) std::swap(branch1, branch2);

			if (GENERATE(false, true))
			{
				// Try with 0, 1, and 2 children added to the leaves of the branch.
				if (GENERATE(false, true)) trees.insert(branch1 - 15);
				if (GENERATE(false, true)) trees.insert(branch1 - 5);
				if (GENERATE(false, true)) trees.insert(branch1 + 5);
				if (GENERATE(false, true)) trees.insert(branch1 + 15);
			}
			else
			{
				// Try with 0, 1 and 2 children removed from branch1.
				if (GENERATE(false, true)) trees.erase(branch1 - 10);
				if (GENERATE(false, true)) trees.erase(branch1 + 10);
			}

			// Try with one of the children removed from branch2.
			if (GENERATE(false, true)) trees.erase(branch2 + GENERATE(-1, +1) * 10);

			REQUIRE(trees.erase(40));
			REQUIRE(trees.equal());
		}
	}

	// NOLINTEND(readability-braces-around-statements)
}

TEST_CASE("rb_tree::clear", "[intrusive][rb_tree]")
{
	unique_elements e;

	tree_type tree;

	// Insert the same elements twice.

	for (int i = 0; i < 10; ++i)
	{
		tree.insert(e(i));
	}
	tree.clear();
	REQUIRE(tree.empty());

	for (int i = 0; i < 10; ++i)
	{
		tree.insert(e(i));
	}
	tree.clear();
	REQUIRE(tree.empty());
}

TEST_CASE("rb_tree iteration", "[intrusive][rb_tree]")
{
	elements e;

	tree_type tree;

	for (int const i : std::views::iota(1, 100) | std::views::reverse)
	{
		tree.insert(e(i));
	}

	REQUIRE(std::ranges::equal(std::views::iota(1, 100), values(tree)));
}

TEST_CASE("rb_tree mass test", "[intrusive][rb_tree]")
{
	static constexpr size_t count = 10'000;

	auto&& rng = Catch::sharedRng();
	Catch::uniform_integer_distribution distribution(
		0,
		std::numeric_limits<int>::max());

	two_trees trees;

	std::vector<two_trees::const_iterator> iterators;
	iterators.reserve(count);

	for (size_t i = 0; i < count; ++i)
	{
		auto const [iterator, inserted] = trees.insert(distribution(rng));

		if (inserted)
		{
			iterators.emplace_back(iterator);
		}
	}

	REQUIRE(trees.equal());

	std::ranges::shuffle(iterators, rng);

	for (two_trees::const_iterator const iterator : iterators)
	{
		trees.erase(iterator);
		REQUIRE(trees.equal());
	}
}

} // namespace
