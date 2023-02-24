#include <vsm/intrusive/wb_tree.hpp>

#include <vsm/intrusive/impl/elements.hpp>

#include <catch2/catch.hpp>

#include <set>

using namespace vsm;

namespace {

struct key_selector
{
	int operator()(element const& node) const
	{
		return node.value;
	}
};

using tree = wb_tree<element, key_selector>;

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
		return vsm::values(vsm_tree);
	}

	bool equal() const
	{
		return std::ranges::equal(std, values());
	}
};


TEST_CASE("wb_tree::insert", "[intrusive][wb_tree]")
{
	elements e;

	tree set;

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

		REQUIRE(set.Size() == 6);
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

		REQUIRE(set.Size() == 9);
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

		REQUIRE(set.Size() == 11);
	}
}

TEST_CASE("wb_tree::remove", "[intrusive][wb_tree]")
{
	two_trees sets;

	SECTION("One element tree")
	{
		sets.insert(0);
		REQUIRE(sets.remove(0));
		REQUIRE(sets.equal());
	}

	SECTION("Perfectly balanced, height 5")
	{
		auto const maybe_remove = [&](int const value) -> bool
		{
			if (GENERATE(false, true))
			{
				sets.remove(value);
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

		sets.insert(80);
		sets.insert(40);
		sets.insert(120);
		sets.insert(20);
		sets.insert(60);
		sets.insert(100);
		sets.insert(140);
		sets.insert(10);
		sets.insert(30);
		sets.insert(50);
		sets.insert(70);
		sets.insert(90);
		sets.insert(110);
		sets.insert(130);
		sets.insert(150);

		int const base = GENERATE(0, 80);

		SECTION("Remove a leaf")
		{
			REQUIRE(sets.remove(base + GENERATE(1, 3, 5, 7) * 10));
			REQUIRE(sets.equal());
		}

		SECTION("Remove an upper branch")
		{
			int const branch = base + GENERATE(2, 6) * 10;

			// Try with 0, 1 and 2 children removed.
			maybe_remove(branch - 10);
			maybe_remove(branch + 10);

			REQUIRE(sets.remove(branch));
			REQUIRE(sets.equal());
		}

		SECTION("Remove a lower branch")
		{
			auto const maybe_remove_upper_branch = [&](int const value) -> bool
			{
				if (maybe_remove(value - 10) && maybe_remove(value + 10) && GENERATE(false, true))
				{
					sets.remove(value);
					return true;
				}
				return false;
			};

			int const branch = base + 40;

			// Try with [0, 6] children removed.
			maybe_remove_upper_branch(branch - 20);
			maybe_remove_upper_branch(branch + 20);

			REQUIRE(sets.remove(branch));
			REQUIRE(sets.equal());
		}

		SECTION("Remove the root")
		{
			REQUIRE(sets.remove(80));
			REQUIRE(sets.equal());
		}
	}
}

TEST_CASE("wb_tree mass test.", "[intrusive][wb_tree]")
{
	elements e;

	auto rng = Catch::rng();
	std::uniform_int_distribution distribution = {};

	tree tree;
	std::set<int> std_tree;

	for (size_t i = 0; i < 10000; ++i)
	{
		int const value = distribution(rng);
		bool const inserted = std_tree.insert(value).second;
		REQUIRE(tree.insert(e(value)).inserted == inserted);
	}

	REQUIRE(std::ranges::equal(std_tree, values(tree)));
}

TEST_CASE("wb_tree::Clear", "[intrusive][wb_tree]")
{
	UniqueElements e;

	tree tree;

	// Insert the same elements twice.

	for (int i = 0; i < 10; ++i)
	{
		tree.insert(e(i));
	}
	tree.Clear();
	REQUIRE(tree.IsEmpty());

	for (int i = 0; i < 10; ++i)
	{
		tree.insert(e(i));
	}
	tree.Clear();
	REQUIRE(tree.IsEmpty());
}

TEST_CASE("wb_tree iteration.", "[intrusive][wb_tree]")
{
	elements e;

	tree tree;

	for (int i : std::views::iota(1, 100) | std::views::reverse)
	{
		tree.insert(e(i));
	}

	REQUIRE(std::ranges::equal(std::views::iota(1, 100), values(tree)));
}

} // namespace
