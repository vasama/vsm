static __m128i ascii_transform_k(
	__m128i const vsrc,
	__m128i const vsub,
	__m128i const vadd,
	__m128i const vmax)
{
	__m128i vcmp;
	vcmp = _mm_sub_epi8(vsrc, vsub);
	vcmp = _mm_max_epu8(vcmp, vmax);
	vcmp = _mm_cmpeq_epi8(vcmp, vmax);
	vcmp = _mm_and_si128(vcmp, vadd);
	vcmp = _mm_add_epi8(vsrc, vcmp);
	return vcmp;
}

static void ascii_transform_n(
	char const* const src_beg,
	char const* const src_end,
	char* const out_beg,
	unsigned char const sub,
	unsigned char const add,
	unsigned char const max)
{
	__m128i const vsub = _mm_set1_epi8(sub);
	__m128i const vadd = _mm_set1_epi8(add);
	__m128i const vmax = _mm_set1_epi8(max);

	size_t const size = static_cast<size_t>(src_end - src_beg);

	size_t index = 0;
	size_t const limit = size - size % 16;

	for (; index < limit; index += 16)
	{
		__m128i const vsrc = _mm_loadu_si128(reinterpret_cast<__m128i const*>(src_beg + index));
		__m128i const vout = ascii_transform_k(vsrc, vsub, vadd, vmax);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(out_beg + index), vout);
	}

	for (; index < size; ++index)
	{
		out_beg[index] = ascii_transform_1(src_beg[index], sub, add, max);
	}
}

static int ascii_compare(
	char const* const lhs,
	char const* const rhs,
	size_t const size)
{
	static constexpr auto sub = sub_to_lower;
	static constexpr auto add = add_to_lower;

	__m128i const vsub = _mm_set1_epi8(sub);
	__m128i const vadd = _mm_set1_epi8(sub);
	__m128i const vmax = _mm_set1_epi8(max);

	size_t index = 0;
	size_t const limit = size - size % 16;

	for (; index < limit; index += 16)
	{
		__m128i const vlhs_1 = _mm_loadu_si128(reinterpret_cast<__m128i const*>(lhs + index));
		__m128i const vrhs_1 = _mm_loadu_si128(reinterpret_cast<__m128i const*>(rhs + index));

		__m128i const vlhs_2 = ascii_transform_k(vlhs_1, vsub, vadd, vmax);
		__m128i const vrhs_2 = ascii_transform_k(vrhs_1, vsub, vadd, vmax);

		__m128i const vcmp = _mm_cmpeq_epi8(vlhs_2, vrhs_2);
		auto const mask = static_cast<uint16_t>(_mm_movemask_epi8(vcmp));

		if (mask != 0xFFFF)
		{
			size_t const mismatch = index + std::countl_one(mask);

			auto const lhs_1 = static_cast<unsigned char>(lhs[mismatch]);
			auto const rhs_1 = static_cast<unsigned char>(rhs[mismatch]);

			return static_cast<int>(rhs_1) - static_cast<int>(lhs_1);
		}
	}

	for (; index < size; ++index)
	{
		auto const lhs_1 = static_cast<unsigned char>(lhs[index]);
		auto const rhs_1 = static_cast<unsigned char>(rhs[index]);

		auto const lhs_2 = ascii_transform_1(lhs_1, sub, add, max);
		auto const rhs_2 = ascii_transform_1(rhs_1, sub, add, max);

		if (lhs_2 != rhs_2)
		{
			return static_cast<int>(rhs_2) - static_cast<int>(lhs_2);
		}
	}

	return 0;
}
