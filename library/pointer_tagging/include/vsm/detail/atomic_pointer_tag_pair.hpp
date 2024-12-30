#ifndef vsm_detail_fetch_mutate_category
#	error vsm_detail_fetch_mutate_category is not defined.
#endif

#ifndef vsm_detail_fetch_mutate_ref
#	error vsm_detail_fetch_mutate_get_ref is not defined.
#endif

[[nodiscard]] vsm_always_inline value_type load(std::memory_order const order) vsm_detail_fetch_mutate_category noexcept
{
	return value_type(vsm_detail_fetch_mutate_ref.load(order));
}

vsm_always_inline void store(value_type const new_value, std::memory_order const order) vsm_detail_fetch_mutate_category noexcept
{
	vsm_detail_fetch_mutate_ref.store(new_value.m_value, order);
}

[[nodiscard]] vsm_always_inline value_type exchange(
	value_type const new_value,
	std::memory_order const order) vsm_detail_fetch_mutate_category noexcept
{
	return value_type(vsm_detail_fetch_mutate_ref.exchange(new_value.m_value, order));
}

[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
	value_type& expected,
	value_type const desired,
	std::memory_order const success,
	std::memory_order const failure) vsm_detail_fetch_mutate_category noexcept
{
	return vsm_detail_fetch_mutate_ref.compare_exchange_weak(
		expected.m_value,
		desired.m_value,
		success,
		failure);
}

[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
	value_type& expected,
	value_type const desired,
	std::memory_order const success,
	std::memory_order const failure) vsm_detail_fetch_mutate_category noexcept
{
	return vsm_detail_fetch_mutate_ref.compare_exchange_strong(
		expected.m_value,
		desired.m_value,
		success,
		failure);
}

#define vsm_detail_fetch_mutate_pointer(operation) \
	[[nodiscard]] value_type operation ## _pointer( \
		ptrdiff_t const offset, \
		std::memory_order const order) vsm_detail_fetch_mutate_category \
		noexcept(BitsRequested <= detail::taggable_bits_available<T>()) \
		requires no_cv_of<T, void> \
	{ \
		uintptr_t const byte_offset = static_cast<uintptr_t>(offset) * sizeof(T); \
		\
		if constexpr (BitsRequested > detail::taggable_bits_available<T>()) \
		{ \
			vsm_assert((byte_offset & value_type::tag_mask) == 0); /*PRECONDITION*/ \
		} \
		\
		return value_type(vsm_detail_fetch_mutate_ref.operation(byte_offset, order)); \
	} \

	vsm_detail_fetch_mutate_pointer(fetch_add)
	vsm_detail_fetch_mutate_pointer(fetch_sub)
#undef vsm_detail_fetch_mutate_pointer

#define vsm_detail_fetch_mutate_tag(operation, operator) \
	[[nodiscard]] value_type operation ## _tag( \
		Tag const tag, \
		std::memory_order const order) vsm_detail_fetch_mutate_category noexcept \
		requires detail::taggable_pointer_tag_arithmetic<Tag> \
	{ \
		std::memory_order const load_order = order == std::memory_order_acq_rel \
			? std::memory_order_acquire \
			: order; \
		\
		auto const& ref = vsm_detail_fetch_mutate_ref; \
		\
		uintptr_t old_value = ref.load(load_order); \
		\
		while (!ref.compare_exchange_weak( \
			old_value, \
			(old_value operator static_cast<uintptr_t>(tag)) & value_type::tag_mask, \
			order, \
			load_order)); \
		\
		return value_type(old_value); \
	} \

	vsm_detail_fetch_mutate_tag(fetch_add, +)
	vsm_detail_fetch_mutate_tag(fetch_sub, -)
#undef vsm_detail_fetch_mutate_tag

#define vsm_detail_fetch_mutate_tag(operation) \
	[[nodiscard]] value_type operation ## _tag( \
		Tag const tag, \
		std::memory_order const order) vsm_detail_fetch_mutate_category \
		requires detail::taggable_pointer_tag_bitwise<Tag> \
	{ \
		vsm_assert(static_cast<uintptr_t>(tag) <= value_type::tag_mask); /*PRECONDITION*/ \
		return value_type(vsm_detail_fetch_mutate_ref.operation(static_cast<uintptr_t>(tag), order)); \
	} \

	vsm_detail_fetch_mutate_tag(fetch_and)
	vsm_detail_fetch_mutate_tag(fetch_or)
	vsm_detail_fetch_mutate_tag(fetch_xor)
#undef vsm_detail_fetch_mutate_tag

#undef vsm_detail_fetch_mutate_category
#undef vsm_detail_fetch_mutate_ref
