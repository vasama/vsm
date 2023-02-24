#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

#include <memory>
#include <type_traits>

#define vsm_value_categories(X) \
	X(&) \
	X(const&) \
	X(&&) \
	X(const&&) \

#define vsm_move_copy_value_categories(X) \
	X(&&) \
	X(const&) \

namespace vsm {
namespace detail::optional_ {

enum class bool_flag {};

template<typename T, auto Sentinel = bool_flag{}>
class optional;


struct disengaged {};

template<typename T, auto Sentinel>
struct storage
{
	static_assert(std::is_trivially_copyable_v<T>);
	static_assert(std::is_same_v<decltype(Sentinel), T>);

	T m_value;

	storage() = default;

	constexpr storage(auto&&... args)
		noexcept(noexcept(T(vsm_forward(args)...)))
		: m_value(vsm_forward(args)...)
	{
	}

	constexpr bool has_value() const noexcept
	{
		return m_value
	}

	constexpr T& emplace(auto&&... args)
		noexcept(noexcept(T(vsm_forward(args)...)))
	{
		m_value = T(vsm_forward(args)...);
		vsm_assert(m_value != Sentinel);
		return m_value;
	}

	constexpr void reset() noexcept
	{
		m_value = Sentinel;
	}
};

template<typename T>
struct storage<T, bool_flag{}>
{
	union
	{
		disengaged m_disengaged = {};
		T m_value;
	};
	bool m_engaged = false;

	storage() = default;

	constexpr storage(std::in_place_t, auto&&... args)
		noexcept(noexcept(T(vsm_forward(args)...)))
		: m_value(vsm_forward(args)...)
		, m_engaged(true)
	{
	}

	constexpr storage(storage&&) = default
		requires std::is_trivially_move_constructible_v<T>;

	constexpr storage(storage&& other)
		noexcept(std::is_nothrow_move_constructible_v<T>)
		requires (std::is_move_constructible_v<T> && !std::is_trivially_movable_v<T>)
		: storage()
	{
		if (other.m_engaged)
		{
			std::construct_at(&m_value, vsm_move(other).m_value);
			m_engaged = true;
		}
	}

	constexpr storage(storage const&) = default
		requires std::is_trivially_copy_constructible_v<T>;

	constexpr storage(storage const& other)
		noexcept(std::is_nothrow_copy_constructible_v<T>)
		requires (std::is_copy_constructible_v<T> && !std::is_trivially_copyable_v<T>)
		: storage()
	{
		if (other.m_engaged)
		{
			std::construct_at(&m_value, other.m_value);
			m_engaged = true;
		}
	}

	constexpr ~storage()
		requires (!std::is_trivially_destructible_v<T>)
	{
		if (m_engaged)
		{
			std::destroy_at(&m_value);
		}
	}

	constexpr bool has_value() const noexcept
	{
		return m_engaged;
	}

	constexpr T& emplace(auto&&... args)
		noexcept(noexcept(T(vsm_forward(args)...)))
	{
		if (m_engaged)
		{
			std::destroy_at(&m_value);
			std::construct_at(&m_disengaged);
			m_engaged = false;
		}
		std::construct_at(&m_value, vsm_forward(args)...);
		m_engaged = true;
		return m_value;
	}

	constexpr void reset() noexcept
	{
		if (m_engaged)
		{
			std::destroy_at(&m_value);
			std::construct_at(&m_disengaged);
			m_engaged = false;
		}
	}
};


template<typename T, typename U>
concept assignment_concept =
	std::constructible_from<U, T> &&
	std::assignable_from<U&, T>;

template<typename T, typename U>
inline constexpr bool assignment_noexcept =
	noexcept(T(std::declval<U>())) &&
	noexcept(std::declval<T&>() = std::declval<U>());

template<auto T>
inline constexpr bool is_bool_flag = std::is_same_v<decltype(T), bool_flag>;

template<auto To, auto From>
inline constexpr bool sentinel_conversion = is_bool_flag<To> >= is_bool_flag<From>;

std::false_type detect_optional(auto const&);

template<typename T, auto S>
std::true_type detect_optional(optional<T, S> const&);

template<typename T, auto Sentinel>
class optional : storage<T, Sentinel>
{
	using base = storage<T, Sentinel>;

