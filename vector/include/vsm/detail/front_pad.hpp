#pragma once

#include <cstddef>

namespace vsm::detail {

template<bool>
struct _front_pad;

template<>
struct _front_pad<0>
{
	template<typename T, size_t Alignment>
	using type = T;
};

template<>
struct _front_pad<1>
{
	template<size_t Size, size_t Alignment>
	struct base
	{
		unsigned char _[((Size + Alignment - 1) & ~(Alignment - 1)) - Size];
	};

	template<typename T, size_t Alignment>
	struct type : base<sizeof(T), Alignment>, T
	{
		using T::T;
	};
};

template<typename T, size_t Alignment>
using front_pad = _front_pad<alignof(T) < Alignment>::template type<T, Alignment>;

} // namespace vsm::detail
