#pragma once

#include <bit>

namespace vsm {
namespace detail {

template<
	typename Container,
	typename Value,
	bool = (sizeof(Container) > sizeof(Value))>
	requires (sizeof(Container) >= sizeof(Value))
struct _bit_pack
{
	Value value;
};

template<typename Container, typename Value>
struct _bit_pack<Container, Value, true>
{
	Value value;
	unsigned char padding[sizeof(Container) - sizeof(Value)];
};

} // namespace detail

template<typename Container, typename Value>
	requires
		(sizeof(Container) >= sizeof(Value)) &&
		std::is_trivially_copyable_v<Container> &&
		std::is_trivially_copyable_v<Value>
[[nodiscard]] constexpr Container bit_pack(Value const& value)
{
	return std::bit_cast<Container>(detail::_bit_pack<Container, Value>(value));
}

template<typename Value, typename Container>
	requires
		(sizeof(Container) >= sizeof(Value)) &&
		std::is_trivially_copyable_v<Container> &&
		std::is_trivially_copyable_v<Value>
[[nodiscard]] constexpr Value bit_unpack(Container const& container)
{
	return std::bit_cast<detail::_bit_pack<Container, Value>>(container).value;
}

} // namespace vsm
