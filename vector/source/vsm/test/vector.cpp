#include <vsm/vector.hpp>

#include <vsm/test/allocator.hpp>
#include <vsm/test/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct trivial
{
	int value;

	trivial() = default;

	trivial(int value)
		: value(value)
	{
	}
};

struct non_trivial : test::counted
{
	int value;
	
	non_trivial()
		: value(0)
	{
	}

	non_trivial(int value)
		: value(value)
	{
	}

	non_trivial(non_trivial&& other) noexcept
		: value(other.value)
	{
		other.value = 0;
	}

	non_trivial& operator=(non_trivial&& other) noexcept
	{
		value = other.value;
		other.value = 0;
		return *this;
	}

	~non_trivial()
	{
	}
};

//TODO: Use object instance counting and a test allocator.

static auto values(auto const& range)
{
	return range | std::views::transform([](auto const& x) { return x.value; });
}

template<typename T, size_t Capacity>
using test_vector = small_vector<T, Capacity, test::allocator>;

using vector_types = std::tuple
<
	test_vector<trivial, 0>,
	test_vector<trivial, 20>,
	test_vector<non_trivial, 0>,
	test_vector<non_trivial, 20>
>;

} // namespace

TEMPLATE_LIST_TEST_CASE("vector default constructor", "[container][vector]", vector_types)
{
	test::scoped_count instance_count;

	TestType vec;
	REQUIRE(vec.size() == 0);

	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector swap", "[container][vector]", vector_types)
{
	test::scoped_count instance_count;
	{
		auto const generate_vector = [](size_t const size) -> TestType
		{
			TestType vector;
			for (size_t i = 0; i < size; ++i)
			{
				vector.emplace_back(static_cast<int>(i));
			}
			return vector;
		};

		size_t const l_size = GENERATE(0, 1, 11, 21);
		size_t const r_size = GENERATE(0, 1, 11, 21);

		TestType l = generate_vector(l_size);
		TestType r = generate_vector(r_size);

		l.swap(r);

		REQUIRE(l.size() == r_size);
		REQUIRE(r.size() == l_size);

		for (size_t i = 0; i < l_size && i < r_size; ++i)
		{
			if (i < l.size())
			{
				REQUIRE(l[i].value == static_cast<int>(i));
			}

			if (i < r.size())
			{
				REQUIRE(r[i].value == static_cast<int>(i));
			}
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector push_back & emplace_back", "[container][vector]", vector_types)
{
	test::scoped_count instance_count;
	{
		TestType vec;

		bool const use_emplace = GENERATE(0, 1);

		for (int i = 0; i < 40; ++i)
		{
			int const v = i + 1;
	
			use_emplace
				? (void)vec.emplace_back(v)
				: (void)vec.push_back(v);

			REQUIRE(vec.size() == v);
		}

		for (int i = 0; i < 40; ++i)
	{
		REQUIRE(vec[i].value == i + 1);
	}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector insert & emplace", "[container][vector]", vector_types)
{
	TestType vsm_vec;
	std::vector<int> std_vec;

	bool const use_emplace = GENERATE(0, 1);
	int const position = GENERATE(0, 1, 2);

	for (int i = 0; i < 40; ++i)
	{
		int const v = i + 1;

		decltype(vsm_vec.begin()) vsm_it;
		decltype(std_vec.begin()) std_it;

		switch (position)
		{
		default:
			REQUIRE(false);

		case 0:
			vsm_it = vsm_vec.begin();
			std_it = std_vec.begin();
			break;

		case 1:
			vsm_it = vsm_vec.begin() + vsm_vec.size() / 2;
			std_it = std_vec.begin() + std_vec.size() / 2;
			break;

		case 2:
			vsm_it = vsm_vec.end();
			std_it = std_vec.end();
			break;
		}

		use_emplace
			? vsm_vec.emplace(vsm_it, v)
			: vsm_vec.insert(vsm_it, v);

		std_vec.insert(std_it, v);

		REQUIRE(std::ranges::equal(values(vsm_vec), std_vec));
	}
}

TEMPLATE_LIST_TEST_CASE("vector resize", "[container][vector]", vector_types)
{
	TestType vec;

	int const initial_size = GENERATE(0, 10, 20, 30);

	for (int i = 0; i < initial_size; ++i)
	{
		vec.emplace_back(i + 1);
	}

	int const new_size = GENERATE(0, 5, 15, 25, 35);

	vec.resize(new_size);
	REQUIRE(vec.size() == new_size);

	for (int i = 0; i < std::min(initial_size, new_size); ++i)
	{
		REQUIRE(vec[i].value == i + 1);
	}

	for (int i = initial_size; i < new_size; ++i)
	{
		REQUIRE(vec[i].value == 0);
	}
}
