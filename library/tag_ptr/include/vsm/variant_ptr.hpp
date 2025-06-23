#pragma once

#include <vsm/tag_ptr.hpp>
#include <vsm/type_list.hpp>

namespace vsm {
namespace detail::_variant_ptr {

template<typename... Ts>
auto _common_type(int) -> std::common_type_t<Ts...>;

template<typename... Ts>
auto _common_type(...) -> std::common_type_t<void*, Ts...>;

template<typename... Ts>
using common_type_t = decltype(_common_type<Ts...>(0));

template<typename T, typename... Ts>
static constexpr bool is_uniquely_convertible_to_v = (std::is_convertible_v<T*, Ts*> + ...) == 1;

template<typename T, typename... Ts, size_t... Indices>
static consteval size_t _uniquely_convertible_to_index(std::index_sequence<Indices...>)
{
	return ((std::is_convertible_v<T*, Ts*> * Indices) + ...);
}

template<typename T, typename... Ts>
static constexpr size_t uniquely_convertible_to_index_v =
	_uniquely_convertible_to_index<T, Ts...>(std::make_index_sequence<sizeof...(Ts)>());

template<typename... Ts>
	requires (sizeof...(Ts) > 0)
class variant_ptr
{
	using common_type = vsm::remove_ptr_t<common_type_t<Ts*...>>;

	vsm::incomplete_tag_ptr<common_type, size_t, sizeof...(Ts) - 1> m_ptr;

public:
	variant_ptr() = default;

	variant_ptr(decltype(nullptr))
		: m_ptr(nullptr)
	{
	}

	template<typename T>
		requires is_uniquely_convertible_to_v<T, Ts...>
	variant_ptr(T* const ptr)
		: m_ptr(ptr, uniquely_convertible_to_index_v<T, Ts...>)
	{
	}

	template<typename T, typename U>
		requires
			is_unique_in_pack_v<T, Ts...> &&
			std::convertible_to<U*, T*>
	explicit variant_ptr(std::in_place_type_t<T>, U* const ptr)
		: m_ptr(ptr, index_in_pack_v<T, Ts...>)
	{
	}

	template<size_t Index, typename T>
		requires
			(Index < sizeof...(Ts)) &&
			std::convertible_to<T*, in_pack_at_t<Index, Ts...>*>
	explicit variant_ptr(std::in_place_index_t<Index>, T* const ptr)
		: m_ptr(ptr, Index)
	{
	}


	[[nodiscard]] size_t index() const noexcept
	{
		return m_ptr.tag();
	}

	[[nodiscard]] common_type* get() const noexcept
	{
		return m_ptr.ptr();
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return static_cast<bool>(m_ptr);
	}
};

template<typename... Ts>
consteval bool check()
{
	return (detail::tag_ptr_check_tag<Ts>(sizeof...(Ts) - 1) && ...);
}

template<typename T, typename... Ts>
[[nodiscard]] bool holds_alternative(variant_ptr<Ts...> const& ptr)
{
	return ptr.index() == index_in_pack_v<T, Ts...>;
}

template<typename T, typename... Ts>
[[nodiscard]] T* get_if(variant_ptr<Ts...> const& ptr)
{
	return ptr.index() == index_in_pack_v<T, Ts...>
		? nullptr
		: const_cast<T*>(static_cast<T const*>(ptr.get()));
}

} // namespace detail::_variant_ptr

template<typename... Ts>
using incomplete_variant_ptr = detail::_variant_ptr::variant_ptr<Ts...>;

template<typename... Ts>
	requires (sizeof...(Ts) > 0) && (detail::_variant_ptr::check<Ts...>())
using variant_ptr = detail::_variant_ptr::variant_ptr<Ts...>;

using detail::_variant_ptr::holds_alternative;
using detail::_variant_ptr::get_if;

} // namespace vsm
