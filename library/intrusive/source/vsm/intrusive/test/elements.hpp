#pragma once

#include <vsm/intrusive/link.hpp>

#include <list>
#include <ranges>
#include <unordered_map>

namespace vsm::intrusive::test {

struct element : intrusive::basic_link<4>
{
	int value;

	element(int const value)
		: value(value)
	{
	}
};

struct elements
{
	std::list<element> list;

	element& operator()(int const value)
	{
		return list.emplace_back(value);
	}
};

struct unique_elements
{
	std::unordered_map<int, element> map;

	element& operator()(int const value)
	{
		return map.try_emplace(value, value).first->second;
	}
};

template<typename Range>
auto values(Range const& range)
	requires std::is_same_v<std::ranges::range_value_t<Range const&>, element>
{
	return std::views::transform(range, [](const auto& x) -> const auto& { return x.value; });
}

} // namespace vsm::intrusive::test
