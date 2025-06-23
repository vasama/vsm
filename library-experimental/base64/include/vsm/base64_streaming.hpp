#pragma once

#include <vsm/assert.h>
#include <vsm/base64.hpp>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

template<typename OutputIterator>
struct out_completed_result
{
	vsm_no_unique_address OutputIterator out;
	bool completed;
};

template<typename Sentinel, typename Iterator>
concept sized_or_unreachable_sentinel_for =
	std::sentinel_for<Sentinel, Iterator> &&
	(
		std::sized_sentinel_for<Sentinel, Iterator> ||
		std::same_as<Sentinel, std::unreachable_sentinel_t>
	);


template<typename T>
concept base64_byte = vsm::byte_type<T> || (std::integral<T> && sizeof(T) == 1);

inline constexpr char base64_encode_table[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};

inline constexpr uint8_t base64_decode_table[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
};


template<typename Char>
void base64_encode_fast_3x(void const* pos, ptrdiff_t diff_div_3, Char* out);

template<typename Char, typename Iterator, typename OutputIterator>
constexpr void base64_encode_3(Iterator& pos, OutputIterator& out)
{
	uint8_t const _0 = static_cast<uint8_t>(*pos);
	uint8_t const _1 = static_cast<uint8_t>(*++pos);
	uint8_t const _2 = static_cast<uint8_t>(*++pos);
	++pos;

	/****/ *out = static_cast<Char>(base64_encode_table[_0 >> 2]);
	/**/ *++out = static_cast<Char>(base64_encode_table[_1 >> 4 | (_0 & 0b00000011) << 4]);
	/**/ *++out = static_cast<Char>(base64_encode_table[_2 >> 6 | (_1 & 0b00001111) << 2]);
	/**/ *++out = static_cast<Char>(base64_encode_table[/*******/ (_2 & 0b00111111)]);
	++out;
}

template<typename Char, typename Iterator, typename Sentinel, typename OutputIterator>
constexpr void base64_encode_end(Iterator& pos, Sentinel const& end, OutputIterator& out)
{
	if (pos != end)
	{
		uint8_t const _0 = static_cast<uint8_t>(*pos);
		if (++pos != end)
		{
			uint8_t const _1 = static_cast<uint8_t>(*pos);
			++pos;
	
			/****/ *out = static_cast<Char>(base64_encode_table[_0 >> 2]);
			/**/ *++out = static_cast<Char>(base64_encode_table[_1 >> 4 | (_0 & 0b00000011) << 4]);
			/**/ *++out = static_cast<Char>(base64_encode_table[/*******/ (_1 & 0b00001111) << 2]);
			/**/ *++out = static_cast<Char>(base64_padding_char);
		}
		else
		{
			/****/ *out = static_cast<Char>(base64_encode_table[_0 >> 2]);
			/**/ *++out = static_cast<Char>(base64_encode_table[/*******/ (_0 & 0b00000011) << 4]);
			/**/ *++out = static_cast<Char>(base64_padding_char);
			/**/ *++out = static_cast<Char>(base64_padding_char);
		}
		++out;
	}
}

template<typename Char, typename Iterator, typename OutputIterator>
constexpr void base64_encode_constexpr_3x(
	Iterator& pos,
	std::iter_difference_t<Iterator> const diff_div_3,
	OutputIterator& out)
{
	for (std::iter_difference_t<Iterator> i = 0; i < diff_div_3; ++i)
	{
		detail::base64_encode_3<Char>(pos, out);
	}
}

