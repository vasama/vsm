#pragma once

#include <vsm/assert.h>

#include <span>
#include <string_view>

namespace vsm {
namespace detail {

inline constexpr char base64_padding_char = '=';

} // namespace detail

template<typename Char>
[[nodiscard]] bool base64_validate(Char const* const data, size_t const size);

[[nodiscard]] inline constexpr size_t base64_encoded_size(size_t const decoded_size)
{
	return decoded_size * 4 / 3 + 3 & static_cast<size_t>(-4);
}

template<typename Char>
[[nodiscard]] constexpr size_t base64_decoded_size(
	Char const* const encoded_data,
	size_t const encoded_size)
{
	vsm_assert(encoded_size % 4 == 0); //PRECONDITION
	size_t const s = encoded_size & static_cast<size_t>(-4);

	if (s == 0)
	{
		return 0;
	}

	size_t padding = 0;
	if (encoded_data[s - 1] == static_cast<Char>(detail::base64_padding_char))
	{
		++padding;
		if (encoded_data[s - 2] == static_cast<Char>(detail::base64_padding_char))
		{
			++padding;
		}
	}

	return s / 4 * 3 - padding;
}


template<typename Char>
void base64_encode(
	void const* const decoded_data,
	size_t const decoded_size,
	Char* const encoded_data);

template<typename Char>
void base64_encode_inplace(void* decoded_data, size_t decoded_size);

template<typename Char>
void base64_decode(Char const* encoded_data, size_t encoded_size, void* decoded_data);

template<typename Char>
void base64_decode_inplace(void* encoded_data, size_t encoded_size);

} // namespace vsm
