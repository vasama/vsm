#pragma once

#include <vsm/cli/base.hpp>

#include <vsm/tag_ptr.hpp>
#include <vsm/vector.hpp>

#include <algorithm>

namespace vsm::cli {

using detail::resource_locks;
using detail::base;

[[nodiscard]] inline base const* lock_resource(resource& resource, base const& owner, bool const exclusive)
{
	if (resource.m_lock_owner == nullptr)
	{
		resource->m_lock_owner.set(&owner, exclusive);
	}
	else if (exclusive || resource.m_lock_owner.tag())
	{
		return resource->m_lock_owner.ptr();
	}
	return nullptr;
}

class resource_locks
{
	vector<tag_ptr<resource, bool>> m_resources;

public:
	void inclusive(resource& resource)
	{
		add_resource(resource, false);
	}

	void exclusive(resource& resource)
	{
		add_resource(resource, true);
	}

	[[nodiscard]] base const* lock(base const& owner)
	{
		for (auto const ptr : m_resources)
		{
			if (base const* const existing_owner = lock_resource(*ptr, owner, ptr.tag()))
			{
				return existing_owner;
			}
		}
		return nullptr;
	}

private:
	void add_resource(resource& resource, bool const exclusive)
	{
		auto const it = std::find_if(
			m_resources.begin(), m_resources.end(),
			[&](auto const ptr) { return ptr.ptr() == &resource; });

		if (it == m_resources.end())
		{
			m_resources.push_back(tag_ptr(&resource, exclusive));
		}
		else if (exclusive)
		{
			it->set_tag(true);
		}
	}
};

} // namespace vsm::cli
