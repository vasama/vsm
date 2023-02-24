#pragma once

#include <vsm/hash_map.hpp>

#include <concepts>
#include <memory>

#include <cstdint>

namespace vsm {
namespace detail {

template<typename T>
class blackboard_key
{
	char m_dummy;
};

struct blackboard_data_base
{
};

template<typename T>
struct blackboard_data : blackboard_data_base
{
	T value;
};

class blackboard
{
	hash_map<void const*, std::unique_ptr<blackboard_data_base>> m_data;

public:
	template<typename T, std::convertible_to<T> U = T>
	T* set(blackboard_key<T> const& key, U&& value)
	{
		auto const r = m_data.insert(&key, vsm_forward(value));
		return static_cast<blackboard_data<T>*>(r.element->value)->value;
	}

	template<typename T>
	T* get(blackboard_key<T> const& key)
	{
		if (auto const data = m_data.find_value(&key))
		{
			return static_cast<blackboard_data<T>*>(*data)->value;
		}
		return nullptr;
	}

	template<typename T>
	T const* get(blackboard_key<T> const& key) const
	{
		if (auto const data = m_data.find_value(&key))
		{
			return static_cast<blackboard_data<T> const*>(*data)->value;
		}
		return nullptr;
	}
};

} // namespace detail
} // namespace vsm