template<typename Char, typename Iterator, typename OutputIterator>
constexpr void base64_encode_3x(
	Iterator& pos,
	std::iter_difference_t<Iterator> const diff_div_3,
	OutputIterator& out)
{
	if constexpr (
		std::contiguous_iterator<Iterator> &&
		std::contiguous_iterator<OutputIterator>)
	{
		vsm_if_consteval
		{
			detail::base64_encode_constexpr_3x<Char>(pos, diff_div_3, out);
		}
		else
		{
			using outdiff_t = std::iter_difference_t<OutputIterator>;

			ptrdiff_t const pos_diff_div_3 = { diff_div_3 };
			outdiff_t const out_diff_div_4 = { diff_div_3 };

			detail::base64_encode_fast_3x<Char>(
				std::to_address(pos),
				pos_diff_div_3,
				std::to_address(out));

			pos += pos_diff_div_3 * 3;
			out += out_diff_div_4 * 4;
		}
	}
	else
	{
		detail::base64_encode_constexpr_3x<Char>(pos, diff_div_3, out);
	}
}

class base64_encoder
{
	uint8_t m_decoded_size = 0;
	uint8_t m_encoded_size = 0;

	alignas(4) uint8_t m_decoded_data[3] = {};
	alignas(4) uint8_t m_encoded_data[4] = {};

public:
	template<
		character Char,
		base64_byte Byte,
		std::output_iterator<Byte> OutputIterator,
		std::sentinel_for<OutputIterator> OutputSentinel>
	[[nodiscard]] constexpr OutputIterator encode_one(
		Byte const value,
		OutputIterator out_beg,
		OutputSentinel const out_end)
	{
		_encode_one<Char>(value, out_beg, out_end);
		return out_beg;
	}

	template<character Char, base64_byte Byte, std::output_iterator<Char> OutputIterator>
	[[nodiscard]] constexpr OutputIterator encode_one(Byte const value, OutputIterator out)
	{
		return encode_one<Char>(value, vsm_move(out), std::unreachable_sentinel);
	}

	template<character Char, base64_byte Byte, std::ranges::output_range<Char> OutputRange>
	[[nodiscard]] constexpr std::ranges::iterator_t<OutputRange> encode_one(
		Byte const value,
		OutputRange&& out_range)
	{
		return encode_one<Char>(value, std::ranges::begin(out_range), std::ranges::end(out_range));
	}

	template<
		character Char,
		std::input_iterator Iterator,
		std::sentinel_for<Iterator> Sentinel,
		std::output_iterator<Char> OutputIterator,
		std::sentinel_for<OutputIterator> OutputSentinel>
		requires base64_byte<std::iter_value_t<Iterator>>
	[[nodiscard]] constexpr std::ranges::in_out_result<Iterator, OutputIterator> encode(
		Iterator beg,
		Sentinel const end,
		OutputIterator out_beg,
		OutputSentinel const out_end)
	{
		if (out_beg != out_end)
		{
			if (_flush<Char>(out_beg, out_end))
			{
				if (beg != end)
				{
					_encode<Char>(beg, end, out_beg, out_end);
				}
			}
		}

		return { vsm_move(beg), vsm_move(out_beg) };
	}

	template<
		character Char,
		std::input_iterator Iterator,
		std::sentinel_for<Iterator> Sentinel,
		std::output_iterator<Char> OutputIterator>
		requires base64_byte<std::iter_value_t<Iterator>>
	[[nodiscard]] constexpr std::ranges::in_out_result<Iterator, OutputIterator> encode(
		Iterator beg,
		Sentinel end,
		OutputIterator out_beg)
	{
		return encode<Char>(
			vsm_move(beg),
			vsm_move(end),
			vsm_move(out_beg),
			std::unreachable_sentinel);
	}

	template<
		character Char,
		std::ranges::input_range Range,
		std::output_iterator<Char> OutputIterator>
	[[nodiscard]] constexpr
	std::ranges::in_out_result<std::ranges::iterator_t<Range>, OutputIterator> encode(
		Range&& range,
		OutputIterator out)
	{
		return encode<Char>(
			std::ranges::begin(range),
			std::ranges::end(range),
			vsm_move(out),
			std::unreachable_sentinel);
	}
	
