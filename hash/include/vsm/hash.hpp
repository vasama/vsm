#pragma once

#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>
#include <vsm/type_traits.hpp>

#include <bit>

#include <cstddef>

namespace vsm {
namespace detail {

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
	{
		return tag_invoke(cpo, state, reinterpret_cast<std::byte const*>(&value), sizeof(T));
	}

	template<typename State, typename T>
	vsm_static_operator State operator()(State const state, T const& value) vsm_static_operator_const
		requires tag_invocable<hash_append_bits_cpo, State, T const&>
	{
		return tag_invoke(hash_append_bits_cpo(), state, value);
	}

	template<typename State, typename T>
	vsm_static_operator State operator()(State const state, std::byte const* const data, size_t const size) vsm_static_operator_const
		requires tag_invocable<hash_append_bits_cpo, State, std::byte const*, size_t>
	{
		return tag_invoke(hash_append_bits_cpo(), state, data, size);
	}
};
static constexpr hash_append_bits_cpo hash_append_bits = {};

struct hash_append_cpo
{
	template<typename State>
	friend State tag_invoke(hash_append_cpo, bool const value, State const state)
	{
		hash_append_bits(state, static_cast<unsigned char>(value));
	}

	template<typename State, std::integral T>
	friend State tag_invoke(hash_append_cpo, T const value, State const state)
	{
		return hash_append_bits(state, static_cast<std::make_unsigned_t<T>>(value));
	}

	template<typename State, std::floating_point T>
	friend State tag_invoke(hash_append_cpo, T const value, State const state)
	{
		return hash_append_bits(state, std::bit_cast<unsigned_integer_of_size<sizeof(T)>>(value == 0 ? 0 : value));
	}

	template<typename State>
	friend State tag_invoke(hash_append_cpo, long double, State const state) = delete;

	template<typename State, typename T>
	friend State tag_invoke(hash_append_cpo, T const* const ptr, State state)
	{
		static constexpr size_t shift = std::countr_zero(alignof(T));
		uintptr_t const value = reinterpret_cast<intptr_t>(ptr) >> shift;
		state = hash_append_bits(state, value);
		state = hash_append_bits(state, value);
		return state;
	}

	template<typename State, typename T>
	vsm_static_operator State operator()(T const& value, State const state) vsm_static_operator_const
		requires tag_invocable<hash_append_cpo, T const&, State> || requires { std::hash<T>()(value); }
	{
		if constexpr (tag_invocable<hash_append_cpo, T const&, State>)
		{
			return tag_invoke(hash_append_cpo(), value, state);
		}
		else
		{
			return hash_append_bits(state, std::hash<T>()(value));
		}
	}
};
static constexpr hash_append_cpo hash_append = {};

} // namespace detail

using detail::hash_append_bits;
using detail::hash_append;

inline size_t get_aslr_seed()
{
	return reinterpret_cast<uintptr_t>(get_aslr_seed);
}

template<typename Hash>
struct basic_hasher
{
	template<typename T>
	vsm_static_operator constexpr size_t operator()(T const& value) vsm_static_operator_const
		requires requires { hash_append(value, std::declval<typename Hash::state_type>()); }
	{
		return Hash::finalize(hash_append(value, Hash::initialize(get_aslr_seed())));
	}
};

} // namespace vsm
