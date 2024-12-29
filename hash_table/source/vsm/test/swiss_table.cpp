#include <vsm/detail/swiss_table.hpp>

#include <catch2/catch_all.hpp>

#include <random>
#include <ranges>

using namespace vsm;
using namespace detail::swiss_table;

namespace {

using group_types = std::tuple<
	group_generic<size_t>

#if vsm_arch_x86
	, group_x86
#endif
>;

TEMPLATE_LIST_TEST_CASE("swiss table group hash matching", "[hash_table][swiss_table]", group_types)
{
	using group = TestType;

	ctrl const fill = GENERATE(ctrl::ctrl_empty, ctrl::ctrl_tomb, ctrl::ctrl_end);

	for (size_t i = 0; i <= 0x7f; ++i)
	{
		std::array<ctrl, group::size> ctrls;
		ctrls.fill(fill);

		ctrls[0] = static_cast<ctrl>(i);
		ctrls[group::size - 1] = static_cast<ctrl>(i);

		auto const match = group(ctrls.data()).match(static_cast<ctrl>(i));
		size_t const indices[] = { 0, group::size - 1 };

		[[maybe_unused]] std::vector<size_t> vv(std::from_range, match);

		if (!std::ranges::equal(match, indices))
		{
			[[maybe_unused]] int x = 0;
		}

		REQUIRE(std::ranges::equal(match, indices));
	}
}

TEMPLATE_LIST_TEST_CASE("swiss_table group matching", "[hash_table][swiss_table]", group_types)
{
	using group = TestType;

	auto const get_bools = [](auto const& iterator)
	{
		std::array<bool, group::size> bools = {};
		for (size_t const index : iterator)
		{
			bools[index] = true;
		}
		return bools;
	};

	std::array<ctrl, group::size> ctrls = {};
	std::array<bool, group::size> bools = {};

	ctrls.fill(GENERATE(static_cast<ctrl>(0), ctrl::ctrl_end));

	auto const set_values = [&](std::same_as<ctrl> auto const... values)
	{
		if (GENERATE(0, 1))
		{
			ctrls[0] = GENERATE_COPY(values...);
			bools[0] = true;
		}

		if (GENERATE(0, 1))
		{
			ctrls[1] = GENERATE_COPY(values...);
			bools[1] = true;
		}

		if (GENERATE(0, 1))
		{
			ctrl const value = GENERATE_COPY(values...);

			for (size_t i = 2; i < group::size - 2; ++i)
			{
				ctrls[i] = value;
				bools[i] = true;
			}
		}

		if (GENERATE(0, 1))
		{
			ctrls[group::size - 2] = GENERATE_COPY(values...);
			bools[group::size - 2] = true;
		}

		if (GENERATE(0, 1))
		{
			ctrls[group::size - 1] = GENERATE_COPY(values...);
			bools[group::size - 1] = true;
		}
	};

	std::array<bool, group::size> match = {};

	SECTION("h2(x)")
	{
		if (ctrls.front() == ctrl::ctrl_end && GENERATE(0, 1))
		{
			ctrls.fill(GENERATE(ctrl::ctrl_empty, ctrl::ctrl_tomb));
		}

		ctrl const value = static_cast<ctrl>(GENERATE(1, 0x7f));

		set_values(value);
		match = get_bools(group(ctrls.data()).match(value));
	}

	SECTION("empty")
	{
		if (ctrls.front() == ctrl::ctrl_end && GENERATE(0, 1))
		{
			ctrls.fill(GENERATE(ctrl::ctrl_tomb));
		}

		set_values(ctrl::ctrl_empty);
		match = get_bools(group(ctrls.data()).match_empty());
	}

	SECTION("free")
	{
		set_values(ctrl::ctrl_empty, ctrl::ctrl_tomb);
		match = get_bools(group(ctrls.data()).match_free());
	}

	REQUIRE(match == bools);
}

TEMPLATE_LIST_TEST_CASE("swiss_table group iteration", "[hash_table][swiss_table]", group_types)
{
	using group = TestType;

	std::array<ctrl, group::size> ctrls = {};
	ctrls.fill(GENERATE(ctrl::ctrl_empty, ctrl::ctrl_tomb));

	size_t const index = GENERATE(range(static_cast<size_t>(0), group::size - 1));
	ctrls[index] = GENERATE(ctrl::ctrl_end, static_cast<ctrl>(0), static_cast<ctrl>(0x7f));
	size_t const match = group(ctrls.data()).count_leading_free_or_end();

	REQUIRE(match == index);
}

TEMPLATE_LIST_TEST_CASE("swiss_table group convert", "[hash_table][swiss_table][ddddd]", group_types)
{
	using group = TestType;

	std::array<ctrl, group::size> ctrls_1 = {};

	ctrls_1[0] = ctrl::ctrl_empty;
	ctrls_1[1] = ctrl::ctrl_tomb;
	ctrls_1[2] = ctrl::ctrl_end;

	std::ranges::shuffle(ctrls_1, Catch::sharedRng());
	std::array<ctrl, group::size> ctrls_2 = ctrls_1;

	group::convert_special_to_empty_and_full_to_tomb(ctrls_2.data());

	for (auto const [x, y] : std::views::zip(ctrls_1, ctrls_2))
	{
		auto const value = static_cast<uint8_t>(x);

		ctrl const expected = value < 0x80
			? ctrl::ctrl_tomb
			: ctrl::ctrl_empty;

		REQUIRE(y == expected);
	}
}

} // namespace