	template<
		character Char,
		std::ranges::input_range Range,
		std::ranges::output_range<Char> OutputRange>
	[[nodiscard]] constexpr std::ranges::in_out_result<
		std::ranges::iterator_t<Range>,
		std::ranges::iterator_t<OutputRange>> encode(Range&& range, OutputRange&& out_range)
	{
		return encode<Char>(
			std::ranges::begin(range),
			std::ranges::end(range),
			std::ranges::begin(out_range),
			std::ranges::end(out_range));
	}

#if 0
	template<character Char, std::output_iterator<Char> OutputIterator>
	[[nodiscard]] OutputIterator encode(
		void const* const data,
		size_t const size,
		OutputIterator out)
	{
		unsigned char const* const unsigned_char_data = static_cast<unsigned char const*>(data);
		return encode<Char>(
			unsigned_char_data,
			unsigned_char_data + size,
			vsm_move(out),
			std::unreachable_sentinel);
	}
#endif

	template<
		character Char,
		std::output_iterator<Char> OutputIterator,
		std::sentinel_for<OutputIterator> OutputSentinel>
	[[nodiscard]] constexpr out_completed_result<OutputIterator> finalize(
		OutputIterator out_beg,
		OutputSentinel const out_end)
	{
		bool completed = false;

		if (_flush<Char>(out_beg, out_end))
		{
			if (m_decoded_size != 0)
			{
				uint8_t const* beg = m_decoded_data;
				uint8_t const* const end = beg + m_decoded_size;

				uint8_t* out = m_encoded_data;
				detail::base64_encode_end<uint8_t>(beg, end, out);
				m_encoded_size = static_cast<uint8_t>(out - m_encoded_data);

				completed = _flush<Char>(out_beg, out_end);
			}
		}

		return { vsm_move(out_beg), completed };
	}

	template<character Char, std::output_iterator<Char> OutputIterator>
	[[nodiscard]] constexpr out_completed_result<OutputIterator> finalize(OutputIterator out_beg)
	{
		return finalize<Char>(out_beg, std::unreachable_sentinel);
	}

	template<character Char, std::ranges::output_range<Char> OutputRange>
	[[nodiscard]] constexpr out_completed_result<std::ranges::iterator_t<OutputRange>> finalize(
		OutputRange&& out_range)
	{
		return finalize<Char>(std::ranges::begin(out_range), std::ranges::end(out_range));
	}

private:
	template<typename Byte>
	constexpr void _store_decoded(Byte const value)
	{
		m_decoded_data[m_decoded_size++] = static_cast<uint8_t>(value);
	}

	template<typename Iterator>
	constexpr void _encode_to_internal_3(Iterator& beg)
	{
		uint8_t* out = m_encoded_data;
		detail::base64_encode_3<uint8_t>(beg, out);
		m_encoded_size = static_cast<uint8_t>(out - m_encoded_data);
	}

	constexpr void _encode_internal_to_internal_3()
	{
		uint8_t const* beg = m_decoded_data;
		_encode_to_internal_3(beg);
		m_decoded_size = 0;
	}

#if 0
	template<typename Iterator>
	constexpr void _encode_to_internal_3(Iterator& beg)
	{
		if (m_decoded_size != 0)
		{
			uint8_t const* pos = m_decoded_data;
			uint8_t const* const end = pos + m_decoded_size;
			detail::base64_encode_end<Char>(pos, end, out);
			m_decoded_size = 0;
		}
	}
#endif

	template<
		typename Char,
		typename Byte,
		typename OutputIterator,
		typename OutputSentinel>
	constexpr void _encode_one(
		Byte const value,
		OutputIterator& out_beg,
		OutputSentinel const& out_end)
	{
		if (_flush<Char>(out_beg, out_end))
		{
			_store_decoded(value);

			if (m_decoded_size >= 3)
			{
				_encode_internal_to_internal_3();
				_flush<Byte>(out_beg, out_end);
			}
		}
	}

