//#include <vsm/any.hpp>
#include <vsm/any_ref.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct my_add
{
	using signature_type = int(int) const;

	template<typename T>
	static int invoke(T const& adder, int const value)
	{
		return adder.add(value);
	}
};

struct my_multiply
{
	using signature_type = int(int) const;

	template<typename T>
	static int invoke(T const& multiplier, int const value)
	{
		return multiplier.multiply(value);
	}
};

struct my_adder
{
	int addend;

	int add(int const operand) const
	{
		return operand + addend;
	}
};

struct my_adder_multiplier
{
	int addend;
	int multiplicand;

	int add(int const operand) const
	{
		return operand + addend;
	}

	int multiply(int const operand) const
	{
		return operand * multiplicand;
	}
};

#if 0
TEST_CASE("any", "[any]")
{
	using any_adder = any<my_add>;
	{
		my_adder adder{ 42 };
		any_adder any(adder);
		std::same_as<int> auto r = any.invoke<my_add>(7);
		REQUIRE(r == 7 + 42);
	}

	using any_adder_multiplier = any<my_add, my_multiply>;
	{
		my_adder_multiplier adder_multiplier{ 17, -3 };

		any_adder_multiplier any1(adder_multiplier);
		std::same_as<int> auto r1 = any1.invoke<my_multiply>(4);
		REQUIRE(r1 == 4 * -3);

		any_adder any2 = any1;
		std::same_as<int> auto r2 = any2.invoke<my_add>(5);
		REQUIRE(r2 == 5 + 17);
	}
}
#endif

TEST_CASE("any_ref", "[any][any_ref]")
{
	using any_adder_ref = any_ref<my_add>;
	{
		my_adder adder{ 42 };
		any_adder_ref ref(adder);
		std::same_as<int> auto r = ref.invoke<my_add>(7);
		REQUIRE(r == 7 + 42);
	}

	using any_adder_multiplier_ref = any_ref<my_add, my_multiply>;
	{
		my_adder_multiplier adder_multiplier{ 17, -3 };

		any_adder_multiplier_ref ref1(adder_multiplier);
		std::same_as<int> auto r1 = ref1.invoke<my_multiply>(4);
		REQUIRE(r1 == 4 * -3);

		any_adder_ref ref2 = ref1;
		std::same_as<int> auto r2 = ref2.invoke<my_add>(5);
		REQUIRE(r2 == 5 + 17);
	}
}

} // namespace
