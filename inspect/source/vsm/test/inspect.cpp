#include <vsm/inspect.hpp>

#include <catch2/catch_all.hpp>

#include <variant>

using namespace vsm;

namespace {

template<size_t>
struct wrapper
{
	int value;
};

} // namespace

TEST_CASE("inspect", "[inspect]")
{
	struct dummy {};

	using variant = std::variant<
		dummy
		, wrapper<1>
		, wrapper<2>
		, wrapper<3>
		, wrapper<4>
	>;

	auto v = GENERATE(
		as<variant>()
		, dummy{}
		, wrapper<1>{ 1 }
		, wrapper<2>{ 2 }
		, wrapper<3>{ 3 }
		, wrapper<4>{ 4 }
	);

	vsm_inspect(v)
	{
		vsm_match(wrapper<1> w)
		{
			CHECK(w.value == 1);
		}

		vsm_match(wrapper<2> const w)
		{
			CHECK(w.value == 2);
		}

		vsm_match(wrapper<3>& w)
		{
			CHECK(w.value == 3);
		}

		vsm_match(wrapper<4> const& w)
		{
			CHECK(w.value == 4);
		}
	}
}
