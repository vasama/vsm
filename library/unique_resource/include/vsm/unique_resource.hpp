#pragma once

#include <vsm/assert.h>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <concepts>
#include <optional>
#include <type_traits>

namespace vsm {
namespace detail {

struct unique_resource_sentinel_t {};
inline constexpr unique_resource_sentinel_t unique_resource_no_sentinel = {};

struct null_resource_t {};
inline constexpr null_resource_t null_resource = {};

template<typename Resource, typename Deleter, auto Sentinel = unique_resource_no_sentinel>
class unique_resource;

template<typename Resource, auto Sentinel>
class unique_resource_optional
{
	Resource m_value;

public:
	constexpr unique_resource_optional()
		: m_value(Sentinel)
	{
	}

	template<typename... Args>
		requires std::constructible_from<Resource, Args...>
	constexpr unique_resource_optional(Args&&... args)
		: m_value(vsm_forward(args)...)
	{
	}

	[[nodiscard]] constexpr Resource const& operator*() const noexcept
	{
		return m_value;
	}

	template<typename... Args>
		requires std::constructible_from<Resource, Args...>
	constexpr void emplace(Args&&... args)
	{
		m_value = Resource(vsm_forward(args)...);
	}

	constexpr void reset() noexcept
	{
		m_value = Sentinel;
	}

	constexpr void swap(unique_resource_optional& other) noexcept
	{
		using std::swap;
		swap(this->m_value, other.m_value);
	}

	[[nodiscard]] explicit constexpr operator bool() const noexcept
	{
		return m_value != Sentinel;
	}
};

template<typename Resource, auto Sentinel>
class unique_resource_base
{
	static_assert(std::is_convertible_v<decltype(Sentinel), Resource>);

	using optional_type = unique_resource_optional<Resource, Sentinel>;

public:
	using sentinel_type = decltype(Sentinel);
	static constexpr sentinel_type sentinel = Sentinel;

private:
	template<typename, typename, auto>
	friend class unique_resource;
};

template<typename Resource>
class unique_resource_base<Resource, unique_resource_no_sentinel>
{
	using optional_type = std::optional<Resource>;

	template<typename, typename, auto>
	friend class unique_resource;
};

template<typename Resource, typename Deleter, auto Sentinel>
class unique_resource : public unique_resource_base<Resource, Sentinel>
{
	using base_type = unique_resource_base<Resource, Sentinel>;
	using optional_type = typename base_type::optional_type;

	static_assert(std::is_trivially_copyable_v<Resource>);
	static_assert(std::is_nothrow_move_constructible_v<Deleter>);
	static_assert(std::is_nothrow_move_assignable_v<Deleter>);

	optional_type m_optional;
	vsm_no_unique_address Deleter m_deleter;

public:
	using resource_type = Resource;
	using deleter_type = Deleter;


	unique_resource() = default;

	constexpr unique_resource(null_resource_t) noexcept
	{
	}

	explicit constexpr unique_resource(null_resource_t, Deleter&& deleter) noexcept
		: m_deleter(vsm_move(deleter))
	{
	}

	explicit constexpr unique_resource(null_resource_t, Deleter const& deleter) noexcept
		: m_deleter(deleter)
	{
	}

	explicit constexpr unique_resource(Resource const resource) noexcept
		: m_optional(resource)
	{
	}

	explicit constexpr unique_resource(Resource const resource, Deleter&& deleter) noexcept
		: m_optional(resource)
		, m_deleter(vsm_move(deleter))
	{
	}

	explicit constexpr unique_resource(Resource const resource, Deleter const& deleter) noexcept
		: m_optional(resource)
		, m_deleter(deleter)
	{
	}

	constexpr unique_resource(unique_resource&& other) noexcept
		: m_optional(other.m_optional)
		, m_deleter(vsm_move(other.m_deleter))
	{
		other.m_optional.reset();
	}

	constexpr unique_resource& operator=(null_resource_t) & noexcept
	{
		if (m_optional)
		{
			m_deleter(*m_optional);
			m_optional.reset();
		}
		return *this;
	}

	constexpr unique_resource& operator=(unique_resource&& other) & noexcept
	{
		auto local = vsm_move(other);
		swap(local, *this);
		return *this;
	}

	constexpr ~unique_resource() noexcept
	{
		if (m_optional)
		{
			m_deleter(*m_optional);
		}
	}


	[[nodiscard]] constexpr Resource const& get() const
	{
		vsm_assert(m_optional);
		return *m_optional;
	}

	[[nodiscard]] constexpr Deleter const& get_deleter() const noexcept
	{
		return m_deleter;
	}


	[[nodiscard]] constexpr Resource release()
	{
		vsm_assert(m_optional);
		auto const r = *m_optional;
		m_optional.reset();
		return r;
	}

	constexpr void reset() & noexcept
	{
		if (m_optional)
		{
			m_deleter(*m_optional);
		}
		m_optional.reset();
	}

	constexpr void reset(Resource const resource) & noexcept
	{
		if (m_optional)
		{
			m_deleter(*m_optional);
		}
		m_optional.emplace(resource);
	}

	friend constexpr void swap(unique_resource& lhs, unique_resource& rhs) noexcept
	{
		lhs.m_optional.swap(rhs.m_optional);
	}


	[[nodiscard]] explicit constexpr operator bool() const noexcept
	{
		return static_cast<bool>(m_optional);
	}

	[[nodiscard]] friend constexpr bool operator==(
		unique_resource const& lhs,
		unique_resource const& rhs) noexcept
	{
		return lhs.get() == rhs.get();
	}

	[[nodiscard]] friend constexpr auto operator<=>(
		unique_resource const& lhs,
		unique_resource const& rhs) noexcept
	{
		return lhs.get() <=> rhs.get();
	}

	[[nodiscard]] friend constexpr bool operator==(
		unique_resource const& r,
		null_resource_t) noexcept
	{
		return static_cast<bool>(r);
	}

	[[nodiscard]] friend constexpr bool operator!=(
		unique_resource const& r,
		null_resource_t) noexcept
	{
		return !static_cast<bool>(r);
	}
};

} // namespace detail

using detail::null_resource_t;
using detail::null_resource;

using detail::unique_resource;

} // namespace vsm