	template<
		typename Char,
		typename SourceIterator,
		typename SourceSentinel,
		typename OutputIterator,
		typename OutputSentinel>
	constexpr void _encode(
		SourceIterator& beg,
		SourceSentinel const& end,
		OutputIterator& out_beg,
		OutputSentinel const& out_end)
	{
		if (m_decoded_size != 0)
		{
			_store_decoded(*beg);
			if (++beg != end && m_decoded_size < 3)
			{
				_store_decoded(*beg);
				if (++beg != end && m_decoded_size < 3)
				{
					_store_decoded(*beg);
					++beg;
				}
			}

			if (m_decoded_size >= 3)
			{
				_encode_internal_to_internal_3();
			}

			if (!_flush<Char>(out_beg, out_end))
			{
				return;
			}

			if (beg == end)
			{
				return;
			}
		}

		if constexpr (
			std::sized_sentinel_for<SourceSentinel, SourceIterator> &&
			sized_or_unreachable_sentinel_for<OutputSentinel, OutputIterator>)
		{
			using source_diff_type = std::iter_difference_t<SourceIterator>;
			using output_diff_type = std::iter_difference_t<OutputIterator>;
			using common_diff_type = std::common_type_t<source_diff_type, output_diff_type>;

			source_diff_type const src_diff = end - beg;
			source_diff_type const src_diff_div_3 = src_diff / static_cast<source_diff_type>(3);
			source_diff_type const src_diff_mod_3 = src_diff % static_cast<source_diff_type>(3);

			if constexpr (std::sized_sentinel_for<OutputSentinel, OutputIterator>)
			{
				output_diff_type const out_diff = out_end - out_beg;
				output_diff_type const out_diff_div_4 = out_diff / static_cast<output_diff_type>(4);
				src_diff_div_3 = std::min<common_diff_type>(src_diff_div_3, out_diff_div_4);
			}

			if (src_diff_div_3 != 0)
			{
				detail::base64_encode_3x<Char>(beg, src_diff_div_3, out_beg);
			}

			if (src_diff_mod_3 >= 1)
			{
				_store_decoded(*beg);
				++beg;

				if (src_diff_mod_3 >= 2)
				{
					_store_decoded(*beg);
					++beg;
				}
			}
		}
		else
		{
			for (; beg != end && out_beg != out_end; ++beg)
			{
				_encode_one<Char>(*beg, out_beg, out_end);
			}
		}
	}

	template<typename Char, typename OutputIterator, typename OutputSentinel>
	constexpr bool _flush_one(OutputIterator& out_beg, OutputSentinel const& out_end)
	{
		if (m_encoded_size == 0)
		{
			return false;
		}

		*out_beg = static_cast<Char>(m_encoded_data[0]);
		++out_beg;

		m_encoded_data[0] = m_encoded_data[1];
		m_encoded_data[1] = m_encoded_data[2];
		m_encoded_data[2] = m_encoded_data[3];
		m_encoded_size -= 1;

		return true;
	}

	template<typename Char, typename OutputIterator, typename OutputSentinel>
	constexpr bool _flush(OutputIterator& out_beg, OutputSentinel const& out_end)
	{
		if (out_beg == out_end)
		{
			return false;
		}

		if (_flush_one<Char>(out_beg, out_end))
		{
			if (out_beg == out_end)
			{
				return false;
			}

			if (_flush_one<Char>(out_beg, out_end))
			{
				if (out_beg == out_end)
				{
					return false;
				}

				if (_flush_one<Char>(out_beg, out_end))
				{
					if (out_beg == out_end)
					{
						return false;
					}
				}
			}
		}

		return true;
	}
};


template<typename Char>
void base64_decode_fast_4x(Char const* pos, ptrdiff_t diff_div_4, void* out);

