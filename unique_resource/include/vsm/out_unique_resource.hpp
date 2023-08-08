#pragma once

#include <vsm/unique_resource.hpp>

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
		std::apply([&](auto&&... args)
		{
			m_owner.reset(vsm_move(m_resource), vsm_forward(args)...);
		}, vsm_move(m_args));
	}

	Resource* operator() const noexcept
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
	explicit out_resource_type(Owner& owner, Args&&... args)
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

	Resource* operator() const noexcept
	{
		return &m_resource;
	}
};

} // namespace detail

template<typename Resource = void, typename Owner, typename... Args>
auto out_resource(Owner& owner, Args&&... args)
{
	return detail::out_resource_type<Owner, Resource, Args&&...>(owner, vsm_forward(args)...);
}

template<typename Resource = void, typename Owner, typename... Args>
auto inout_resource(Owner& owner, Args&&... args)
{
	return detail::inout_resource_type<Owner, Resource, Args&&...>(owner, vsm_forward(args)...);
}

} // namespace vsm
