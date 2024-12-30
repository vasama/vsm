#pragma once

#include <vsm/platform.h>

#include <type_traits>

namespace vsm {

vsm_gnu_diagnostic(push)

vsm_gcc_diagnostic(ignored "-Wcast-function-type")

#if vsm_compiler_clang && __clang_major__ >= 19
vsm_clang_diagnostic(ignored "-Wcast-function-type-mismatch")
#endif

class function_ptr_t
{
	struct incomplete_type;
	using function_type = void(incomplete_type);

	function_type* m_function;

public:
	function_ptr_t() = default;

	template<typename F>
	/*constexpr*/ function_ptr_t(F* const function) noexcept
		requires std::is_function_v<F>
		: m_function(reinterpret_cast<function_type*>(function))
	{
	}

	template<typename F>
	explicit /*constexpr*/ operator F*() const noexcept
		requires std::is_function_v<F>
	{
		return reinterpret_cast<F*>(m_function);
	}
};

vsm_gnu_diagnostic(pop)

} // namespace vsm