template<typename Byte, bool Checked, typename Iterator, typename OutputIterator>
constexpr void base64_decode_4(Iterator& pos, OutputIterator& out)
{
	uint8_t const _0 = static_cast<uint8_t>(*pos);
	uint8_t const _1 = static_cast<uint8_t>(*++pos);
	uint8_t const _2 = static_cast<uint8_t>(*++pos);
	uint8_t const _3 = static_cast<uint8_t>(*++pos);
	++pos;

	uint8_t const decoded_0 = base64_decode_table[_0 & 0x7F];
	uint8_t const decoded_1 = base64_decode_table[_1 & 0x7F];
	uint8_t const decoded_2 = base64_decode_table[_2 & 0x7F];
	uint8_t const decoded_3 = base64_decode_table[_3 & 0x7F];

	if constexpr (Checked)
	{
		*out = static_cast<Byte>(decoded_0 << 2 | decoded_1 >> 4);

		if (_2 != static_cast<uint8_t>(base64_padding_char))
		{
			*++out = static_cast<Byte>(decoded_1 << 4 | decoded_2 >> 2);

			if (_3 != static_cast<uint8_t>(base64_padding_char))
			{
				*++out = static_cast<Byte>(decoded_2 << 6 | decoded_3);
			}
		}
	}
	else
	{
		/****/ *out = static_cast<Byte>(decoded_0 << 2 | decoded_1 >> 4);
		/**/ *++out = static_cast<Byte>(decoded_1 << 4 | decoded_2 >> 2);
		/**/ *++out = static_cast<Byte>(decoded_2 << 6 | decoded_3);
	}

	++out;
}

template<typename Byte, typename Iterator, typename OutputIterator>
constexpr void base64_decode_constexpr_4x(
	Iterator& pos,
	std::iter_difference_t<Iterator> const diff_div_4,
	OutputIterator& out)
{
	for (std::iter_difference_t<Iterator> i = 0; i < diff_div_4; ++i)
	{
		detail::base64_decode_4<Byte, /* Checked: */ false>(pos, out);
	}
}

template<typename Byte, typename Iterator, typename OutputIterator>
constexpr void base64_decode_4x(
	Iterator& pos,
	std::iter_difference_t<Iterator> const diff_div_4,
	OutputIterator& out)
{
	if constexpr (
		std::contiguous_iterator<Iterator> &&
		std::contiguous_iterator<OutputIterator>)
	{
		vsm_if_consteval
		{
			detail::base64_decode_constexpr_4x<Byte>(pos, diff_div_4, out);
		}
		else
		{
			using outdiff_t = std::iter_difference_t<OutputIterator>;

			ptrdiff_t const pos_diff_div_4 = { diff_div_4 };
			outdiff_t const out_diff_div_3 = { diff_div_4 };

			detail::base64_decode_fast_4x<std::iter_value_t<Iterator>>(
				std::to_address(pos),
				pos_diff_div_4,
				std::to_address(out));

			pos += pos_diff_div_4 * 4;
			out += out_diff_div_3 * 3;
		}
	}
	else
	{
		detail::base64_decode_constexpr_4x<Byte>(pos, diff_div_4, out);
	}
}

class base64_decoder
{
	uint8_t m_encoded_size = 0;
	uint8_t m_decoded_size = 0;

	alignas(4) uint8_t m_encoded_data[4] = {};
	alignas(4) uint8_t m_decoded_data[3] = {};

public:
	[[nodiscard]] constexpr bool is_valid_final_state() const
	{
		return m_encoded_size != 0;
	}

	template<
		base64_byte Byte,
		character Char,
		std::output_iterator<Byte> OutputIterator,
		std::sentinel_for<OutputIterator> OutputSentinel>
	[[nodiscard]] constexpr OutputIterator decode_one(
		Char const value,
		OutputIterator out_beg,
		OutputSentinel const out_end)
	{
		_decode_one<Byte>(value, out_beg, out_end);
		return out_beg;
	}

	template<base64_byte Byte, character Char, std::output_iterator<Byte> OutputIterator>
	[[nodiscard]] constexpr OutputIterator decode_one(Char const value, OutputIterator out_beg)
	{
		return decode_one<Byte>(value, vsm_move(out_beg), std::unreachable_sentinel);
	}

	template<base64_byte Byte, character Char, std::ranges::output_range<Byte> OutputRange>
	[[nodiscard]] constexpr std::ranges::iterator_t<OutputRange> encode_one(
		Char const value,
		OutputRange&& out_range)
	{
		return decode_one<Byte>(value, std::ranges::begin(out_range), std::ranges::end(out_range));
	}

