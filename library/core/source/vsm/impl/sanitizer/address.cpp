#include <vsm/sanitizer/address.h>

#if vsm_has_address_sanitizer

extern "C" {

vsm_no_sanitize_address
void* vsm_memcpy_no_sanitize_address(void* const dst, void const* const src, size_t const size)
{
	if (dst != src && size != 0)
	{
		unsigned char volatile* dst_pos = static_cast<unsigned char volatile*>(dst);
		unsigned char const volatile* src_pos = static_cast<unsigned char const volatile*>(src);
		unsigned char const volatile* const src_end = src_pos + size;

		while (src_pos != src_end)
		{
			*dst_pos++ = *src_pos++;
		}
	}

	return dst;
}

vsm_no_sanitize_address
void* vsm_memset_no_sanitize_address(void* const dst, int const value, size_t const size)
{
	if (size != 0)
	{
		unsigned char volatile* dst_pos = static_cast<unsigned char volatile*>(dst);
		unsigned char volatile* const dst_end = dst_pos + size;

		while (dst_pos != dst_end)
		{
			*dst_pos++ = static_cast<unsigned char>(value);
		}
	}

	return dst;
}

vsm_no_sanitize_address
void* vsm_memmove_no_sanitize_address(void* const dst, void const* const src, size_t const size)
{
	if (dst != src && size != 0)
	{
		unsigned char volatile* dst_pos = static_cast<unsigned char volatile*>(dst);
		unsigned char volatile* dst_end = dst_pos + size;

		unsigned char const volatile* src_pos = static_cast<unsigned char const volatile*>(src);
		unsigned char const volatile* src_end = src_pos + size;

		if (reinterpret_cast<uintptr_t>(dst) < reinterpret_cast<uintptr_t>(src))
		{
			while (src_pos != src_end)
			{
				*dst_pos++ = *src_pos++;
			}
		}
		else
		{
			while (src_pos != src_end)
			{
				*--dst_end = *--src_end;
			}
		}
	}

	return dst;
}

} // extern "C"

#endif
