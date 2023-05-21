#pragma once

#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>
#include <vsm/utility.hpp>

#include <type_traits>

namespace vsm::compilers {

struct is_a_cpo
{
	template<typename To, typename From>
	constexpr friend bool tag_invoke(is_a_cpo, tag<To>, From const& from)
		requires std::is_base_of_v<To, From>
	{
		return true;
	}
};

template<typename To, typename From>
constexpr bool is_a(From const& from)
	requires std::is_base_of_v<To, From> || std::is_base_of_v<From, To>
{
	return vsm::tag_invoke(is_a_cpo(), tag<To>(), from);
}


struct cast_cpo
{
	template<typename To, typename From>
	constexpr friend To* tag_invoke(cast_cpo, tag<To*>, From* const from)
	{
		return from != nullptr && is_a<To>(*from) ? static_cast<To*>(from) : nullptr;
	}
};

template<typename To, typename From>
constexpr To cast(From&& from)
{
	return vsm::tag_invoke(cast_cpo(), tag<To>(), vsm_forward(from));
}

} // namespace vsm::compilers