	template<
		base64_byte Byte,
		std::input_iterator Iterator,
		std::sentinel_for<Iterator> Sentinel,
		std::output_iterator<Byte> OutputIterator,
		std::sentinel_for<OutputIterator> OutputSentinel>
		requires character<std::iter_value_t<Iterator>>
	[[nodiscard]] constexpr std::ranges::in_out_result<Iterator, OutputIterator> decode(
		Iterator beg,
		Sentinel const end,
		OutputIterator out_beg,
		OutputSentinel const out_end)
	{
		if (out_beg != out_end)
		{
			if (_flush<Byte>(out_beg, out_end))
			{
				if (beg != end)
				{
					_decode<Byte>(beg, end, out_beg, out_end);
				}
			}
		}

		return { vsm_move(beg), vsm_move(out_beg) };
	}

	template<
		base64_byte Byte,
		std::input_iterator Iterator,
		std::sentinel_for<Iterator> Sentinel,
		std::output_iterator<Byte> OutputIterator>
		requires character<std::iter_value_t<Iterator>>
	[[nodiscard]] constexpr std::ranges::in_out_result<Iterator, OutputIterator> decode(
		Iterator beg,
		Sentinel end,
		OutputIterator out_beg)
	{
		return decode<Byte>(
			vsm_move(beg),
			vsm_move(end),
			vsm_move(out_beg),
			std::unreachable_sentinel);
	}

	template<
		base64_byte Byte,
		std::ranges::input_range Range,
		std::output_iterator<Byte> OutputIterator>
	[[nodiscard]] constexpr
	std::ranges::in_out_result<std::ranges::iterator_t<Range>, OutputIterator> decode(
		Range&& range,
		OutputIterator out)
	{
		return decode<Byte>(
			std::ranges::begin(range),
			std::ranges::end(range),
			vsm_move(out),
			std::unreachable_sentinel);
	}

	template<
		base64_byte Byte,
		std::ranges::input_range Range,
		std::ranges::output_range<Byte> OutputRange>
	[[nodiscard]] constexpr std::ranges::in_out_result<
		std::ranges::iterator_t<Range>,
		std::ranges::iterator_t<OutputRange>> decode(Range&& range, OutputRange&& out_range)
	{
		return decode<Byte>(
			std::ranges::begin(range),
			std::ranges::end(range),
			std::ranges::begin(out_range),
			std::ranges::end(out_range));
	}

private:
	template<typename Char>
	constexpr void _store_encoded(Char const value)
	{
		m_encoded_data[m_encoded_size++] = static_cast<uint8_t>(value);
	}

	template<typename Iterator>
	constexpr void _decode_to_internal_4(Iterator& beg)
	{
		uint8_t* out = m_decoded_data;
		detail::base64_decode_4<uint8_t, /* Checked: */ true>(beg, out);
		m_decoded_size = static_cast<uint8_t>(out - m_decoded_data);
	}

	constexpr void _decode_internal_to_internal_4()
	{
		uint8_t const* beg = m_encoded_data;
		_decode_to_internal_4(beg);
		m_encoded_size = 0;
	}

	template<
		typename Byte,
		typename Char,
		typename OutputIterator,
		typename OutputSentinel>
	constexpr void _decode_one(
		Char const value,
		OutputIterator& out_beg,
		OutputSentinel const& out_end)
	{
		if (_flush<Byte>(out_beg, out_end))
		{
			_store_encoded(value);
	
			if (m_encoded_size >= 4)
			{
				_decode_internal_to_internal_4();
				_flush<Byte>(out_beg, out_end);
			}
		}
	}

