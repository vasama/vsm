#pragma once

namespace vsm {

template<typename Key, typename Value>
struct key_value_pair
{
	Key key;
	Value value;
};

namespace detail {

template<template<typename> typename Iterator, typename T>
struct key_value_pair_key_iterator : Iterator<T>
{
	using Iterator<T>::Iterator;

	[[nodiscard]] auto& operator*() const
	{
		return static_cast<Iterator<T> const&>(*this)->key;
	}

	[[nodiscard]] auto* operator->() const
	{
		return &static_cast<Iterator<T> const&>(*this)->key;
	}
};

template<template<typename> typename Iterator, typename T>
struct key_value_pair_value_iterator : Iterator<T>
{
	using Iterator<T>::Iterator;

	[[nodiscard]] auto& operator*() const
	{
		return static_cast<Iterator<T> const&>(*this)->value;
	}

	[[nodiscard]] auto* operator->() const
	{
		return &static_cast<Iterator<T> const&>(*this)->value;
	}
};

} // namespace detail
} // namespace vsm
