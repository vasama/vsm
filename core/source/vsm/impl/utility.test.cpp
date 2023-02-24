#include <vsm/utility.hpp>

namespace {

template<typename int_ = int>
int_ prvalue();

int lvalue = 0;
int const const_lvalue = 0;

int& lvalue_ref = lvalue;
int const& const_lvalue_ref = lvalue;


/* vsm_forward */

static_assert(std::is_same_v<decltype(vsm_forward(prvalue<int&>())), int&>);
static_assert(std::is_same_v<decltype(vsm_forward(prvalue<int&&>())), int&&>);

static_assert(std::is_same_v<decltype(vsm_forward(prvalue<int const&>())), int const&>);
static_assert(std::is_same_v<decltype(vsm_forward(prvalue<int const&&>())), int const&&>);


/* vsm_move */

static_assert(std::is_same_v<decltype(vsm_move(prvalue())), int&&>);

static_assert(std::is_same_v<decltype(vsm_move(lvalue)), int&&>);
static_assert(std::is_same_v<decltype(vsm_move(const_lvalue)), int const&&>);

static_assert(std::is_same_v<decltype(vsm_move(lvalue_ref)), int&&>);
static_assert(std::is_same_v<decltype(vsm_move(const_lvalue_ref)), int const&&>);


/* vsm_as_const */

template<typename int_>
constexpr bool as_const_test()
{
	static_assert(!requires
	{
		vsm_as_const(prvalue<int_>());
	});

	return true;
}
static_assert(as_const_test<int>());

static_assert(std::is_same_v<decltype(vsm_as_const(lvalue)), int const&>);
static_assert(std::is_same_v<decltype(vsm_as_const(const_lvalue)), int const&>);

static_assert(std::is_same_v<decltype(vsm_as_const(lvalue_ref)), int const&>);
static_assert(std::is_same_v<decltype(vsm_as_const(const_lvalue_ref)), int const&>);

} // namespace
