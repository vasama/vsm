#pragma once

#include <vsm/hash.hpp>

#include <cstddef>
#include <cstdint>

namespace vsm {

inline constexpr void umul128(uint64_t& lhs, uint64_t& rhs);

inline constexpr uint64_t wy_hash_secret[] =
{
	0xa0761d6478bd642full,
	0xe7037ed1a0b428dbull,
	0x8ebc6af09c88c6e3ull,
	0x589965cc75374cc3ull,
};

inline void wy_hash_multiply(uint64_t& lhs, uint64_t& rhs)
{
	uint64_t a = lhs;
	uint64_t b = rhs;
	umul128(a, b);
	lhs ^= a;
	rhs ^= b;
}

inline uint64_t wy_hash_mix(uint64_t lhs, uint64_t rhs)
{
	wy_hash_multiply(lhs, rhs);
	return lhs ^ rhs;
}



struct wy_hash
{
	enum class state : uint64_t {};

	static state initialize(size_t const seed)
	{
		return static_cast<state>(seed);
	}

	static size_t finalize(state const s)
	{
		return static_cast<size_t>(static_cast<uint64_t>(s));
	}

	friend state tag_invoke(decltype(hash_append_bits), std::integral auto const value, state const s)
	{
		
	}
};

} // namespace vsm
