#pragma once

#include <vsm/assert.h>
#include <vsm/utility.hpp>

#include <concepts>
#include <optional>
#include <type_traits>

namespace vsm {
namespace detail {

enum { unique_resource_sentinel };

struct null_resource_t {};
inline constexpr null_resource_t null_resource = {};

template<typename Resource, typename Deleter, auto Sentinel = unique_resource_sentinel>
class unique_resource;

template<typename Resource, auto Sentinel = unique_resource_sentinel>
class unique_resource_storage
{
	Resource m_resource;

public:
	using sentinel_type = decltype(Sentinel);
	static_assert(std::is_convertible_v<sentinel_type, Resource>);

	static constexpr sentinel_type sentinel = Sentinel;


	[[nodiscard]] Resource const& get() const noexcept
	{
		vsm_assert(m_resource != Sentinel);
		return m_resource;
	}

	explicit operator bool() const noexcept
	{
		return m_resource != Sentinel;
	}

private:
	unique_resource_storage(Resource const resource = Sentinel) noexcept
		: m_resource(resource)
	{
	}

	unique_resource_storage(unique_resource_storage&& source) noexcept
		: m_resource(source.m_resource)
	{
		source.m_resource = Sentinel;
	}

	unique_resource_storage& operator=(unique_resource_storage&& source) noexcept
	{
		m_resource = source.m_resource;
		source.m_resource = Sentinel;
		return *this;
	}

	void set(Resource const resource) noexcept
	{
		m_resource = resource;
	}

	void clear() noexcept
	{
		m_resource = Sentinel;
	}

	template<typename, typename, auto>
	friend class unique_resource;
};

template<typename Resource>
class unique_resource_storage<Resource, unique_resource_sentinel>
{
	std::optional<Resource> m_resource;

public:
	[[nodiscard]] Resource const& get() const noexcept
	{
		return *m_resource;
	}

	explicit operator bool() const noexcept
	{
		return m_resource.has_value();
	}

private:
	unique_resource_storage() = default;

	unique_resource_storage(Resource const resource) noexcept
		: m_resource(resource)
	{
	}

	unique_resource_storage(unique_resource_storage&& source) noexcept
		: m_resource(source.m_resource)
	{
		source.m_resource.reset();
	}

	unique_resource_storage& operator=(unique_resource_storage&& source) noexcept
	{
		m_resource = source.m_resource;
		source.m_resource.reset();
		return *this;
	}

	void set(Resource const resource) noexcept
	{
		m_resource = resource;
	}

	void clear() noexcept
	{
		m_resource.reset();
	}

	template<typename, typename, auto>
	friend class unique_resource;
};

template<typename Resource, typename Deleter, auto Sentinel>
class unique_resource : public unique_resource_storage<Resource, Sentinel>
{
	using storage_type = unique_resource_storage<Resource, Sentinel>;

	static_assert(std::is_trivially_copyable_v<Resource>);
	static_assert(std::is_nothrow_move_constructible_v<Deleter>);
	static_assert(std::is_nothrow_move_assignable_v<Deleter>);

	[[no_unique_address]] Deleter m_deleter;

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
		: storage_type(resource)
	{
	}

	explicit constexpr unique_resource(Resource const resource, Deleter&& deleter) noexcept
		: storage_type(resource)
		, m_deleter(vsm_move(deleter))
	{
	}

	explicit constexpr unique_resource(Resource const resource, Deleter const& deleter) noexcept
		: storage_type(resource)
		, m_deleter(deleter)
	{
	}

	constexpr unique_resource(unique_resource&& source) noexcept
		: storage_type(static_cast<storage_type&&>(source))
		, m_deleter(vsm_move(source.m_deleter))
	{
	}

	constexpr unique_resource& operator=(null_resource_t) noexcept
	{
		if (*this)
		{
			m_deleter(this->get());
			this->clear();
		}
		return *this;
	}

	constexpr unique_resource& operator=(unique_resource&& source) noexcept
	{
		if (*this)
		{
			m_deleter(this->get());
		}
		static_cast<storage_type&>(*this) = static_cast<storage_type&&>(source);
		m_deleter = vsm_move(source.m_deleter);
		return *this;
	}

	constexpr ~unique_resource() noexcept
	{
		if (*this)
		{
			m_deleter(this->get());
		}
	}


	[[nodiscard]] constexpr Deleter const& get_deleter() const noexcept
	{
		return m_deleter;
	}


	[[nodiscard]] constexpr Resource release() noexcept
	{
		Resource resource = this->get();
		this->clear();
		return resource;
	}

	constexpr void reset() noexcept
	{
		if (*this)
		{
			m_deleter(this->get());
		}
		this->clear();
	}

	constexpr void reset(Resource const resource) noexcept
	{
		if (*this)
		{
			m_deleter(this->get());
		}
		this->set(resource);
	}


	friend constexpr bool operator==(unique_resource const& lhs, unique_resource const& rhs) noexcept
	{
		return lhs.get() == rhs.get();
	}

	friend constexpr auto operator<=>(unique_resource const& lhs, unique_resource const& rhs) noexcept
	{
		return lhs.get() <=> rhs.get();
	}

	friend constexpr bool operator==(unique_resource const& r, null_resource_t) noexcept
	{
		return static_cast<bool>(r);
	}

	friend constexpr bool operator!=(unique_resource const& r, null_resource_t) noexcept
	{
		return !static_cast<bool>(r);
	}

	friend constexpr bool operator==(null_resource_t, unique_resource const& r) noexcept
	{
		return static_cast<bool>(r);
	}

	friend constexpr bool operator!=(null_resource_t, unique_resource const& r) noexcept
	{
		return !static_cast<bool>(r);
	}
};


template<typename T>
concept unique_resource_concept = requires (T&& t)
{
	t.get();
	{ t.release() } -> std::same_as<std::remove_cvref_t<decltype(t.get())>>;
};

template<typename Consumer, unique_resource_concept... Resources>
auto consume_resources(Consumer&& consumer, Resources&&... resources)
{
	auto r = vsm_forward(consumer)(resources.get()...);
	if (r)
	{
		((void)resources.release(), ...);
	}
	return r;
}

} // namespace detail

using detail::null_resource_t;
using detail::null_resource;

using detail::unique_resource;
using detail::consume_resources;

} // namespace vsm
