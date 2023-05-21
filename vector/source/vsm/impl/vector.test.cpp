#include <vsm/vector.hpp>

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

struct non_trivial
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
	
	non_trivial(non_trivial&& other)
		: value(other.value)
	{
		other.value = 0;
	}

	non_trivial& operator=(non_trivial&& other)
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

using vector_types = std::tuple
<
	small_vector<trivial, 0>,
	small_vector<trivial, 20>,
	small_vector<non_trivial, 0>,
	small_vector<non_trivial, 20>
>;

} // namespace

TEMPLATE_LIST_TEST_CASE("vector default constructor", "[container][vector]", vector_types)
{
	TestType vec;

	REQUIRE(vec.size() == 0);
}

TEMPLATE_LIST_TEST_CASE("vector push_back & emplace_back", "[container][vector]", vector_types)
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
		REQUIRE(v[i].value == i + 1);
	}
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
	}

	REQUIRE(vsm_vec.size() = 40);

	for (int i = 0; i < 40; ++i)
	{
		REQUIRE(vsm_vec[i].value == std_vec[i]);
	}
}

TEMPLATE_LIST_TEST_CASE("vector resize", "[container][vector]", vector_types)
{
	TestType vec;

	int const initial_size = GENERATE(0, 10, 20, 30);

	for (int i = 1; i <= initial_size; ++i)
	{
		vec.emplace_back(i);
	}

	int const new_size = GENERATE(0, 5, 15, 25, 35);

	vec.resize(new_size);
	REQUIRE(vec.size() == new_size);

	for (int i = 1; i <= std::min(initial_size, new_size); ++i)
	{
		REQUIRE(v[i].value == i);
	}

	for (int i = initial_size; i < new_size; ++i)
	{
		REQUIRE(v[i].value == 0);
	}
}
