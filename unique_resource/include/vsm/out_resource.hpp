#pragma once

#include <tuple>

namespace vsm {
namespace detail {

template<typename Owner, typename Resource, typename... Args>
class out_resource_type
{
	Owner& m_owner;
	std::tuple<Args...> m_args;
	mutable Resource m_resource;

public:
	explicit out_resource_type(Owner& owner, Args&&... args)
		: m_owner(owner)
		, m_args(vsm_forward(args)...)
		, m_resource(Owner::sentinel)
	{
	}

	out_resource_type(out_resource_type const&) = delete;
	out_resource_type& operator=(out_resource_type const&) = delete;

	~out_resource_type()
	{
		if (m_resource != Owner::sentinel)
		{
			std::apply([&](auto&&... args)
			{
				if constexpr (requires { m_owner.reset(vsm_move(m_resource), vsm_forward(args)...); })
				{
					m_owner.reset(vsm_move(m_resource), vsm_forward(args)...);
				}
				else
				{
					m_owner = Owner(vsm_move(m_resource), vsm_forward(args)...);
				}
			}, vsm_move(m_args));
		}
	}

	operator Resource*() const noexcept
	{
		return &m_resource;
	}
};

template<typename Owner, typename Resource, typename... Args>
class inout_resource_type
{
	Owner& m_owner;
	std::tuple<Args...> m_args;
	mutable Resource m_resource;

public:
	explicit inout_resource_type(Owner& owner, Args&&... args)
		: m_owner(owner)
		, m_args(vsm_forward(args)...)
		, m_resource(owner.release())
	{
	}

	inout_resource_type(inout_resource_type const&) = delete;
	inout_resource_type& operator=(inout_resource_type const&) = delete;

	~inout_resource_type()
	{
		std::apply([&](auto&&... args)
		{
			m_owner.reset(vsm_move(m_resource), vsm_forward(args)...);
		}, vsm_move(m_args));
	}

	operator Resource*() const noexcept
	{
		return &m_resource;
	}
};

template<bool is_void>
struct resource_type;

template<>
struct resource_type<0>
{
	template<typename Owner, typename Resource>
	using type = Resource;
};

template<>
struct resource_type<1>
{
	template<typename Owner, typename Resource>
	using type = Owner::resource_type;
};

} // namespace detail

template<typename Resource = void, typename Owner, typename... Args>
[[nodiscard]] auto out_resource(Owner& owner, Args&&... args)
{
	using resource_type =
		detail::resource_type<std::is_void_v<Resource>>::template type<Owner, Resource>;
	return detail::out_resource_type<Owner, resource_type, Args&&...>(owner, vsm_forward(args)...);
}

template<typename Resource = void, typename Owner, typename... Args>
[[nodiscard]] auto inout_resource(Owner& owner, Args&&... args)
{
	using resource_type =
		detail::resource_type<std::is_void_v<Resource>>::template type<Owner, Resource>;
	return detail::inout_resource_type<Owner, resource_type, Args&&...>(
		owner,
		vsm_forward(args)...);
}

} // namespace vsm
