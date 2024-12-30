#include <vsm/inplace_any.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct my_add
{
	using signature_type = int(int) const;

	template<typename T>
	static int invoke(T const& adder, int const value)
		requires requires { { adder.add(value) } -> std::convertible_to<int>; }
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

	[[nodiscard]] int add(int const operand) const
	{
		return operand + addend;
	}
};

struct my_adder_multiplier
{
	int addend;
	int multiplicand;

	[[nodiscard]] int add(int const operand) const
	{
		return operand + addend;
	}

	[[nodiscard]] int multiply(int const operand) const
	{
		return operand * multiplicand;
	}
};


TEST_CASE("inplace_any can be default-constructed", "[any][inplace_any]")
{
	inplace_any<16, my_add> any;
	REQUIRE(!any);
}

TEST_CASE("inplace_any can be constructed", "[any][inplace_any]")
{
	inplace_any<16, my_add> any(my_adder(42));
	REQUIRE(any);
	REQUIRE(any.invoke<my_add>(1) == 43);
}

TEST_CASE("inplace_any can be constructed explicitly", "[any][inplace_any]")
{
	inplace_any<16, my_add> any(std::in_place_type<my_adder const>, 42);
	REQUIRE(any);
	REQUIRE(any.invoke<my_add>(1) == 43);
}

TEST_CASE("inplace_any can be move-constructed", "[any][inplace_any]")
{
	inplace_any<16, my_add> any_1(my_adder(42));
	inplace_any<16, my_add> any_2(vsm_move(any_1));

	REQUIRE(!any_1);
	REQUIRE(any_2);
	REQUIRE(any_2.invoke<my_add>(1) == 43);
}

TEST_CASE("inplace_any can be move-constructed from a superset", "[any][inplace_any]")
{
	inplace_any<16, my_add, my_multiply> any_1(my_adder_multiplier(42));
	inplace_any<16, my_add> any_2(vsm_move(any_1));

	REQUIRE(!any_1);
	REQUIRE(any_2);
	REQUIRE(any_2.invoke<my_add>(1) == 43);
}

TEST_CASE("inplace_any can be move-constructed from a larger one", "[any][inplace_any]")
{
	inplace_any<16, my_add> any_1(my_adder(42));
	inplace_any<32, my_add> any_2(vsm_move(any_1));

	REQUIRE(!any_1);
	REQUIRE(any_2);
	REQUIRE(any_2.invoke<my_add>(1) == 43);
}

} // namespace