	static_assert(std::is_same_v<std::remove_cvref_t<T>, T>);

public:
	using value_type = T;


	optional() = default;

	optional(optional&&) = default;
	optional(optional const&) = default;

	constexpr optional(std::nullopt_t) noexcept
	{
	}

	template<constructible_to<T> U = T>
	explicit(!std::is_convertible_v<U&&, T>)
	constexpr optional(U&& value)
		noexcept(noexcept(T(vsm_forward(value))))
		: base(std::in_place, vsm_forward(value))
	{
	}

#if 0
	template<constructible_to<T> U, auto S>
		requires sentinel_conversion<Sentinel, S>
	explicit(!std::is_convertible_v<U&&, T>)
	constexpr optional(optional<U, S>&& other)
		noexcept(noexcept(T(std::declval<U&&>())))
	{
		if (other.has_value())
		{
			base::emplace(vsm_move(other.m_value));
		}
	}

	template<constructible_to<T> U, auto S>
		requires sentinel_conversion<Sentinel, S>
	explicit(!std::is_convertible_v<U const&, T>)
	constexpr optional(optional<U, S> const& other)
		noexcept(noexcept(T(std::declval<U const&>())))
	{
		if (other.has_value())
		{
			base::emplace(other.m_value);
		}
	}
#endif

	template<typename Args>
		requires std::constructible_from<T, Args&&...>
	explicit constexpr optional(std::in_place_t, Args&&... args)
		noexcept(noexcept(T(vsm_forward(args)...)))
		: base(std::in_place, vsm_forward(args)...)
	{
	}

	template<typename Args>
		requires std::constructible_from<T, Args&&...>
	explicit constexpr optional(std::in_place_type_t<T>, Args&&... args)
		noexcept(noexcept(T(vsm_forward(args)...)))
		: base(std::in_place, vsm_forward(args)...)
	{
	}


	optional& operator=(optional&&) = default;
	optional& operator=(optional const&) = default;

	constexpr optional& operator=(std::nullopt_t) noexcept
	{
		base::reset();
		return *this;
	}

	template<assignment_concept<T> U = T>
	constexpr optional& operator=(U&& value)
		noexcept(assignment_noexcept<T, U&&>)
	{
		if (base::has_value())
		{
			base::m_value = vsm_forward(value);
		}
		else
		{
			base::emplace(vsm_forward(value));
		}
		return *this;
	}

#if 0
	template<assignment_concept<T> U, auto S>
		requires sentinel_conversion<Sentinel, S>
	constexpr optional& operator=(optional<U, S>&& other)
		noexcept(assignment_noexcept<T, U&&>)
	{
		if (base::has_value() == other.has_value())
		{
			base::m_value = vsm_move(other.m_value);
		}
		else if (other.has_value())
		{
			base::emplace(vsm_move(other.m_value));
		}
		else
		{
			base::reset();
		}
		return *this;
	}

	template<assignment_concept<T> U, auto S>
		requires sentinel_conversion<Sentinel, S>
	constexpr optional& operator=(optional<U, S> const& other)
		noexcept(assignment_noexcept<T, U const&>)
	{
		if (base::has_value() == other.has_value())
		{
			base::m_value = other.m_value;
		}
		else if (other.has_value())
		{
			base::emplace(other.m_value);
		}
		else
		{
			base::reset();
		}
		return *this;
	}
#endif

