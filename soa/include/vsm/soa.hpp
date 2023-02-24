#pragma once

namespace vsm {

template<typename T>
class soa_channels
{
	T* base;
	size_t count;
};

namespace detail::soa_ {

template<typename T>
struct traits
{
	using base_type = T;
	using has_channels_type = char;

	template<template<typename> typename Qualify>
	using data_type = qualify<T>*;
};

template<typename T>
struct traits<soa_channels<T>>
{
	using base_type = T;
	using has_channels_type = char[2];

	template<template<typename> typename Qualify>
	using data_type = soa_channels<Qualify<T>>;
};

template<typename T>
using select_identity = T;

template<typename T>
using select_element = typename traits<T>::template data_type<select_identity>;

template<typename T>
using select_const = T const;

template<typename T>
using select_const_element = typename traits<T>::template data_type<select_const>;

template<typename T>
using select_char = char;

template<template<template<typename> typename> T>
inline constexpr size_t element_count = sizeof(T<select_char>);

template<typename T>
using select_base = typename traits<T>::base_type;

template<typename T>
using select_has_channels_type = typename traits<T>::has_channels_type;

template<template<template<typename> typename> T>
inline constexpr bool has_channels = sizeof(T<select_has_channels_type>) > sizeof(T<select_char>);

} // namespace detail::soa_

#define vsm_detail detail::soa_

template<template<template<typename> typename> typename T, typename Allocator>
class soa
{
	using data_type = T<vsm_detail::select_element>;
	using const_data_type = T<vsm_detail::select_const_element>;

	data_layout<T, Allocator> m;

public:
	static constexpr bool has_channels = vsm_detail::has_channels<T>;


	soa() = default;

	explicit soa(Allocator const& allocator);


	size_t size() const
	{
		return m.c.size;
	}

	size_t capacity() const
	{
		return m.c.capacity;
	}
	
	data_type const& data()
	{
		return m.d;
	}
	
	const_data_type const& data() const
	{
		return reinterpret_cast<const_data_type const&>(m.d);
	}

	data_type const* operator->()
	{
		return &m.d;
	}

	data_type const* operator->()
	{
		return &reinterpret_cast<const_data_type const&>(m.d);
	}


	template<typename Field>
	std::span<Field> field(Field* const& field)
	{
		return std::span<Field>(field, m.c.size);
	}

	template<typename Field>
	std::span<Field const> field(Field const* const& field) const
	{
		return std::span<Field const>(field, m.c.size);
	}


	template<typename Field>
	vsm::span2<Field> field(soa_channels<Field> const& field)
	{
		return std::span2<Field>(field.base, m.c.size, field.count);
	}

	template<typename Field>
	vsm::span2<Field const> field(soa_channels<Field const> const& field) const
	{
		return std::span2<Field const>(field.base, m.c.size, field.count);
	}


	template<typename Field>
	void set_channel_count(soa_channels<Field> const& field, size_t const count)
	{
		const_cast<soa_channels<Field>&>(field);
	}

	void resize(size_t const new_size)
	{
		
	}
	
	void reserve(size_t const min_capacity)
	{
	}

	void shrink_to_fit()
	{
	}
};

#undef vsm_detail

} // namespace vsm
