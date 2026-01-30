#pragma once

#include <functional>

namespace vsm {

struct
{
	template<typename T>
	constexpr std::remove_reference_t<T>* operator()(T&& reference) const noexcept
	{
		return std::addressof(reference);
	}
}
inline constexpr addressof;

} // namespace vsm
