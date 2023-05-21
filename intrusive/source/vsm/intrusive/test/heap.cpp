#include <vsm/intrusive/heap.hpp>

#include <vsm/intrusive/test/elements.hpp>

#include <catch2/catch_all.hpp>

#include <random>

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

struct two_heaps
{
	std::vector<int> std_heap;
	max_heap<element, key_selector> vsm_heap;
	elements e;

	bool empty() const
	{
		bool const empty = vsm_heap.empty();
		REQUIRE(empty == std_heap.empty());
		return empty;
	}

	void push(int const value)
	{
		vsm_heap.push(e(value));

		std_heap.push_back(value);
		std::ranges::push_heap(std_heap);
	}

	int pop()
	{
		int const vsm_value = vsm_heap.pop()->value;

		std::ranges::pop_heap(std_heap);
		int const std_value = std_heap.back();
		std_heap.pop_back();

		REQUIRE(vsm_value == std_value);

		return vsm_value;
	}

	bool remove(int const value)
	{
		auto const std_it = std::ranges::find(std_heap, value);
		auto const vsm_it = std::find_if(
			e.list.begin(), e.list.end(),
			[&](auto const& x) { return x.value == value; });

		bool const std_found = std_it != std_heap.end();
		bool const vsm_found = vsm_it != e.list.end();

		REQUIRE(std_found == vsm_found);

		if (vsm_found)
		{
			std_heap.erase(std_it);
			std::ranges::make_heap(std_heap);
			vsm_heap.remove(&*vsm_it);
		}

		return vsm_found;
	}
};

TEST_CASE("heap::push", "[intrusive][heap]")
{
	two_heaps heaps;

	heaps.push(2);
	heaps.push(1);
	heaps.push(3);

	CHECK(heaps.pop() == 3);
	CHECK(heaps.pop() == 2);
	CHECK(heaps.pop() == 1);

	CHECK(heaps.empty());
}

TEST_CASE("heap::remove", "[intrusive][heap]")
{
	two_heaps heaps;

	for (int i = 7; i > 0; --i)
	{
		heaps.push(i);
	}

	int const remove = GENERATE(1, 4, 7);
	CHECK(heaps.remove(remove));

	for (int i = 7; i > 0; --i)
	{
		if (i != remove)
		{
			CHECK(i == heaps.pop());
		}
	}

	CHECK(heaps.empty());
}

TEST_CASE("heap mass test", "[intrusive][heap]")
{
	std::vector<int> array;
	max_heap<element, key_selector> heap;
	elements e;

	auto&& rng = Catch::sharedRng();
	auto distribution = std::uniform_int_distribution();

	for (size_t i = 0; i < 10000; ++i)
	{
		int const value = distribution(rng);

		array.push_back(value);
		heap.push(e(value));
	}

	std::ranges::sort(array, std::greater{});

	for (int const value : array)
	{
		REQUIRE(!heap.empty());
		CHECK(value == heap.pop()->value);
	}

	CHECK(heap.empty());
}

} // namespace