	using base::has_value;
	using base::emplace;
	using base::reset;

#define vsm_x_entry(category) \
	template<constructible_to<T> U, auto S> \
		requires sentinel_conversion<Sentinel, S> \
	explicit(!std::is_convertible_v<U category, T>) \
	constexpr optional(optional<U, S> category other) \
		noexcept(noexcept(T(std::declval<U category>()))) \
	{ \
		if (other.has_value()) \
		{ \
			base::emplace(static_cast<U category>(other.m_value)); \
		} \
	} \
	\
	template<assignment_concept<T> U, auto S> \
		requires sentinel_conversion<Sentinel, S> \
	constexpr optional& operator=(optional<U, S> category other) \
		noexcept(assignment_noexcept<T, U category>) \
	{ \
		if (base::has_value() == other.has_value()) \
		{ \
			base::m_value = static_cast<U category>(other.m_value); \
		} \
		else if (other.has_value()) \
		{ \
			base::emplace(static_cast<U category>(other.m_value)); \
		} \
		else \
		{ \
			base::reset(); \
		} \
		return *this; \
	} \
	\
	[[nodiscard]] constexpr T value_or(std::convertible_to<T> auto&& default_value) category \
	{ \
		return base::has_value() \
			? static_cast<T category>(base::m_value) \
			: T(vsm_forward(default_value)); \
	} \
	\
	template<typename Callable>
	[[nodiscard]] constexpr optional or_else(Callable&& callable) category \
	{ \
		using result_type = std::remove_cvref_t<std::invoke_result_t<Callable&&, T category>>; \
		static_assert(std::is_same_v<result_type, optional>); \
		return base::has_value() \
			? static_cast<T category>(base::m_value) \
			: std::invoke(vsm_forward(callable)); \
	} \

	vsm_move_copy_value_categories(vsm_x_entry)
#undef vsm_x_entry

#define vsm_x_entry(category) \
	[[nodiscard]] constexpr T category value() category noexcept \
	{ \
		vsm_assert(base::has_value()); \
		return static_cast<T category>(base::m_value); \
	} \
	\
	[[nodiscard]] constexpr T category operator*() category noexcept \
	{ \
		vsm_assert(base::has_value()); \
		return static_cast<T category>(base::m_value); \
	} \
	\
	template<typename Callable> \
	[[nodiscard]] constexpr auto and_then(Callable&& callable) category \
		noexcept(std::invoke(vsm_forward(callable), std::declval<T category>())) \
	{ \
		using result_type = std::remove_cvref_t<std::invoke_result_t<Callable&&, T category>>; \
		static_assert(decltype(detect_optional(std::declval<result_type const&>()))::value); \
		return base::has_value() \
			? std::invoke(vsm_forward(callable), static_cast<T category>(base::m_value)) \
			: result_type(); \
	} \
	\
	template<typename Callable> \
	[[nodiscard]] constexpr auto transform(Callable&& callable) category \
		noexcept(std::invoke(vsm_forward(callable), std::declval<T category>())) \
	{ \
		using result_type = std::remove_cvref_t<std::invoke_result_t<Callable&&, T category>>; \
		return base::has_value() \
			? optional<result_type>(std::invoke(vsm_forward(callable), static_cast<T category>(base::m_value))) \
			: optional<result_type>(); \
	} \

	vsm_value_categories(vsm_x_entry)
#undef vsm_x_entry

	[[nodiscard]] constexpr T* operator->() noexcept
	{
		vsm_assert(base::has_value());
		return &base::m_value;
	}

	[[nodiscard]] constexpr T const* operator->() const noexcept
	{
		vsm_assert(base::has_value());
		return &base::m_value;
	}

	[[nodiscard]] explicit constexpr bool() const noexcept
	{
		return base::has_value();
	}

private:
	template<typename, auto>
	friend class optional;
};

#if 0
template<typename T>
optional(T&&) -> optional<std::remove_cvref_t<T>>;

template<typename T>
optional(std::in_place_type_t<T>, auto&&...) -> optional<T>;
#endif

} // namespace detail::optional_

using detail::optional_::optional;

} // namespace vsm
