#include <vsm/inplace_function.hpp>

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
using function_template = inplace_function<T, 8>;

TEMPLATE_TEST_CASE(
	"inplace_function value category",
	"[inplace_function]",
	std::false_type,
	std::true_type)
{
	static constexpr bool is_noexcept = TestType::value;
	using canary = basic_canary<is_noexcept>;

	using s = std::string_view;

	SECTION("T")
	{
		using function_type = function_template<s() noexcept(is_noexcept)>;
		function_type f = canary();

		REQUIRE(f() == "&");
		REQUIRE(vsm_move(f)() == "&");

		static_assert(!requires { static_cast<function_type const&>(f)(); });
		static_assert(!requires { static_cast<function_type const&&>(f)(); });
	}

	SECTION("T&")
	{
		using function_type = function_template<s() & noexcept(is_noexcept)>;
		function_type f = canary();

		REQUIRE(f() == "&");
		static_assert(!requires { vsm_move(f)(); });

		static_assert(!requires { static_cast<function_type const&>(f)(); });
		static_assert(!requires { static_cast<function_type const&&>(f)(); });
	}

	SECTION("T&&")
	{
		using function_type = function_template<s() && noexcept(is_noexcept)>;
		function_type f = canary();

		static_assert(!requires { f(); });
		REQUIRE(vsm_move(f)() == "&&");

		static_assert(!requires { static_cast<function_type const&>(f)(); });
		static_assert(!requires { static_cast<function_type const&&>(f)(); });
	}

	SECTION("T const")
	{
		using function_type = function_template<s() const noexcept(is_noexcept)>;
		function_type f = canary();

		REQUIRE(f() == "const&");
		REQUIRE(vsm_move(f)() == "const&");

		REQUIRE(static_cast<function_type const&>(f)() == "const&");
		REQUIRE(static_cast<function_type const&&>(f)() == "const&");
	}

	SECTION("T const&")
	{
		using function_type = function_template<s() const& noexcept(is_noexcept)>;
		function_type f = canary();

		REQUIRE(f() == "const&");
		REQUIRE(vsm_move(f)() == "const&");

		REQUIRE(static_cast<function_type const&>(f)() == "const&");
		REQUIRE(static_cast<function_type const&&>(f)() == "const&");
	}

	SECTION("T const&&")
	{
		using function_type = function_template<s() const&& noexcept(is_noexcept)>;
		function_type f = canary();

		static_assert(!requires { f(); });
		REQUIRE(vsm_move(f)() == "const&&");

		static_assert(!requires { static_cast<function_type const&>(f)(); });
		REQUIRE(static_cast<function_type const&&>(f)() == "const&&");
	}
}

} // namespace
