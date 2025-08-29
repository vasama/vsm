#include <vsm/vector.hpp>

#include <vsm/testing/instance_counter.hpp>

#include <catch2/catch_all.hpp>

#include <ranges>

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

template<typename Range>
static auto values(Range const& range)
{
	using type = std::ranges::range_value_t<Range>;
	return range | std::views::transform(&type::value);
}

template<typename T, size_t Capacity>
using test_vector = small_vector<T, Capacity>;

using vector_types = std::tuple
<
	test_vector<trivial, 0>,
	test_vector<trivial, 20>,
	test_vector<non_trivial, 0>,
	test_vector<non_trivial, 20>
>;

} // namespace

TEMPLATE_LIST_TEST_CASE("vector can be default constructed", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;

	TestType vec;
	REQUIRE(vec.size() == 0);

	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be grown by emplacing back single elements",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		for (size_t i = 0; i < 100; ++i)
		{
			vec.emplace_back(i);
			REQUIRE(vec.size() == i + 1);
			REQUIRE(vec[i].value == i);
		}

		for (size_t i = 1; i < 100; ++i)
		{
			REQUIRE(vec[i].value == i);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be grown by emplacing single elements at the back",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		for (size_t i = 0; i < 100; ++i)
		{
			vec.emplace(vec.end(), i);
			REQUIRE(vec.size() == i + 1);
			REQUIRE(vec.back().value == i);
		}

		for (size_t i = 0; i < 100; ++i)
		{
			REQUIRE(vec[i].value == i);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be grown by emplacing single elements in the middle",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;
		std::vector<size_t> std_vec;

		for (size_t i = 0; i < 100; ++i)
		{
			size_t const j = vec.size() / 2;

			vec.emplace(vec.begin() + j, i);
			std_vec.insert(std_vec.begin() + static_cast<ptrdiff_t>(j), i);

			REQUIRE(vec.size() == i + 1);
			REQUIRE(vec[j].value == i);
		}

		REQUIRE(std::ranges::equal(values(vec), std_vec));
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be grown by emplacing single elements at the front",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		for (size_t i = 0; i < 100; ++i)
		{
			vec.emplace(vec.begin(), i);
			REQUIRE(vec.size() == i + 1);
			REQUIRE(vec.front().value == i);
		}

		for (size_t i = 0; i < 100; ++i)
		{
			REQUIRE(vec[i].value == 100 - i - 1);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector can be grown by resizing", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		vec.resize(1);
		REQUIRE(vec.size() == 1);
		REQUIRE(vec.front().value == 0);

		vec.front().value = 42;

		vec.resize(100);
		REQUIRE(vec.size() == 100);
		REQUIRE(vec.back().value == 0);

		REQUIRE(vec.front().value == 42);
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector can be shrunk by resizing", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		vec.resize(100);
		vec.front().value = 42;

		vec.resize(1);
		REQUIRE(vec.size() == 1);
		REQUIRE(vec.front().value == 42);
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector resizing can be a no-op", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		vec.resize(100);
		for (size_t i = 0; i < 100; ++i)
		{
			vec[i].value = i;
		}

		vec.resize(100);
		REQUIRE(vec.size() == 100);

		for (size_t i = 0; i < 100; ++i)
		{
			REQUIRE(vec[i].value == i);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be shrunk by popping back single elements",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		bool const use_pop_back_value = GENERATE(0, 1);

		TestType vec;

		vec.resize(100);
		for (size_t i = 0; i < 100; ++i)
		{
			vec[i].value = i;
		}

		for (size_t i = 0; i < 100; ++i)
		{
			REQUIRE(vec.back().value == 100 - i - 1);

			if (use_pop_back_value)
			{
				vec.pop_back();
			}
			else
			{
				REQUIRE(vec.pop_back_value().value == 100 - i - 1);
			}

			REQUIRE(vec.size() == 100 - i - 1);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be shrunk by erasing single elements at the front",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		vec.resize(100);
		for (size_t i = 0; i < 100; ++i)
		{
			vec[i].value = i;
		}

		for (size_t i = 0; i < 100; ++i)
		{
			REQUIRE(vec.front().value == i);
			vec.erase(vec.begin());
			REQUIRE(vec.size() == 100 - i - 1);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be shrunk by erasing single elements in the middle",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;
		std::vector<size_t> std_vec;

		vec.resize(100);
		std_vec.resize(100);

		for (size_t i = 0; i < 100; ++i)
		{
			vec[i].value = i;
			std_vec[i] = i;
		}

		for (size_t i = 0; i < 100; ++i)
		{
			size_t const j = vec.size() / 2;

			REQUIRE(vec[j].value == std_vec[j]);
			vec.erase(vec.begin() + j);
			std_vec.erase(std_vec.begin() + static_cast<ptrdiff_t>(j));
			REQUIRE(vec.size() == 100 - i - 1);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE(
	"vector can be shrunk by erasing single elements at the back",
	"[container][vector]",
	vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		vec.resize(100);
		for (size_t i = 0; i < 100; ++i)
		{
			vec[i].value = i;
		}

		for (size_t i = 0; i < 100; ++i)
		{
			REQUIRE(vec.back().value == 100 - i - 1);
			vec.erase(vec.end() - 1);
			REQUIRE(vec.size() == 100 - i - 1);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector can be cleared", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;

	TestType vec;
	vec.resize(GENERATE(as<size_t>(), 0, 1, 100));
	vec.clear();

	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector can be shrunk to fit", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec;

		size_t const default_capacity = vec.capacity();

		vec.resize(1000);
		for (size_t i = 0; i < 100; ++i)
		{
			vec[i].value = i;
		}

		size_t const new_size = GENERATE(as<size_t>(), 0, 1, 100);

		vec.resize(new_size);
		vec.shrink_to_fit();

		REQUIRE(vec.size() == new_size);
		for (size_t i = 0; i < vec.size(); ++i)
		{
			REQUIRE(vec[i].value == i);
		}

		REQUIRE(vec.capacity() == std::max(new_size, default_capacity));
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector can be move constructed", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec_1;

		size_t const size = GENERATE(as<size_t>(), 0, 1, 100);

		vec_1.resize(size);
		for (size_t i = 0; i < size; ++i)
		{
			vec_1[i].value = i;
		}

		TestType vec_2 = vsm_move(vec_1);

		REQUIRE(vec_1.size() == 0);
		REQUIRE(vec_2.size() == size);

		for (size_t i = 0; i < size; ++i)
		{
			REQUIRE(vec_2[i].value == i);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("vector can be move assigned", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec_1;
		TestType vec_2;

		size_t const size_1 = GENERATE(as<size_t>(), 0, 1, 100);
		size_t const size_2 = GENERATE(as<size_t>(), 0, 1, 100);

		vec_1.resize(size_1);
		for (size_t i = 0; i < size_1; ++i)
		{
			vec_1[i].value = i;
		}

		vec_2.resize(size_2);
		for (size_t i = 0; i < size_2; ++i)
		{
			vec_2[i].value = 100 + i;
		}

		vec_2 = vsm_move(vec_1);

		REQUIRE(vec_1.size() == 0);
		REQUIRE(vec_2.size() == size_1);

		for (size_t i = 0; i < size_1; ++i)
		{
			REQUIRE(vec_2[i].value == i);
		}
	}
	REQUIRE(instance_count.empty());
}

TEMPLATE_LIST_TEST_CASE("two vectors can be swapped", "[container][vector]", vector_types)
{
	test::scoped_count const instance_count;
	{
		TestType vec_1;
		TestType vec_2;

		size_t const size_1 = GENERATE(as<size_t>(), 0, 1, 100);
		size_t const size_2 = GENERATE(as<size_t>(), 0, 1, 100);

		vec_1.resize(size_1);
		for (size_t i = 0; i < size_1; ++i)
		{
			vec_1[i].value = i;
		}

		vec_2.resize(size_2);
		for (size_t i = 0; i < size_2; ++i)
		{
			vec_2[i].value = 100 + i;
		}

		vec_1.swap(vec_2);

		REQUIRE(vec_1.size() == size_2);
		REQUIRE(vec_2.size() == size_1);

		for (size_t i = 0; i < size_1; ++i)
		{
			REQUIRE(vec_2[i].value == i);
		}

		for (size_t i = 0; i < size_2; ++i)
		{
			REQUIRE(vec_1[i].value == 100 + i);
		}
	}
	REQUIRE(instance_count.empty());
}
