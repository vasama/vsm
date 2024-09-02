#include <vsm/atomic.hpp>

#include <vsm/test/dependent_context.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

vsm_dependent_context // atomic_ref
{
	using atomic_ref_t = vsm_dependent_t(atomic_ref<int>);

	static_assert(
		requires (int x) { atomic_ref_t(x); },
		"atomic_ref<T> can be constructed explicitly from T lvalue");

	static_assert(
		!requires (void(*f)(atomic_ref_t), int x) { f(x); },
		"atomic_ref<T> cannot be constructed implicitly from T lvalue");

	static_assert(
		!requires (atomic_ref_t a, int x) { a = x; },
		"atomic_ref<T> cannot be assigned T");
};

vsm_dependent_context // atomic
{
	using atomic_t = vsm_dependent_t(atomic<int>);

	static_assert(
		!requires (atomic_t a, atomic_t b) { a = b; },
		"atomic<T> cannot be copy assigned");
};

TEST_CASE("", "[atomic]")
{
	
}
