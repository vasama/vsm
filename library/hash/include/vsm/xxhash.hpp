#pragma once

#include <vsm/hash.hpp>
#include <vsm/preprocessor.h>

//TODO: This should be defined by the xxhash package.
#define XXH_STATIC_LINKING_ONLY

#include <xxhash.h>

namespace vsm {

#ifdef __INTELLISENSE__
#	define vsm_detail_xxhash(name) XXH64_ ## name
#else
#	define vsm_detail_xxhash(name) vsm_pp_cat(XXH, vsm_pp_cat(vsm_word_bits, vsm_pp_cat(_, name)))
#endif

static_assert(sizeof(vsm_detail_xxhash(hash_t)) == sizeof(size_t));

class xxhash
{
public:
	struct state_type : vsm_detail_xxhash(state_t) {};

	static state_type initialize(size_t const seed)
	{
		state_type state;
		vsm_detail_xxhash(reset)(&state, seed);
		return state;
	}

	static size_t finalize(state_type const& state)
	{
		return vsm_detail_xxhash(digest)(&state);
	}

private:
	vsm_always_inline friend void tag_invoke(
		decltype(hash_append_bits),
		state_type& state,
		void const* const data,
		size_t const size)
	{
		vsm_detail_xxhash(update)(&state, data, size);
	}
};

#undef vsm_detail_xxhash

} // namespace vsm
