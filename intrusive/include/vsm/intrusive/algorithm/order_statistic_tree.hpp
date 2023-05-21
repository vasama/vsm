#pragma once

#include <array>
#include <concepts>
#include <limits>
#include <optional>
#include <type_traits>

namespace vsm::intrusive {

template<typename T>
concept order_statistic_tree = requires (T& mt, T const& ct, typename T::iterator_const const& ci)
{
	//{ typename T::size_type } -> std::unsigned_integral;

	{ mt.root() } -> std::same_as<typename T::iterator>;
	{ ct.root() } -> std::same_as<typename T::const_iterator>;

	{ mt.children(ci) } -> std::same_as<std::array<typename T::iterator, 2>>;
	{ ct.children(ci) } -> std::same_as<std::array<typename T::const_iterator, 2>>;

	{ ct.weight(ci) } -> std::same_as<typename T::size_type>;
};

template<order_statistic_tree OrderStatisticTree>
	requires std::integral<typename OrderStatisticTree::key_type>
std::optional<typename OrderStatisticTree::key_type>
	order_statistic_free_key(OrderStatisticTree& tree, auto select_key, auto select_key_index)
{
	using key_type = typename OrderStatisticTree::key_type;
	using unsigned_key_type = std::make_unsigned_t<key_type>;

	using size_type = typename OrderStatisticTree::size_type;
	using common_type = std::common_type_t<unsigned_key_type, size_type>;

	static constexpr auto max_key = std::numeric_limits<unsigned_key_type>::max();

	size_type const size = tree.size();
	vsm_assert_slow(size <= max_key);

	if (size >= max_key)
	{
		return std::nullopt;
	}

	// Index of the resulting key in the set of all available keys.
	auto key_index = select_key_index(max_key - size);

	auto const end = tree.end();
	auto const weight = [&](auto const iterator) -> size_type
	{
		return iterator != end ? tree.weight(iterator) : 0;
	};

	for (auto root = tree.root(); root != end;)
	{
		key_type const root_key = select_key(*root);
		auto const children = tree.children(root);

		size_type const l_weight = weight(children[0]);

		// How many keys are available on the left side of root.
		auto const l_available = root_key - l_weight;

		if (key_index < l_available)
		{
		}
		else
		{
			
		}
	}
}

} // namespace vsm::intrusive
