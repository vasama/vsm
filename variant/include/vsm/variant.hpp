#pragma once

#include <algorithm>

namespace vsm {
namespace detail {

template<size_t N>
struct _variant1
{
	size_t m_index;
};

template<typename... Ts>
using variant1 = _variant1<sizeof...(Ts)>;

template<size_t Size, size_t Alignment>
struct _variant2
{
	alignas(Alignment) unsigned char m_storage[Size];
};

template<typename... Ts>
using variant2 = _variant2<std::max({sizeof(Ts)...}), std::max({alignof(Ts)...})>;

} // namespace detail

template<typename... Ts>
class variant
	: detail::variant1<Ts...>
	, detail::variant2<Ts...>
{
	using variant2 = detail::variant2<Ts...>;

public:
	constexpr 

	template<typename T>
	[[nodiscard]] variant& from(T& object)
	{
		return static_cast<variant&>(*reinterpret_cast<variant2*>(
			std::launder(reinterpret_cast<decltype(variant2::m_storage)*>(std::addressof(object)))));
	}

	template<typename T>
	[[nodiscard]] variant const& from(T const& object)
	{
		return static_cast<variant const&>(*reinterpret_cast<variant2 const*>(
			std::launder(reinterpret_cast<decltype(variant2::m_storage) const*>(std::addressof(object)))));
	}
};

} // namespace vsm
