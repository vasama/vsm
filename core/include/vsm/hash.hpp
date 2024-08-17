#pragma once

#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>
#include <vsm/type_traits.hpp>

#include <bit>

#include <cstddef>

namespace vsm {
namespace detail {

template<typename T>
inline constexpr bool is_trivially_hashable_v = false;

template<std::integral T>
inline constexpr bool is_trivially_hashable_v<T> = true;

template<auto Mix, std::unsigned_integral T>
T mix_unsigned_integer(T state, std::unsigned_integral auto value)
{
	if constexpr (sizeof(value) <= sizeof(state))
	{
		state = Mix(state, static_cast<T>(value));
	}
	else
	{
		static constexpr size_t size_ratio = sizeof(value) / sizeof(state);
		static constexpr size_t state_bits = sizeof(state) * CHAR_BIT;
		
		for (size_t i = 0; i < size_ratio; ++i)
		{
			state = Mix(state, static_cast<T>(value));
			value >>= state_bits;
		}
	}
	return state;
}

struct hash_append_bits_cpo
{
	template<typename State, typename T>
	friend State tag_invoke(hash_append_bits_cpo cpo, State const state, T const& value)
		requires is_trivially_hashable_v<T>
	{
		return tag_invoke(cpo, state, reinterpret_cast<std::byte const*>(&value), sizeof(T));
	}

	template<typename State, typename T>
	vsm_static_operator State operator()(
		State const state,
		T const& value) vsm_static_operator_const
		requires tag_invocable<hash_append_bits_cpo, State, T const&>
	{
		return tag_invoke(hash_append_bits_cpo(), state, value);
	}

	template<typename State>
	vsm_static_operator State operator()(
		State const state,
		std::byte const* const data,
		size_t const size) vsm_static_operator_const
		requires tag_invocable<hash_append_bits_cpo, State, std::byte const*, size_t>
	{
		return tag_invoke(hash_append_bits_cpo(), state, data, size);
	}
};
static constexpr hash_append_bits_cpo hash_append_bits = {};

struct hash_append_cpo;

template<typename State, typename T>
concept _hash_append_concept =
	tag_invocable<hash_append_cpo, State, T const&> ||
	is_trivially_hashable_v<T> ||
	requires (T const& value) { std::hash<T>()(value); };

struct hash_append_cpo
{
	template<typename State>
	friend State tag_invoke(hash_append_cpo, State const state, bool const value)
	{
		hash_append_bits(state, static_cast<unsigned char>(value));
	}

	template<typename State, std::integral T>
	friend State tag_invoke(hash_append_cpo, State const state, T const value)
	{
		return hash_append_bits(state, static_cast<std::make_unsigned_t<T>>(value));
	}

	template<typename State, std::floating_point T>
	friend State tag_invoke(hash_append_cpo, State const state, T const value)
	{
		return hash_append_bits(
			state,
			std::bit_cast<unsigned_integer_of_size<sizeof(T)>>(value == 0 ? 0 : value));
	}

	template<typename State>
	friend State tag_invoke(hash_append_cpo, State const state, long double) = delete;

	template<typename State>
	friend State tag_invoke(hash_append_cpo, State state, void const* const ptr)
	{
		auto const value = reinterpret_cast<uintptr_t>(ptr);
		state = hash_append_bits(state, value);
		state = hash_append_bits(state, value);
		return state;
	}

	template<typename State, non_cvref T>
		requires _hash_append_concept<State, T>
	vsm_static_operator State operator()(
		State const state,
		T const& value) vsm_static_operator_const
	{
		if constexpr (tag_invocable<hash_append_cpo, State, T const&>)
		{
			return tag_invoke(hash_append_cpo(), state, value);
		}
		else if constexpr (is_trivially_hashable_v<T>)
		{
			return hash_append_bits(state, value);
		}
		else
		{
			return hash_append_bits(state, std::hash<T>()(value));
		}
	}

	template<typename State, std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
		requires _hash_append_concept<State, std::iter_value_t<Iterator>>
	vsm_static_operator State operator()(
		State state,
		Iterator begin,
		Sentinel end) vsm_static_operator_const
	{
		using value_type = std::iter_value_t<Iterator>;

		if constexpr (tag_invocable<hash_append_cpo, State, Iterator, Sentinel>)
		{
			return tag_invoke(hash_append_cpo(), vsm_move(state), vsm_move(begin), vsm_move(end));
		}
		else if constexpr (
			is_trivially_hashable_v<value_type> &&
			std::contiguous_iterator<Iterator> &&
			std::sized_sentinel_for<Sentinel, Iterator>)
		{
			vsm::any_cv_of<value_type> auto* const ptr = std::to_address(begin);

			return hash_append_bits(
				state,
				reinterpret_cast<std::byte const*>(ptr),
				static_cast<size_t>(end - begin) * sizeof(value_type));
		}
		else
		{
			for (; begin != end; ++begin)
			{
				state = operator()(vsm_move(state), *begin);
			}

			return state;
		}
	}
};
static constexpr hash_append_cpo hash_append = {};

//TODO: Add conditional DLL export
extern void const* const aslr_seed;

} // namespace detail

using detail::is_trivially_hashable_v;

using detail::hash_append_bits;
using detail::hash_append;

inline size_t get_aslr_seed()
{
	return static_cast<size_t>(reinterpret_cast<uintptr_t>(&detail::aslr_seed));
}

template<typename Hash>
struct basic_hasher
{
	template<typename T>
	vsm_static_operator constexpr size_t operator()(T const& value) vsm_static_operator_const
		requires requires { hash_append(std::declval<typename Hash::state_type>(), value); }
	{
		return Hash::finalize(hash_append(Hash::initialize(get_aslr_seed()), value));
	}
};

} // namespace vsm
