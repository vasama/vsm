#include <vsm/function_view.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {
namespace static_tests {

template<int Category>
struct callable
{
	void operator()() & requires ((Category & 0b1000) != 0);
	void operator()() && requires ((Category & 0b0100) != 0);
	void operator()() const& requires ((Category & 0b0010) != 0);
	void operator()() const&& requires ((Category & 0b0001) != 0);
};

template<int Category>
using c = callable<Category>;

using v = function_view<void()>;

static_assert(requires { v(std::declval<c<0b1000>&>()); });
static_assert(requires { v(std::declval<c<0b0100>&&>()); });
static_assert(requires { v(std::declval<c<0b0010> const&>()); });
static_assert(requires { v(std::declval<c<0b0001> const&&>()); });

} // namespace static_tests
} // namespace


using function_view_type = function_view<int(int)>;

#if 0
TEST_CASE("function_view default constructor", "[function_view]")
{
	function_view_type const f;
	REQUIRE(!f);
}

TEST_CASE("function_view borrows callable object", "[function_view]")
{
	int y = 7;

	auto const callable = [&](int const x) -> int
	{
		return x * y;
	};

	function_view_type const f = callable;
	REQUIRE(f);

	if (GENERATE(0, 1))
	{
		y = -y;
	}

	CHECK(f(42) == 42 * y);
}

TEST_CASE("function_view binds to function", "[function_view]")
{
	auto const callable = [](int const x) -> int
	{
		return x * 7;
	};

	int(*const function)(int) = callable;

	function_view_type const f = *function;
	REQUIRE(f);

	CHECK(f(42) == 42 * 7);
}
#endif
