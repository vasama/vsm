#include <vsm/detail/swiss_table.hpp>

#include <catch2/catch_all.hpp>

#include <random>
#include <ranges>

using namespace vsm;
using namespace vsm::detail;

namespace {

using group_types = std::tuple<
	_swiss_table_group_generic<size_t>

#if vsm_arch_x86
	, _swiss_table_group_x86
#endif
>;

TEMPLATE_LIST_TEST_CASE("swiss table group hash matching", "[hash_table][swiss_table]", group_types)
{
	using group_type = TestType;

	_swiss_table_ctrl const fill = GENERATE(
		_swiss_table_ctrl::empty,
		_swiss_table_ctrl::tomb,
		_swiss_table_ctrl::end);

	for (size_t i = 0; i <= 0x7f; ++i)
	{
		std::array<_swiss_table_ctrl, group_type::size> ctrl;
		ctrl.fill(fill);

		ctrl[0] = static_cast<_swiss_table_ctrl>(i);
		ctrl[group_type::size - 1] = static_cast<_swiss_table_ctrl>(i);

		auto const match = group_type(ctrl.data()).match(static_cast<_swiss_table_ctrl>(i));
		size_t const indices[] = { 0, group_type::size - 1 };

		REQUIRE(std::ranges::equal(match, indices));
	}
}

TEMPLATE_LIST_TEST_CASE("swiss_table group matching", "[hash_table][swiss_table]", group_types)
{
	using group_type = TestType;

	auto const get_bools = [](auto const& iterator)
	{
		std::array<bool, group_type::size> bools = {};
		for (size_t const index : iterator)
		{
			bools[index] = true;
		}
		return bools;
	};

	std::array<_swiss_table_ctrl, group_type::size> ctrl = {};
	std::array<bool, group_type::size> bools = {};

	ctrl.fill(GENERATE(static_cast<_swiss_table_ctrl>(0), _swiss_table_ctrl::end));

	auto const set_values = [&](std::same_as<_swiss_table_ctrl> auto const... values)
	{
		if (GENERATE(0, 1))
		{
			ctrl[0] = GENERATE_COPY(values...);
			bools[0] = true;
		}

		if (GENERATE(0, 1))
		{
			ctrl[1] = GENERATE_COPY(values...);
			bools[1] = true;
		}

		if (GENERATE(0, 1))
		{
			_swiss_table_ctrl const value = GENERATE_COPY(values...);

			for (size_t i = 2; i < group_type::size - 2; ++i)
			{
				ctrl[i] = value;
				bools[i] = true;
			}
		}

		if (GENERATE(0, 1))
		{
			ctrl[group_type::size - 2] = GENERATE_COPY(values...);
			bools[group_type::size - 2] = true;
		}

		if (GENERATE(0, 1))
		{
			ctrl[group_type::size - 1] = GENERATE_COPY(values...);
			bools[group_type::size - 1] = true;
		}
	};

	std::array<bool, group_type::size> match = {};

	SECTION("h2(x)")
	{
		if (ctrl.front() == _swiss_table_ctrl::end && GENERATE(0, 1))
		{
			ctrl.fill(GENERATE(_swiss_table_ctrl::empty, _swiss_table_ctrl::tomb));
		}

		_swiss_table_ctrl const value = static_cast<_swiss_table_ctrl>(GENERATE(1, 0x7f));

		set_values(value);
		match = get_bools(group_type(ctrl.data()).match(value));
	}

	SECTION("empty")
	{
		if (ctrl.front() == _swiss_table_ctrl::end && GENERATE(0, 1))
		{
			ctrl.fill(GENERATE(_swiss_table_ctrl::tomb));
		}

		set_values(_swiss_table_ctrl::empty);
		match = get_bools(group_type(ctrl.data()).match_empty());
	}

	SECTION("free")
	{
		set_values(_swiss_table_ctrl::empty, _swiss_table_ctrl::tomb);
		match = get_bools(group_type(ctrl.data()).match_free());
	}

	REQUIRE(match == bools);
}

TEMPLATE_LIST_TEST_CASE("swiss_table group iteration", "[hash_table][swiss_table]", group_types)
{
	using group_type = TestType;

	std::array<_swiss_table_ctrl, group_type::size> ctrl = {};
	ctrl.fill(GENERATE(_swiss_table_ctrl::empty, _swiss_table_ctrl::tomb));

	size_t const index = GENERATE(range(static_cast<size_t>(0), group_type::size - 1));

	ctrl[index] = GENERATE(
		_swiss_table_ctrl::end,
		static_cast<_swiss_table_ctrl>(0),
		static_cast<_swiss_table_ctrl>(0x7f));

	size_t const match = group_type(ctrl.data()).count_leading_free_or_end();

	REQUIRE(match == index);
}

TEMPLATE_LIST_TEST_CASE("swiss_table group convert", "[hash_table][swiss_table]", group_types)
{
	using group_type = TestType;

	std::array<_swiss_table_ctrl, group_type::size> ctrl_1 = {};

	ctrl_1[0] = _swiss_table_ctrl::empty;
	ctrl_1[1] = _swiss_table_ctrl::tomb;
	ctrl_1[2] = _swiss_table_ctrl::end;

	std::ranges::shuffle(ctrl_1, Catch::sharedRng());
	std::array<_swiss_table_ctrl, group_type::size> ctrl_2 = ctrl_1;

	group_type::convert_special_to_empty_and_full_to_tomb(ctrl_2.data());

	for (auto const [x, y] : std::views::zip(ctrl_1, ctrl_2))
	{
		auto const value = static_cast<uint8_t>(x);

		_swiss_table_ctrl const expected = value < 0x80
			? _swiss_table_ctrl::tomb
			: _swiss_table_ctrl::empty;

		REQUIRE(y == expected);
	}
}

} // namespace
