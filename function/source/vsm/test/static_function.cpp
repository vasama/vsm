#include <vsm/static_function.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

template<bool N>
struct basic_canary
{
	std::string_view operator()() & noexcept(N)
	{
		return "&";
	}
	std::string_view operator()() && noexcept(N)
	{
		return "&&";
	}
	std::string_view operator()() const& noexcept(N)
	{
		return "const&";
	}
	std::string_view operator()() const&& noexcept(N)
	{
		return "const&&";
	}
};

template<typename T>
using f = static_function<T, 8>;

TEMPLATE_TEST_CASE("static_function value category", "[static_function]", std::false_type, std::true_type)
{
	static constexpr bool is_noexcept = TestType::value;
	using canary = basic_canary<is_noexcept>;

	using s = std::string_view;

	//TODO: Negative compilation tests for lvalue/rvalue

	SECTION("T")
	{
		f<s() noexcept(is_noexcept)> x = canary();
		REQUIRE(x() == "&");
		REQUIRE(vsm_move(x)() == "&");
	}

	SECTION("T&")
	{
		f<s() & noexcept(is_noexcept)> x = canary();
		REQUIRE(x() == "&");
	}

	SECTION("T&&")
	{
		f<s() && noexcept(is_noexcept)> x = canary();
		REQUIRE(vsm_move(x)() == "&&");
	}

	SECTION("T const")
	{
		f<s() const noexcept(is_noexcept)> x = canary();
		REQUIRE(x() == "const&");
		REQUIRE(vsm_move(x)() == "const&");
	}

	SECTION("T const&")
	{
		f<s() const& noexcept(is_noexcept)> x = canary();
		REQUIRE(x() == "const&");
	}

	SECTION("T const&&")
	{
		f<s() const&& noexcept(is_noexcept)> x = canary();
		REQUIRE(vsm_move(x)() == "const&&");
	}
}

} // namespace
