#if 0
#include <vsm/default_hash.hpp>

#include <algorithm>

using namespace vsm;

template<typename T>
static T loadu(void const* const ptr)
{
	T value;
	memcpy(&value, ptr, sizeof(T));
	return value;
}

uint64_t default_hash::hash_data(uint64_t const seed, std::byte const* const data, size_t const size)
{
	static constexpr uint64_t salt[] =
	{
		0x243F6A8885A308D3,
		0x13198A2E03707344,
		0xA4093822299F31D0,
		0x082EFA98EC4E6C89,
		0x452821E638D01377,
	};

	return hash_data(seed, data, size, salt);
}

uint64_t default_hash::hash_data(uint64_t const seed, std::byte const* data, size_t size, uint64_t const* const salt)
{
	size_t const full_size = size;

	uint64_t s0 = seed ^ salt[0];

	if (size >= 64)
	{
		uint64_t s1 = s0;

		do
		{
			uint64_t const _0 = loadu<uint64_t>(data + 000);
			uint64_t const _1 = loadu<uint64_t>(data + 010);
			uint64_t const _2 = loadu<uint64_t>(data + 020);
			uint64_t const _3 = loadu<uint64_t>(data + 030);
			uint64_t const _4 = loadu<uint64_t>(data + 040);
			uint64_t const _5 = loadu<uint64_t>(data + 050);
			uint64_t const _6 = loadu<uint64_t>(data + 060);
			uint64_t const _7 = loadu<uint64_t>(data + 070);
	
			s0 = mix(_0 ^ salt[1], _1 ^ s0) ^ mix(_2 ^ salt[2], _3 ^ s0);
			s1 = mix(_4 ^ salt[3], _5 ^ s1) ^ mix(_6 ^ salt[4], _7 ^ s1);
	
			data += 64;
			size -= 64;
		}
		while (size >= 64);
		
		s0 ^= s1;
	}

	size_t i = 0;

	while (size >= 16)
	{
		uint64_t const _0 = loadu<uint64_t>(data + 000);
		uint64_t const _1 = loadu<uint64_t>(data + 010);

		s0 = mix(_0 ^ salt[1], _1 ^ s0);
		
		data += 16;
		size -= 16;
	}

	if (size >= 1)
	{
		uint64_t _0 = 0;
		uint64_t _1 = 0;

		if (size & 8)
		{
			_1 = loadu<uint64_t>(data + (size & 7));
		}

		if (size & 4)
		{
			_0 = _0 << 32 | loadu<uint32_t>(data + (size & 3));
		}

		if (size & 2)
		{
			_0 = _0 << 16 | loadu<uint16_t>(data + (size & 1));
		}

		if (size & 1)
		{
			_0 = _0 << 8 | loadu<uint8_t>(data);
		}

		s0 = mix(_0 ^ salt[1], _1 ^ s0);
	}

	return s0;
}
#endif
