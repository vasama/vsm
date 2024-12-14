
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
