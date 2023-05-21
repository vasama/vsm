#include <vsm/result.hpp>

#include <vsm/concepts.hpp>
#include <vsm/test/instance_counter.hpp>
#include <vsm/utility.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

template<typename T>
struct object : test::instance_counter<object<int>>
{
	T value;

	object(no_cvref_of<object> auto&& value)
		: value(vsm_forward(value))
	{
	}
};

TEST_CASE("vsm_try", "[result][try]")
{
	static constexpr int error_value = 666;

	using void_result_type = result<void, object<int>>;

	bool const has_value = GENERATE(1, 0);

	auto const make_result = [&]<typename T>(T&& value)
	{
		using result_type = result<object<std::decay_t<T>>, object<int>>;
	
		return has_value
			? result_type(vsm_forward(value))
			: result_type(std::unexpected(error_value));
	};

	auto const r = [&]() -> void_result_type
	{
		SECTION("vsm_try_result")
		{
			vsm_try_result(r, make_result(42));

			// The only live instance at this time should be the value held by `r`.
			CHECK(test::instance_count<object<int>>() == 1);

			CHECK(r->value == 42);
		}

		SECTION("vsm_try")
		{
			vsm_try(value, make_result(42));

			// The only live instance at this time should be `value`.
			CHECK(test::instance_count<object<int>>() == 1);

			CHECK(value.value == 42);
		}

		SECTION("vsm_try by reference")
		{
			auto r = make_result(42);

			vsm_try((auto&&, value), vsm_move(r));

			static_assert(std::is_same_v<decltype(value), object<int>&&>);

			// The only live instance at this time should be the value held by `r`.
			CHECK(test::instance_count<object<int>>() == 1);

			CHECK(value.value == 42);
			CHECK(&value == &r.value());
		}

		SECTION("vsm_try_void")
		{
			auto const make_void_result = [&]() -> void_result_type
			{
				return has_value
					? void_result_type{}
					: void_result_type(std::unexpected(error_value));
			};

			vsm_try_void(make_void_result());
		}

		SECTION("vsm_try_discard")
		{
			vsm_try_discard(make_result(42));

			// No instances should be live after discarding the result.
			CHECK(test::instance_count<object<int>>() == 0);
		}

		SECTION("vsm_try_bind")
		{
			using pair_type = std::pair<object<int>, object<int>>;
			using pair_result_type = result<pair_type, object<int>>;

			auto const make_pair_result = [&](int const first, int const second) -> pair_result_type
			{
				return has_value
					? pair_result_type(pair_type(first, second))
					: pair_result_type(std::unexpected(error_value));
			};

			vsm_try_bind((first, second), make_pair_result(42, 9001));

			// The only live instance at this time should be
			// the two to which `first` and `second` are bound.
			CHECK(test::instance_count<object<int>>() == 2);

			CHECK(first.value == 42);
			CHECK(second.value == 9001);
		}

		SECTION("vsm_try_assign")
		{
			object<int> value = 0;

			vsm_try_assign(value, make_result(42));

			// The only live instance at this time should be `value`.
			CHECK(test::instance_count<object<int>>() == 1);

			CHECK(value.value == 42);
		}

		return {};
	}();

	REQUIRE(r.has_value() == has_value);

	if (!has_value)
	{
		CHECK(r.error().value == error_value);
	}
}

} // namespace
