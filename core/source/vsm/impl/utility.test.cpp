#include <vsm/utility.hpp>

#include <vsm/test/dependent_context.hpp>

namespace {

int prvalue();

int lvalue = 0;
int const const_lvalue = 0;

int& lvalue_ref = lvalue;
int const& const_lvalue_ref = lvalue;

int&& rvalue_ref = static_cast<int&&>(lvalue);
int const&& const_rvalue_ref = static_cast<int&&>(lvalue);


/* vsm_forward */

static_assert(std::is_same_v<decltype(vsm_forward(prvalue())), int&&>);

static_assert(std::is_same_v<decltype(vsm_forward(lvalue_ref)), int&>);
static_assert(std::is_same_v<decltype(vsm_forward(rvalue_ref)), int&&>);

static_assert(std::is_same_v<decltype(vsm_forward(const_lvalue_ref)), int const&>);
static_assert(std::is_same_v<decltype(vsm_forward(const_rvalue_ref)), int const&&>);


/* vsm_move */

static_assert(std::is_same_v<decltype(vsm_move(prvalue())), int&&>);

static_assert(std::is_same_v<decltype(vsm_move(lvalue)), int&&>);
static_assert(std::is_same_v<decltype(vsm_move(lvalue_ref)), int&&>);
static_assert(std::is_same_v<decltype(vsm_move(rvalue_ref)), int&&>);

static_assert(std::is_same_v<decltype(vsm_move(const_lvalue)), int const&&>);
static_assert(std::is_same_v<decltype(vsm_move(const_lvalue_ref)), int const&&>);
static_assert(std::is_same_v<decltype(vsm_move(const_rvalue_ref)), int const&&>);


/* vsm_as_const */

vsm_dependent_context
(
	static_assert(not requires { vsm_as_const(vsm_dep_v(prvalue())); });
);

static_assert(std::is_same_v<decltype(vsm_as_const(lvalue)), int const&>);
static_assert(std::is_same_v<decltype(vsm_as_const(const_lvalue)), int const&>);

static_assert(std::is_same_v<decltype(vsm_as_const(lvalue_ref)), int const&>);
static_assert(std::is_same_v<decltype(vsm_as_const(const_lvalue_ref)), int const&>);

} // namespace
