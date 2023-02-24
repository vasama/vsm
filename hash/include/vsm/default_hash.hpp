#pragma once

#include <vsm/hash.hpp>
#include <vsm/platform.h>

#if !vsm_word_32
#	include <vsm/int128.hpp>
#endif

namespace vsm {

class default_hash
{
	using uint_type = uint64_t;

public:
	enum class state_type : uint_type {};

	static state_type initialize(size_t const seed)
	{
		//TODO: FNV1A only.
		return static_cast<state_type>(seed * static_cast<uint_type>(0xcbf29ce484222325));
	}

	static size_t finalize(state_type const s)
	{
		return static_cast<size_t>(static_cast<uint_type>(s));
	}

private:
#if 1
	template<std::integral T>
	friend state_type tag_invoke(decltype(hash_append_bits), state_type const s, T const value)
		requires (sizeof(T) <= sizeof(uint_type))
	{
		return static_cast<state_type>(fnv1a(static_cast<uint_type>(s), reinterpret_cast<std::byte const*>(&value), sizeof(value)));
	}

	friend state_type tag_invoke(decltype(hash_append_bits), state_type const s, std::byte const* const data, size_t const size)
	{
		return static_cast<state_type>(fnv1a(static_cast<uint_type>(s), data, size));
	}
	
	static uint_type fnv1a(uint_type state, std::byte const* const data, size_t const size)
	{
		for (size_t i = 0; i < size; ++i)
		{
			state *= static_cast<uint_type>(0x100000001b3);
			state ^= static_cast<uint8_t>(data[i]);
		}
		return state;
	}
#else
	template<std::integral T>
	friend state_type tag_invoke(decltype(hash_append_bits), state_type const s, T const value)
		requires (sizeof(T) <= sizeof(uint_type))
	{
		return static_cast<state_type>(detail::mix_unsigned_integer<mix>(
			static_cast<uint_type>(s), static_cast<std::make_unsigned_t<T>>(value)));
	}

	friend state_type tag_invoke(decltype(hash_append_bits), state_type const s, std::byte const* const data, size_t const size)
	{
		return static_cast<state_type>(hash_data(static_cast<uint_type>(s), data, size));
	}

	static uint_type mix(uint_type const lhs, uint_type const rhs)
	{
#if vsm_word_32
		using multiply_type = uint64_t;
		static constexpr uint64_t multiplier = 0xcc9e2d51;
#else
		using multiply_type = uint128_t;
		static constexpr uint64_t multiplier = 0x9ddfea08eb382d69;
#endif

		auto const m = static_cast<multiply_type>(lhs + rhs) * multiplier;

#if vsm_word_32
		return static_cast<uint64_t>(m);
#else
		return static_cast<uint64_t>(m) ^ static_cast<uint64_t>(m >> 64);
#endif
	}

	static uint_type hash_data(uint_type seed, std::byte const* data, size_t size);
	static uint_type hash_data(uint_type seed, std::byte const* data, size_t size, uint_type const* salt);
#endif
};
using default_hasher = basic_hasher<default_hash>;

} // namespace vsm