	template<
		typename Byte,
		typename SourceIterator,
		typename SourceSentinel,
		typename OutputIterator,
		typename OutputSentinel>
	constexpr void _decode(
		SourceIterator& beg,
		SourceSentinel const& end,
		OutputIterator& out_beg,
		OutputSentinel const& out_end)
	{
		if (m_encoded_size != 0)
		{
			_store_encoded(*beg);
			if (++beg != end && m_encoded_size < 4)
			{
				_store_encoded(*beg);
				if (++beg == end && m_encoded_size < 4)
				{
					_store_encoded(*beg);
					++beg;
				}
			}

			if (m_encoded_size >= 4)
			{
				_decode_internal_to_internal_4();
			}

			if (!_flush<Byte>(out_beg, out_end))
			{
				return;
			}

			if (beg == end)
			{
				return;
			}
		}

		if constexpr (
			std::sized_sentinel_for<SourceSentinel, SourceIterator> &&
			sized_or_unreachable_sentinel_for<OutputSentinel, OutputIterator>)
		{
			using source_diff_type = std::iter_difference_t<SourceIterator>;
			using output_diff_type = std::iter_difference_t<OutputIterator>;
			using common_diff_type = std::common_type_t<source_diff_type, output_diff_type>;

			source_diff_type const src_diff = end - beg;
			source_diff_type src_diff_div_4 = src_diff / static_cast<source_diff_type>(4);
			source_diff_type src_diff_mod_4 = src_diff % static_cast<source_diff_type>(4);

			// If there is no data after the last full group, it is processed separately after the
			// main 4x loop, in order to handle any padding characters that may be encountered.
			if (src_diff_div_4 != 0 && src_diff_mod_4 == 0)
			{
				src_diff_div_4 -= 1;
				src_diff_mod_4 += 4;
			}

			if constexpr (std::sized_sentinel_for<OutputSentinel, OutputIterator>)
			{
				output_diff_type const out_diff = out_end - out_beg;
				output_diff_type const out_diff_div_3 = out_diff / static_cast<output_diff_type>(3);
				src_diff_div_4 = std::min<common_diff_type>(src_diff_div_4, out_diff_div_3);
			}

			if (src_diff_div_4 != 0)
			{
				detail::base64_decode_4x<Byte>(beg, src_diff_div_4, out_beg);
			}

			if (src_diff_mod_4 >= 4)
			{
				_decode_to_internal_4(beg);

				if (!_flush<Byte>(out_beg, out_end))
				{
					return;
				}

				src_diff_mod_4 -= 4;
			}

			if (src_diff_mod_4 >= 1)
			{
				_store_encoded(*beg);
				++beg;

				if (src_diff_mod_4 >= 2)
				{
					_store_encoded(*beg);
					++beg;

					if (src_diff_mod_4 >= 3)
					{
						_store_encoded(*beg);
						++beg;
					}
				}
			}
		}
		else
		{
			for (; beg != end && out_beg != out_end; ++beg)
			{
				_decode_one<Byte>(*beg, out_beg, out_end);
			}
		}
	}

	template<typename Byte, typename OutputIterator, typename OutputSentinel>
	constexpr bool _flush_one(OutputIterator& out_beg, OutputSentinel const& out_end)
	{
		if (m_decoded_size == 0)
		{
			return false;
		}

		*out_beg = static_cast<Byte>(m_decoded_data[0]);
		++out_beg;

		m_decoded_data[0] = m_decoded_data[1];
		m_decoded_data[1] = m_decoded_data[2];
		m_decoded_size -= 1;

		return true;
	}

	template<typename Byte, typename OutputIterator, typename OutputSentinel>
	constexpr bool _flush(OutputIterator& out_beg, OutputSentinel const& out_end)
	{
		if (out_beg == out_end)
		{
			return false;
		}

		if (_flush_one<Byte>(out_beg, out_end))
		{
			if (out_beg == out_end)
			{
				return false;
			}

			if (_flush_one<Byte>(out_beg, out_end))
			{
				if (out_beg == out_end)
				{
					return false;
				}

				if (_flush_one<Byte>(out_beg, out_end))
				{
					if (out_beg == out_end)
					{
						return false;
					}
				}
			}
		}

		return true;
	}
};

} // namespace detail

using detail::base64_encoder;
using detail::base64_decoder;

} // namespace vsm
