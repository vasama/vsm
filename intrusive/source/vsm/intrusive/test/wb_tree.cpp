#include <vsm/intrusive/wb_tree.hpp>

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

using tree_type = wb_tree<element, key_selector>;

static_assert(std::bidirectional_iterator<tree_type::iterator>);
static_assert(std::bidirectional_iterator<tree_type::const_iterator>);
static_assert(std::ranges::bidirectional_range<tree_type>);


struct two_trees
{
	std::list<element> list;
	std::set<int> std_tree;
	wb_tree<element, key_selector> vsm_tree;

	auto insert(int const value)
	{
		auto const it = list.insert(list.end(), element(value));

		auto const vsm_result = vsm_tree.insert(&*it);
		auto const std_result = std_tree.insert(value);
		REQUIRE(vsm_result.inserted == std_result.second);

		if (!vsm_result.inserted)
		{
			list.erase(it);
		}

		return vsm_result;
	}

	bool remove(int const value)
	{
		element* const element = vsm_tree.find(value);
		auto const it = std_tree.find(value);

		REQUIRE((element != nullptr) == (it != std_tree.end()));
		if (element == nullptr)
		{
			return false;
		}

		vsm_tree.remove(element);
		std_tree.erase(it);

		return true;
	}

	auto values() const
	{
		return test::values(vsm_tree);
	}

	bool equal() const
	{
		return std::ranges::equal(std_tree, values());
	}
};


TEST_CASE("wb_tree::insert", "[intrusive][wb_tree]")
{
	elements e;

	tree_type set;

	SECTION("123")
	{
		REQUIRE(set.insert(e(1)).inserted);
		REQUIRE(set.insert(e(2)).inserted);
		REQUIRE(set.insert(e(3)).inserted);

		REQUIRE(!set.insert(e(1)).inserted);
		REQUIRE(!set.insert(e(2)).inserted);
		REQUIRE(!set.insert(e(3)).inserted);

		REQUIRE(set.insert(e(-1)).inserted);
		REQUIRE(set.insert(e(-2)).inserted);
		REQUIRE(set.insert(e(-3)).inserted);

		REQUIRE(!set.insert(e(-1)).inserted);
		REQUIRE(!set.insert(e(-2)).inserted);
		REQUIRE(!set.insert(e(-3)).inserted);

		REQUIRE(set.size() == 6);
	}

	SECTION("Pathological case 1")
	{
		REQUIRE(set.insert(e(9)).inserted);
		REQUIRE(set.insert(e(7)).inserted);
		REQUIRE(set.insert(e(5)).inserted);
		REQUIRE(set.insert(e(8)).inserted);
		REQUIRE(set.insert(e(6)).inserted);
		REQUIRE(set.insert(e(2)).inserted);
		REQUIRE(set.insert(e(4)).inserted);
		REQUIRE(set.insert(e(1)).inserted);
		REQUIRE(set.insert(e(3)).inserted);

		REQUIRE(set.size() == 9);
	}

	SECTION("Pathological case 2")
	{
		REQUIRE(set.insert(e(3)).inserted);
		REQUIRE(set.insert(e(2)).inserted);
		REQUIRE(set.insert(e(7)).inserted);
		REQUIRE(set.insert(e(1)).inserted);
		REQUIRE(set.insert(e(4)).inserted);
		REQUIRE(set.insert(e(9)).inserted);
		REQUIRE(set.insert(e(6)).inserted);
		REQUIRE(set.insert(e(8)).inserted);
		REQUIRE(set.insert(e(11)).inserted);
		REQUIRE(set.insert(e(10)).inserted);
		REQUIRE(set.insert(e(5)).inserted);

		REQUIRE(set.size() == 11);
	}
}

TEST_CASE("wb_tree::remove", "[intrusive][wb_tree]")
{
	two_trees trees;

	SECTION("One element tree")
	{
		trees.insert(0);
		REQUIRE(trees.remove(0));
	}

	SECTION("Perfectly balanced, height 5")
	{
		auto const maybe_remove = [&](int const value) -> bool
		{
			if (GENERATE(false, true))
			{
				trees.remove(value);
				return true;
			}
			return false;
		};

		/*
		 *                     8
		 *                    / \
		 *                   /   \
		 *                  /     \
		 *                 /       \
		 *                /         \
		 *               /           \
		 *              /             \
		 *             /               \
		 *            /                 \
		 *           4                  12
		 *          / \                 / \
		 *         /   \               /   \
		 *        /     \             /     \
		 *       /       \           /       \
		 *      2         6         10       14
		 *     / \       / \       / \       / \
		 *    /   \     /   \     /   \     /   \
		 *   1     3   5     7   9    11   13   15
		 */

		// Insert values in breadth-first order to produce the described tree:
		trees.insert(80);
		trees.insert(40);
		trees.insert(120);
		trees.insert(20);
		trees.insert(60);
		trees.insert(100);
		trees.insert(140);
		trees.insert(10);
		trees.insert(30);
		trees.insert(50);
		trees.insert(70);
		trees.insert(90);
		trees.insert(110);
		trees.insert(130);
		trees.insert(150);

		int const base = GENERATE(0, 80);

		SECTION("Remove a leaf")
		{
			REQUIRE(trees.remove(base + GENERATE(1, 3, 5, 7) * 10));
		}

		SECTION("Remove an upper branch")
		{
			int const branch = base + GENERATE(2, 6) * 10;

			// Try with 0, 1 and 2 children removed.
			maybe_remove(branch - 10);
			maybe_remove(branch + 10);

			REQUIRE(trees.remove(branch));
		}

		SECTION("Remove a lower branch")
		{
			auto const maybe_remove_upper_branch = [&](int const value) -> bool
			{
				bool const removed_l = maybe_remove(value - 10);
				bool const removed_r = maybe_remove(value + 10);

				return removed_l && removed_r && maybe_remove(value);
			};

			int const branch = base + 40;

			// Try with [0, 6] children removed.
			maybe_remove_upper_branch(branch - 20);
			maybe_remove_upper_branch(branch + 20);

			REQUIRE(trees.remove(branch));
		}

		SECTION("Remove the root")
		{
			REQUIRE(trees.remove(80));
		}
	}

	REQUIRE(trees.equal());
}

TEST_CASE("wb_tree mass test", "[intrusive][wb_tree]")
{
	elements e;

	auto&& rng = Catch::sharedRng();
	std::uniform_int_distribution distribution = {};

	tree_type tree;
	std::set<int> std_tree;

	for (size_t i = 0; i < 10000; ++i)
	{
		int const value = distribution(rng);
		bool const inserted = std_tree.insert(value).second;
		REQUIRE(tree.insert(e(value)).inserted == inserted);
	}

	REQUIRE(std::ranges::equal(std_tree, values(tree)));
}

TEST_CASE("wb_tree::clear", "[intrusive][wb_tree]")
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

TEST_CASE("wb_tree iteration.", "[intrusive][wb_tree]")
{
	elements e;

	tree_type tree;

	for (int i : std::views::iota(1, 100) | std::views::reverse)
	{
		tree.insert(e(i));
	}

	REQUIRE(std::ranges::equal(std::views::iota(1, 100), values(tree)));
}

} // namespace
