#include <vsm/base64_streaming.hpp>

#include <vsm/platform.h>
#include <vsm/preprocessor.h>

#define vsm_base64_instantiate(extern, Char) \
	extern template void vsm::detail::base64_encode_fast_3x<Char>(void const*, ptrdiff_t, Char*); \
	extern template void vsm::detail::base64_decode_fast_4x<Char>(Char const*, ptrdiff_t, void*); \

#define vsm_base64_instantiate_all(extern) \
	vsm_base64_instantiate(extern, char) \
	vsm_base64_instantiate(extern, wchar_t) \
	vsm_base64_instantiate(extern, char8_t) \
	vsm_base64_instantiate(extern, char16_t) \
	vsm_base64_instantiate(extern, char32_t) \

// Needed due to possible Clang bug: https://github.com/llvm/llvm-project/issues/134289
vsm_base64_instantiate_all(extern)

#if __has_include(vsm_pp_include(vsm/impl/base64_streaming/vsm_arch.hpp))
#	include vsm_pp_include(vsm/impl/base64_streaming/vsm_arch.cpp)
#else
#	include <vsm/impl/base64_streaming/generic.cpp>
#endif

vsm_base64_instantiate_all()
