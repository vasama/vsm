#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

#include <memory>
#include <optional>
#include <type_traits>

namespace vsm {
namespace detail::_optional {

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
		return m_value;
	}

	constexpr T& emplace(auto&&... args) &
	{
		m_value = T(vsm_forward(args)...);
		vsm_assert(m_value != Sentinel);
		return m_value;
	}

	constexpr void reset() & noexcept
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

	constexpr storage(storage&& other) = default;
	constexpr storage(storage&& other)
		noexcept(std::is_nothrow_move_constructible_v<T>)
		requires (std::is_move_constructible_v<T> && !std::is_trivially_copyable_v<T>)
		: storage()
	{
		if (other.m_engaged)
		{
			std::construct_at(&m_value, vsm_move(other).m_value);
			m_engaged = true;
		}
	}

	constexpr storage(storage const& other) = default;
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

	constexpr ~storage() = default;
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

	constexpr T& emplace(auto&&... args) &
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

	constexpr void reset() & noexcept
	{
		if (m_engaged)
		{
			std::destroy_at(&m_value);
			std::construct_at(&m_disengaged);
			m_engaged = false;
		}
	}
};


template<typename LHS, typename RHS>
concept _assignable_from = std::constructible_from<LHS, RHS> && std::assignable_from<LHS&, RHS>;

template<auto T>
inline constexpr bool is_bool_flag = std::is_same_v<decltype(T), bool_flag>;

template<auto To, auto From>
inline constexpr bool sentinel_conversion = is_bool_flag<To> >= is_bool_flag<From>;

std::false_type detect_optional(auto const&);

template<typename T, auto S>
std::true_type detect_optional(optional<T, S> const&);

void _optional_concept(auto const&);

template<typename T, auto S>
char _optional_concept(optional<T, S> const&);

template<typename T>
concept optional_concept = requires
{
	sizeof(_optional_concept(vsm_declval(std::remove_cvref_t<T>)));
};

template<optional_concept Optional>
using value_cvref_t = copy_cvref_t<Optional, typename std::remove_cvref_t<Optional>::value_type>;


template<typename T, auto Sentinel>
class optional : storage<T, Sentinel>
{
	using base = storage<T, Sentinel>;

	static_assert(std::is_same_v<std::remove_cvref_t<T>, T>);

public:
	using value_type = T;


	// [optional.ctor]

	optional() = default;

	constexpr optional(std::nullopt_t) noexcept
	{
	}

	optional(optional&&) = default;
	optional(optional const&) = default;

	template<typename... Args>
		requires std::constructible_from<T, Args&&...>
	explicit constexpr optional(std::in_place_t, Args&&... args)
		: base(std::in_place, vsm_forward(args)...)
	{
	}

	template<constructible_to<T> U = T>
	explicit(!std::is_convertible_v<U&&, T>)
		constexpr optional(U&& value)
		noexcept(noexcept(T(vsm_forward(value))))
		: base(std::in_place, vsm_forward(value))
	{
	}

	template<optional_concept U>
		requires std::constructible_from<T, value_cvref_t<U>>
	explicit(!std::convertible_to<T, value_cvref_t<U>>)
	constexpr optional(U&& other)
	{
		if (other.has_value())
		{
			base::emplace(static_cast<value_cvref_t<U>&&>(other.m_value));
		}
	}


	// [optional.assign]

	constexpr optional& operator=(std::nullopt_t) & noexcept
	{
		base::reset();
		return *this;
	}

	optional& operator=(optional&&) & = default;
	optional& operator=(optional const&) & = default;

	template<typename U = T>
		requires _assignable_from<T, U>
	constexpr optional& operator=(U&& value) &
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

	template<optional_concept U>
		requires _assignable_from<T, value_cvref_t<U>>
	constexpr optional& operator=(U&& other) &
	{
		if (base::has_value() == other.has_value())
		{
			base::m_value = static_cast<value_cvref_t<U>&&>(other.m_value);
		}
		else if (other.has_value())
		{
			base::emplace(static_cast<value_cvref_t<U>&&>(other.m_value));
		}
		else
		{
			base::reset();
		}
		return *this;
	}

	using base::emplace;


	// [optional.observe]

	template<typename U>
	[[nodiscard]] constexpr remove_ref_t<copy_cvref_t<U, T>>* operator->(this U&& self)
	{
		vsm_assert(base::has_value());
		return &base::m_value;
	}

	template<typename U>
	[[nodiscard]] constexpr copy_cvref_t<U, T>&& operator*(this U&& self)
	{
		vsm_assert(base::has_value());
		return static_cast<copy_cvref_t<U, T>&&>(base::m_value);
	}

	[[nodiscard]] explicit constexpr operator bool() const noexcept
	{
		return base::has_value();
	}

	using base::has_value;

	template<typename U>
	[[nodiscard]] constexpr copy_cvref_t<U, T>&& value(this U&& self)
	{
		vsm_assert(base::has_value());
		return static_cast<copy_cvref_t<U, T>&&>(base::m_value);
	}

	template<typename U>
	[[nodiscard]] constexpr T value_or(this U&& self, std::convertible_to<T> auto&& default_value)
	{
		return base::has_value()
			? static_cast<copy_cvref_t<U, T>&&>(base::m_value)
			: T(vsm_forward(default_value));
	}


	// [optional.monadic]

	template<typename U, typename Callable>
	[[nodiscard]] constexpr auto and_then(this U&& self, Callable&& callable)
	{
		using result_type = std::remove_cvref_t<std::invoke_result_t<Callable&&, copy_cvref_t<U, T>>>;
		static_assert(optional_concept<result_type>);
		return base::has_value()
			? std::invoke(vsm_forward(callable), static_cast<copy_cvref_t<U, T>&&>(base::m_value))
			: result_type();
	}

	template<typename U, typename Callable>
	[[nodiscard]] constexpr auto transform(this U&& self, Callable&& callable)
	{
		using result_type = std::decay_t<std::invoke_result_t<Callable&&, copy_cvref_t<U, T>>>;
		return base::has_value()
			? optional<result_type>(std::invoke(vsm_forward(callable), static_cast<copy_cvref_t<U, T>&&>(base::m_value)))
			: optional<result_type>();
	}

	template<typename U, typename Callable>
	[[nodiscard]] constexpr optional or_else(this U&& self, Callable&& callable)
	{
		static_assert(std::is_same_v<
			optional,
			std::remove_cvref_t<std::invoke_result_t<Callable&&, copy_cvref_t<U, T>>>>);

		return base::has_value()
			? static_cast<copy_cvref_t<U, T>&&>(base::m_value)
			: std::invoke(vsm_forward(callable));
	}


	// [optional.mod]

	using base::reset;

private:
	template<typename, auto>
	friend class optional;
};

template<typename T>
optional(T&&) -> optional<std::decay_t<T>>;

} // namespace detail::_optional

using detail::_optional::optional;

} // namespace vsm
