#pragma once

#include <vsm/cli/detail/base.hpp>

#include <vsm/pointer_tag_pair.hpp>
#include <vsm/vector.hpp>

#include <algorithm>

namespace vsm::cli {

using detail::resource_locks;
using detail::base;

class resource_locks
{
	using pair_type = pointer_tag_pair<resource, bool>;

	vector<pair_type> m_resources;

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
		for (auto const [pointer, exclusive] : m_resources)
		{
			if (base const* const existing_owner = base::lock_resource(*pointer, owner, exclusive))
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
			[&](auto const ptr) { return ptr.pointer() == &resource; });

		if (it == m_resources.end())
		{
			m_resources.push_back(pair_type(&resource, exclusive));
		}
		else if (exclusive)
		{
			*it = pair_type(it->pointer(), true);
		}
	}
};

} // namespace vsm::cli
