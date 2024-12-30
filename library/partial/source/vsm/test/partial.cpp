#include <vsm/partial.hpp>

#include <vsm/testing/dependent_context.hpp>

using namespace vsm;

namespace {

class s
{
	vsm_partial(s);

public:
	void f();
	void f() const;

	void g();
	void g() const;
};

using s_internal = vsm::partial::internal_class<s>;
class s::internal_class : public s
{
};

using s_private = vsm::partial::private_class<s>;
class s::private_class : public internal_class
{
};

[[maybe_unused]] void s::f()
{
	vsm_self(internal_class);
	static_assert(std::is_same_v<decltype(self), s::internal_class* const>);
}

[[maybe_unused]] void s::f() const
{
	vsm_self(internal_class);
	static_assert(std::is_same_v<decltype(self), s::internal_class const* const>);
}

[[maybe_unused]] void s::g()
{
	vsm_self(private_class);
	static_assert(std::is_same_v<decltype(self), s::private_class* const>);
}

[[maybe_unused]] void s::g() const
{
	vsm_self(private_class);
	static_assert(std::is_same_v<decltype(self), s::private_class const* const>);
}


class t
{
	vsm_partial(t);
	vsm_partial_delete(t);
};

[[maybe_unused]] void t::operator delete(t*, std::destroying_delete_t)
{
}


vsm_dependent_context
{
	static_assert(not requires { new vsm_dependent_t(s); });
#if vsm_compiler_gcc
	static_assert(not requires { vsm_dependent_v(static_cast<s*>(nullptr))->~s(); });
#endif

	static_assert(requires { new vsm_dependent_t(s_private); });
	static_assert(requires { vsm_dependent_v(static_cast<s_private*>(nullptr))->~s_private(); });

	static_assert(not requires { new vsm_dependent_t(t); });
#if vsm_compiler_gcc
	static_assert(not requires { vsm_dependent_v(static_cast<t*>(nullptr))->~t(); });
#endif
	static_assert(requires { delete vsm_dependent_v(static_cast<t*>(nullptr)); });
};

} // namespace
