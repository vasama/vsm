#include <vsm/tag_ptr.hpp>

#include <vsm/testing/dependent_context.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

template<size_t Alignment>
struct alignas(Alignment) s;

using incomplete_ptr_1 = incomplete_tag_ptr<s<1>, bool, true>;
using incomplete_ptr_2 = incomplete_tag_ptr<s<2>, bool, true>;


template<size_t Alignment>
struct alignas(Alignment) s
{
	signed char v;
};

static_assert(!check_incomplete_tag_ptr<incomplete_ptr_1>());
static_assert(check_incomplete_tag_ptr<incomplete_ptr_2>());


template<typename T>
using ptr = tag_ptr<T, bool>;

vsm_dependent_context
{
	static_assert(requires { typename ptr<s<2>>; });
	static_assert(not requires { typename ptr<vsm_dependent_t(s<1>)>; });
};

TEST_CASE("tag_ptr", "[tag_ptr]")
{
	s<2> object = { 42 };

	bool const tag = GENERATE(0, 1);
	ptr<s<2>> p = { &object, tag };

	REQUIRE(p != nullptr);
	REQUIRE(p.ptr() == &object);
	CHECK(p.tag() == tag);
}

TEST_CASE("tag_ptr static_pointer_cast", "[tag_ptr]")
{
	using b = s<2>;
	using b_ptr = ptr<b const>;

	struct d : b {};
	using d_ptr = ptr<d const>;

	d const object = {};

	b_ptr const p_b = { &object, true };
	d_ptr const p_d = static_pointer_cast<d_ptr>(p_b);

	REQUIRE(p_d != nullptr);
	REQUIRE(p_d.ptr() == &object);
	REQUIRE(p_d.tag() == true);
}

TEST_CASE("tag_ptr const_pointer_cast", "[tag_ptr]")
{
	using t = s<2>;
	
	using m_ptr = ptr<t>;
	using c_ptr = ptr<t const>;

	t object = {};

	c_ptr const p_c = { &object, true };
	m_ptr const p_m = const_pointer_cast<m_ptr>(p_c);

	REQUIRE(p_m != nullptr);
	REQUIRE(p_m.ptr() == &object);
	REQUIRE(p_m.tag() == true);

	p_m->v = 1;
	CHECK(p_c->v == 1);
}

TEST_CASE("tag_ptr reinterpret_pointer_cast pointer", "[tag_ptr]")
{
	using t = s<2>;
	using t_ptr = ptr<t>;

	using u = s<4>;
	using u_ptr = ptr<u>;

	t object = {};

	u_ptr const p_u = { reinterpret_cast<u*>(&object), true };
	t_ptr const p_t = reinterpret_pointer_cast<t_ptr>(p_u);

	REQUIRE(p_t != nullptr);
	REQUIRE(p_t.ptr() == &object);
	REQUIRE(p_t.tag() == true);
}

TEST_CASE("tag_ptr reinterpret_pointer_cast integer", "[tag_ptr]")
{
	using t = s<2>;
	using t_ptr = ptr<t>;

	t object = {};
	
	t_ptr const p_t1 = { &object, true };
	uintptr_t const integer = reinterpret_pointer_cast<uintptr_t>(p_t1);
	t_ptr const p_t2 = reinterpret_pointer_cast<t_ptr>(integer);

	REQUIRE(p_t2 != nullptr);
	REQUIRE(p_t2.ptr() == &object);
	REQUIRE(p_t2.tag() == true);
}

TEST_CASE("tag_ptr dynamic_pointer_cast", "[tag_ptr]")
{
	struct b
	{
		virtual ~b() = default;
	};
	using b_ptr = ptr<b>;

	struct d : b
	{
	};
	using d_ptr = ptr<d>;

	d object = {};

	b_ptr const p_b = { &object, true };
	d_ptr const p_d = dynamic_pointer_cast<d_ptr>(p_b);

	REQUIRE(p_d != nullptr);
	REQUIRE(p_d.ptr() == &object);
	REQUIRE(p_d.tag() == true);
}

} // namespace
