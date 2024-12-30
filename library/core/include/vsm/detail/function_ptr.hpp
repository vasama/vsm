#pragma once

#include <vsm/platform.h>

#include <type_traits>

namespace vsm::detail {

#define vsm_detail_function_ptr_constexpr

class function_ptr_t
{
	vsm_gnu_diagnostic(push)

	vsm_gcc_diagnostic(ignored "-Wcast-function-type")

#if vsm_compiler_clang && __clang_major__ >= 19
	vsm_clang_diagnostic(ignored "-Wcast-function-type-mismatch")
#endif

	struct incomplete_type;
	using function_type = void(incomplete_type);

	function_type* m_function;

public:
	template<typename F>
	constexpr function_ptr_t(F* const function)
		requires std::is_function_v<F>
		: m_function(reinterpret_cast<function_type*>(function))
	{
	}

	template<typename F>
	explicit constexpr operator F*() const
		requires std::is_function_v<F>
	{
		return reinterpret_cast<F*>(m_function);
	}

	vsm_gnu_diagnostic(pop)
};

} // namespace vsm::detail
