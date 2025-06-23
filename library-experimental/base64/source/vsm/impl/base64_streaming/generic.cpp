#include <vsm/base64_streaming.hpp>

using namespace vsm;
using namespace vsm::detail;

template<typename Char>
void detail::base64_encode_fast_3x(void const* const pos, ptrdiff_t const diff_div_3, Char* out)
{
	unsigned char const* unsigned_char_pos = static_cast<unsigned char const*>(pos);
	base64_encode_constexpr_3x<Char>(unsigned_char_pos, diff_div_3, out);
}

template<typename Char>
void detail::base64_decode_fast_4x(Char const* pos, ptrdiff_t const size_div_4, void* const out)
{
	unsigned char* unsigned_char_out = static_cast<unsigned char*>(out);
	base64_decode_constexpr_4x<unsigned char>(pos, size_div_4, unsigned_char_out);
}
