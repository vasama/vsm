#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/defer.hpp>
#include <vsm/exceptions.hpp>
#include <vsm/tag_invoke.hpp>

#include <memory>
#include <new>

#include <cstring>

namespace vsm {
namespace detail {

template<typename T>
concept _nothrow_boolean_testable = std::convertible_to<T, bool>;

template<typename T>
concept nothrow_boolean_testable =
	_nothrow_boolean_testable<T> &&
	requires
	{
		{ !std::declval<T>() } -> _nothrow_boolean_testable;
	};

template<typename T, typename U>
concept nothrow_equality_comparable_with =
	std::equality_comparable_with<T, U> &&
	requires (remove_ref_t<T> const& t, remove_ref_t<U> const& u)
	{
		{ t == u } noexcept -> nothrow_boolean_testable;
		{ t != u } noexcept -> nothrow_boolean_testable;
		{ u == t } noexcept -> nothrow_boolean_testable;
		{ u != t } noexcept -> nothrow_boolean_testable;
	};

template<typename T>
concept nothrow_equality_comparable =
	std::equality_comparable<T> &&
	nothrow_equality_comparable_with<T, T>;

template<typename T, typename U>
concept nothrow_totally_ordered_with =
	std::totally_ordered_with<T, U> &&
	nothrow_equality_comparable_with<T, U> &&
	requires (remove_ref_t<T> const& t, remove_ref_t<U> const& u)
	{
		{ t < u } noexcept -> nothrow_boolean_testable;
		{ t > u } noexcept -> nothrow_boolean_testable;
		{ t <= u } noexcept -> nothrow_boolean_testable;
		{ t >= u } noexcept -> nothrow_boolean_testable;
		{ u < t } noexcept -> nothrow_boolean_testable;
		{ u > t } noexcept -> nothrow_boolean_testable;
		{ u <= t } noexcept -> nothrow_boolean_testable;
		{ u >= t } noexcept -> nothrow_boolean_testable;
	};

template<typename T>
concept nothrow_totally_ordered =
	std::totally_ordered<T> &&
	nothrow_totally_ordered_with<T, T>;

template<typename Iterator>
concept nothrow_input_iterator =
	std::input_iterator<Iterator> &&
	std::is_nothrow_move_constructible_v<Iterator> &&
	std::is_nothrow_copy_constructible_v<Iterator> &&
	std::is_nothrow_move_assignable_v<Iterator> &&
	std::is_nothrow_copy_assignable_v<Iterator> &&
	requires (Iterator& i, Iterator const& j)
	{
		{ ++i } noexcept;
		{ i++ } noexcept;
		{ *j } noexcept;
	};

template<typename Sentinel, typename Iterator>
concept nothrow_sentinel_for =
	std::sentinel_for<Sentinel, Iterator> &&
	nothrow_equality_comparable_with<Sentinel, Iterator>;

template<typename Iterator>
concept nothrow_forward_iterator =
	std::forward_iterator<Iterator> &&
	nothrow_sentinel_for<Iterator, Iterator>;

template<typename Iterator>
concept nothrow_bidirectional_iterator =
	std::bidirectional_iterator<Iterator> &&
	nothrow_forward_iterator<Iterator> &&
	requires (Iterator& x)
	{
		{ --x } noexcept;
		{ x-- } noexcept;
	};

template<typename Sentinel, typename Iterator>
concept nothrow_sized_sentinel_for =
	std::sized_sentinel_for<Sentinel, Iterator> &&
	nothrow_sentinel_for<Sentinel, Iterator> &&
	requires (Sentinel const& s, Iterator const& i)
	{
		{ s - i } noexcept;
		{ i - s } noexcept;
	};

template<typename Iterator>
concept nothrow_random_access_iterator =
	std::random_access_iterator<Iterator> &&
	nothrow_bidirectional_iterator<Iterator> &&
	nothrow_sized_sentinel_for<Iterator, Iterator> &&
	nothrow_totally_ordered<Iterator> &&
	requires (Iterator& i, Iterator const& j, std::iter_difference_t<Iterator> const& n)
	{
		{ i += n } noexcept;
		{ j + n } noexcept;
		{ n + j } noexcept;
		{ i -= n } noexcept;
		{ j - n } noexcept;
		{ j[n] } noexcept;
	};

template<typename Iterator>
concept nothrow_contiguous_iterator =
	std::contiguous_iterator<Iterator> &&
	nothrow_random_access_iterator<Iterator>;

} // namespace detail


template<typename T>
inline constexpr bool is_trivially_relocatable_v = std::is_trivially_copyable_v<T>;

template<typename T>
inline constexpr bool is_nothrow_relocatable_v =
	is_trivially_relocatable_v<T> || std::is_nothrow_move_constructible_v<T>;


// P1144
template<typename T>
[[nodiscard]] remove_cv_t<T> relocate(T* const source)
{
	remove_cv_t<T> r = vsm_move(*source);
	std::destroy_at(source);
	return r;
}

// P1144
template<typename T>
T* relocate_at(T* const source, T* const destination)
{
	if constexpr (is_trivially_relocatable_v<T>)
	{
		std::memcpy(destination, source, sizeof(T));
		return destination;
	}
	else
	{
		vsm_defer { std::destroy_at(source); };
		return ::new(static_cast<void*>(destination)) T(vsm_move(*source));
	}
}

template<
	detail::nothrow_input_iterator SourceIterator,
	detail::nothrow_sentinel_for<SourceIterator> SourceSentinel,
	detail::nothrow_forward_iterator OutputIterator>
OutputIterator uninitialized_relocate(
	SourceIterator src_beg,
	SourceSentinel const src_end,
	OutputIterator const out_beg)
{
	using source_type = std::iterator_traits<SourceIterator>::value_type;
	using output_type = std::iterator_traits<OutputIterator>::value_type;

	static constexpr bool use_trivial_copy =
		std::is_same_v<source_type, output_type>
		&& is_trivially_relocatable_v<output_type>;

	SourceIterator const& c_src_beg = src_beg;

	if constexpr (
		use_trivial_copy
		&& detail::nothrow_contiguous_iterator<SourceIterator>
		&& detail::nothrow_sized_sentinel_for<SourceSentinel, SourceIterator>
		&& detail::nothrow_contiguous_iterator<OutputIterator>)
	{
		if (c_src_beg != src_end)
		{
			std::integral auto const n = src_end - c_src_beg;

			using integral_type = std::remove_const_t<decltype(n)>;
			using unsigned_type = std::make_unsigned_t<integral_type>;

			if constexpr (std::is_signed_v<integral_type>)
			{
				vsm_assert(n >= static_cast<integral_type>(0));
			}

			std::memmove(
				std::to_address(out_beg),
				std::addressof(*c_src_beg),
				static_cast<unsigned_type>(n) * sizeof(output_type));

			return out_beg + n;
		}
		else
		{
			return out_beg;
		}
	}
	else
	{
		OutputIterator out_pos = out_beg;
		OutputIterator const& c_out_pos = out_pos;

		vsm_except_try
		{
			vsm_clang_diagnostic(push)
			vsm_clang_diagnostic(ignored "-Wfor-loop-analysis")
			for (; c_src_beg != src_end; ++src_beg, (void)++out_pos)
			{
				if constexpr (use_trivial_copy)
				{
					std::memcpy(
						std::to_address(c_out_pos),
						std::addressof(*c_src_beg),
						sizeof(output_type));
				}
				else
				{
					source_type* const p = std::addressof(*c_src_beg);
					::new (static_cast<void*>(std::to_address(c_out_pos))) output_type(vsm_move(*p));
					std::destroy_at(p);
				}
			}
			vsm_clang_diagnostic(pop)
			return out_pos;
		}
		vsm_except_catch(...)
		{
			std::destroy(out_beg, c_out_pos);
			++src_beg;
			std::destroy(c_src_beg, src_end);
			vsm_except_rethrow;
		}
	}
}

template<
	detail::nothrow_input_iterator SourceIterator,
	std::integral SizeT,
	detail::nothrow_forward_iterator OutputIterator>
std::pair<SourceIterator, OutputIterator> uninitialized_relocate_n(
	SourceIterator src_beg,
	SizeT n,
	OutputIterator const out_beg)
{
	if constexpr (std::is_signed_v<SizeT>)
	{
		vsm_assert(n >= static_cast<SizeT>(0));
	}

	using source_type = std::iterator_traits<SourceIterator>::value_type;
	using output_type = std::iterator_traits<OutputIterator>::value_type;

	static constexpr bool use_trivial_copy =
		std::is_same_v<source_type, output_type>
		&& is_trivially_relocatable_v<output_type>;

	SourceIterator const& c_src_beg = src_beg;
	SizeT const& c_n = n;

	if constexpr (
		use_trivial_copy
		&& detail::nothrow_contiguous_iterator<SourceIterator>
		&& detail::nothrow_contiguous_iterator<OutputIterator>)
	{
		if (c_n != 0)
		{
			using unsigned_type = std::make_unsigned_t<SizeT>;

			std::memmove(
				std::to_address(out_beg),
				std::addressof(*c_src_beg),
				static_cast<unsigned_type>(c_n) * sizeof(output_type));
		}

		return { src_beg + c_n, out_beg + c_n };
	}
	else
	{
		OutputIterator out_pos = out_beg;
		OutputIterator const& c_out_pos = out_pos;

		vsm_except_try
		{
			vsm_clang_diagnostic(push)
			vsm_clang_diagnostic(ignored "-Wfor-loop-analysis")
			for (; c_n > 0; --n, (void)++out_pos)
			{
				if constexpr (use_trivial_copy)
				{
					std::memcpy(
						std::to_address(c_out_pos),
						std::addressof(*c_src_beg),
						sizeof(output_type));
				}
				else
				{
					source_type* const p = std::addressof(*c_src_beg);
					::new (static_cast<void*>(std::to_address(c_out_pos))) output_type(vsm_move(*p));
					std::destroy_at(p);
				}
			}
			vsm_clang_diagnostic(pop)
			return { c_src_beg, c_out_pos };
		}
		vsm_except_catch(...)
		{
			std::destroy(out_beg, c_out_pos);
			++src_beg;
			--n;
			std::destroy_n(c_src_beg, c_n);
			vsm_except_rethrow;
		}
	}
}

template<
	detail::nothrow_bidirectional_iterator SourceIterator,
	detail::nothrow_bidirectional_iterator OutputIterator>
OutputIterator uninitialized_relocate_backward(
	SourceIterator const src_beg,
	SourceIterator src_end,
	OutputIterator const out_end)
{
	using source_type = std::iterator_traits<SourceIterator>::value_type;
	using output_type = std::iterator_traits<OutputIterator>::value_type;

	static constexpr bool use_trivial_copy =
		std::is_same_v<source_type, output_type>
		&& is_trivially_relocatable_v<output_type>;

	SourceIterator const& c_src_end = src_end;

	if constexpr (
		use_trivial_copy
		&& detail::nothrow_contiguous_iterator<SourceIterator>
		&& detail::nothrow_contiguous_iterator<OutputIterator>)
	{
		if (src_beg != c_src_end)
		{
			std::integral auto const n = c_src_end - src_beg;

			using integral_type = std::remove_const_t<decltype(n)>;
			using unsigned_type = std::make_unsigned_t<integral_type>;

			if constexpr (std::is_signed_v<integral_type>)
			{
				vsm_assert(n >= static_cast<integral_type>(0));
			}

			std::memmove(
				std::to_address(out_end) - n,
				std::addressof(*src_beg),
				static_cast<unsigned_type>(n) * sizeof(output_type));

			return out_end - n;
		}
		else
		{
			return out_end;
		}
	}
	else
	{
		OutputIterator out_pos = out_end;
		OutputIterator const& c_out_pos = out_pos;

		vsm_except_try
		{
			while (c_src_end != src_beg)
			{
				--src_end;
				--out_pos;

				if constexpr (use_trivial_copy)
				{
					std::memcpy(
						std::to_address(c_out_pos),
						std::addressof(*c_src_end),
						sizeof(output_type));
				}
				else
				{
					source_type* const p = std::addressof(*c_src_end);
					::new (static_cast<void*>(std::to_address(c_out_pos))) output_type(vsm_move(*p));
					std::destroy_at(p);
				}
			}
			return out_pos;
		}
		vsm_except_catch(...)
		{
			++out_pos;
			std::destroy(c_out_pos, out_end);
			++src_end;
			std::destroy(src_beg, c_src_end);
			vsm_except_rethrow;
		}
	}
}

} // namespace vsm
