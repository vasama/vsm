#include <vsm/test/dependent_context.hpp>

vsm_dependent_context
{
	struct s {};

	static_assert(not requires { typename vsm_dependent_t(s)::type; });
	static_assert(not requires { vsm_dependent_v(s()) + vsm_dependent_v(s()); });
};
