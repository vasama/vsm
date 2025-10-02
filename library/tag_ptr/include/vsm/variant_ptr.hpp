#pragma once

#include <vsm/concepts.hpp>
#include <vsm/tag_ptr.hpp>

namespace vsm {
namespace detail::_variant_ptr {

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

template<typename Base, typename... Ts>
class basic_variant_ptr
{
	static_assert(sizeof...(Ts) != 0);

	using base_type = remove_ptr_t<std::common_type_t<copy_cv_t<Ts, Base>*...>>;

	vsm::incomplete_tag_ptr<Base, size_t, sizeof...(Ts) - 1> m_ptr;

public:
	basic_variant_ptr() = default;

	basic_variant_ptr(decltype(nullptr))
		: m_ptr(nullptr)
	{
	}

	template<typename T>
		requires is_uniquely_convertible_to_v<T, Ts...>
	basic_variant_ptr(T* const ptr)
		: m_ptr(ptr, uniquely_convertible_to_index_v<T, Ts...>)
	{
	}

	template<typename T, typename U>
		requires
			is_unique_in_pack_v<T, Ts...> &&
			std::convertible_to<U*, T*>
	explicit basic_variant_ptr(std::in_place_type_t<T>, U* const ptr)
		: m_ptr(ptr, index_in_pack_v<T, Ts...>)
	{
	}

	template<size_t Index, typename T>
		requires
			(Index < sizeof...(Ts)) &&
			std::convertible_to<T*, in_pack_at_t<Index, Ts...>*>
	explicit basic_variant_ptr(std::in_place_index_t<Index>, T* const ptr)
		: m_ptr(ptr, Index)
	{
	}


	[[nodiscard]] size_t index() const noexcept
	{
		return m_ptr.tag();
	}

	[[nodiscard]] base_type* get() const noexcept
	{
		return m_ptr.ptr();
	}

	template<typename T = base_type>
	[[nodiscard]] T& operator*() const noexcept
		requires vsm::not_same_as<Base, void>
	{
		return *m_ptr.ptr();
	}

	template<typename T = base_type>
	[[nodiscard]] T* operator->() const noexcept
		requires vsm::not_same_as<Base, void>
	{
		return m_ptr.ptr();
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return static_cast<bool>(m_ptr);
	}

	[[nodiscard]] friend bool operator==(basic_variant_ptr const& ptr, decltype(nullptr))
	{
		return ptr.m_ptr.is_zero();
	}

	[[nodiscard]] friend bool operator==(basic_variant_ptr const&, basic_variant_ptr const&) = default;
};

template<typename Base, typename... Ts>
consteval bool check()
{
	return
		requires { typename std::common_type_t<Base*, Ts*...>; } &&
		(detail::tag_ptr_check_tag<Ts>(sizeof...(Ts) - 1) && ...);
}

template<typename T, typename Base, typename... Ts>
	requires is_unique_in_pack_v<T, Ts...>
[[nodiscard]] bool holds_alternative(basic_variant_ptr<Base, Ts...> const& ptr)
{
	return ptr != nullptr && ptr.index() == index_in_pack_v<T, Ts...>;
}

template<typename T, typename Base, typename... Ts>
	requires is_unique_in_pack_v<T, Ts...>
[[nodiscard]] T* get_if(basic_variant_ptr<Base, Ts...> const& ptr)
{
	return ptr.index() != index_in_pack_v<T, Ts...>
		? nullptr
		: const_cast<T*>(static_cast<T const*>(ptr.get()));
}

} // namespace detail::_variant_ptr

template<vsm::non_cvref Base, vsm::non_ref... Ts>
	requires (sizeof...(Ts) > 0)
using basic_incomplete_variant_ptr = detail::_variant_ptr::basic_variant_ptr<Base, Ts...>;

template<vsm::non_ref... Ts>
	requires (sizeof...(Ts) > 0)
using incomplete_variant_ptr = detail::_variant_ptr::basic_variant_ptr<void, Ts...>;

template<vsm::non_cvref Base, vsm::non_ref... Ts>
	requires (sizeof...(Ts) > 0) && (detail::_variant_ptr::check<Base, Ts...>())
using basic_variant_ptr = detail::_variant_ptr::basic_variant_ptr<Base, Ts...>;

template<vsm::non_ref... Ts>
	requires (sizeof...(Ts) > 0) && (detail::_variant_ptr::check<void, Ts...>())
using variant_ptr = detail::_variant_ptr::basic_variant_ptr<void, Ts...>;

using detail::_variant_ptr::holds_alternative;
using detail::_variant_ptr::get_if;

} // namespace vsm
