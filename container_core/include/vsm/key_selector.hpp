#pragma once

#include "vsm/concepts.hpp"

namespace vsm {

struct identity_key_selector
{
	template<typename T>
	T const& operator()(T const& key) const noexcept
	{
		return key;
	}
};

struct address_key_selector
{
	template<typename T>
	T const* operator()(T const& key) const noexcept
	{
		return &key;
	}
};

template<typename KeySelector, typename T>
concept key_selector = requires (KeySelector const& key_selector, T const& object)
{
	{ key_selector(object) } -> not_same_as<void>;
};

} // namespace vsm
