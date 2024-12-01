static void ascii_transform_n(
	char const* src_beg,
	char const* const src_end,
	char* out_beg,
	unsigned char const sub,
	unsigned char const add,
	unsigned char const max)
{
	for (; src_beg != src_end; ++src_beg, ++out_beg)
	{
		auto const src = static_cast<unsigned char>(*src_beg);

		*out_beg = src - sub > max
			? src
			: src + add;
	}
}

static int ascii_compare(
	char const* const lhs,
	char const* const rhs,
	size_t const size)
{
	static constexpr auto sub = sub_to_lower;
	static constexpr auto add = add_to_lower;

	for (size_t i = 0; i < size; ++i)
	{
		auto const lhs_1 = static_cast<unsigned char>(lhs[i]);
		auto const rhs_1 = static_cast<unsigned char>(rhs[i]);

		auto const lhs_2 = ascii_transform_1(lhs_1, sub, add, max);
		auto const rhs_2 = ascii_transform_1(rhs_1, sub, add, max);

		if (lhs_2 != rhs_2)
		{
			return static_cast<int>(rhs_2) - static_cast<int>(lhs_2);
		}
	}

	return 0;
}
