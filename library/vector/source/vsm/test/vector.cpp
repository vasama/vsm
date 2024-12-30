#include <vsm/vector.hpp>

#include <vsm/testing/allocator.hpp>
#include <vsm/testing/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct trivial
{
	size_t value;

	trivial() = default;

	trivial(size_t value)
		: value(value)
	{
	}
};

struct non_trivial : test::counted
{
	size_t value;
	
	non_trivial()
		: value(0)
	{
	}

	non_trivial(size_t value)
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

	~non_trivial() // NOLINT(modernize-use-equals-default)
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
	test::scoped_count const instance_count;

	TestType vec;
	REQUIRE(vec.size() == 0);

	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector swap", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		auto const generate_vector = [](size_t const size) -> TestType
		{
			TestType vector;
			for (size_t i = 0; i < size; ++i)
			{
				vector.emplace_back(i);
			}
			return vector;
		};

		size_t const l_size = GENERATE(as<size_t>(), 0, 1, 11, 21);
		size_t const r_size = GENERATE(as<size_t>(), 0, 1, 11, 21);

		TestType l = generate_vector(l_size);
		TestType r = generate_vector(r_size);

		l.swap(r);

		REQUIRE(l.size() == r_size);
		REQUIRE(r.size() == l_size);

		for (size_t i = 0; i < l_size && i < r_size; ++i)
		{
			if (i < l.size())
			{
				REQUIRE(l[i].value == static_cast<size_t>(i));
			}

			if (i < r.size())
			{
				REQUIRE(r[i].value == static_cast<size_t>(i));
			}
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector push_back & emplace_back", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		bool const use_emplace = GENERATE(0, 1);

		for (size_t i = 0; i < 40; ++i)
		{
			size_t const v = i + 1;
	
			use_emplace
				? (void)vec.emplace_back(v)
				: (void)vec.push_back(v);

			REQUIRE(vec.size() == v);
		}

		for (size_t i = 0; i < 40; ++i)
	{
		REQUIRE(vec[i].value == i + 1);
	}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector insert & emplace", "[container][vector]", vector_types)
{
	TestType vsm_vec;
	std::vector<size_t> std_vec;

	bool const use_emplace = GENERATE(as<size_t>(), 0, 1);
	size_t const position = GENERATE(as<size_t>(), 0, 1, 2);

	for (size_t i = 0; i < 40; ++i)
	{
		size_t const v = i + 1;

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
			vsm_it = vsm_vec.begin() + static_cast<ptrdiff_t>(vsm_vec.size() / 2);
			std_it = std_vec.begin() + static_cast<ptrdiff_t>(std_vec.size() / 2);
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

	size_t const initial_size = GENERATE(as<size_t>(), 0, 10, 20, 30);

	for (size_t i = 0; i < initial_size; ++i)
	{
		vec.emplace_back(i + 1);
	}

	size_t const new_size = GENERATE(as<size_t>(), 0, 5, 15, 25, 35);

	vec.resize(new_size);
	REQUIRE(vec.size() == new_size);

	for (size_t i = 0; i < std::min(initial_size, new_size); ++i)
	{
		REQUIRE(vec[i].value == i + 1);
	}

	for (size_t i = initial_size; i < new_size; ++i)
	{
		REQUIRE(vec[i].value == 0);
	}
}
