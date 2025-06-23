#pragma once

#include <vsm/concepts.hpp>
#include <vsm/standard/memory.hpp>

#include <bit>
#include <utility>

namespace vsm {
namespace detail {

template<typename Pointer>
concept taggable_pointer = std::is_pointer_v<Pointer> && !std::is_function_v<vsm::remove_ptr_t<Pointer>>;

template<typename Tag>
concept taggable_pointer_tag =
	std::unsigned_integral<Tag> ||
	std::is_enum_v<Tag> && std::unsigned_integral<std::underlying_type_t<Tag>>;

template<typename Tag>
concept taggable_pointer_tag_arithmetic = std::unsigned_integral<Tag> && vsm::not_same_as<Tag, bool>;

} // namespace detail

template<typename Pointer>
struct pointer_tag_traits;

template<typename Pointee>
	requires std::is_object_v<Pointee>
struct pointer_tag_traits<Pointee*>
{
	template<unsigned Alignment = alignof(Pointee)>
		requires (std::has_single_bit(Alignment))
	static constexpr unsigned bits_available = std::countr_zero(Alignment);
};

template<>
struct pointer_tag_traits<void*>
{
	template<unsigned Alignment = 1>
		requires (std::has_single_bit(Alignment))
	static constexpr unsigned bits_available = std::countr_zero(Alignment);
};

template<typename Pointee>
struct pointer_tag_traits<Pointee const*> : pointer_tag_traits<Pointee*> {};

template<
	detail::taggable_pointer Pointer,
	detail::taggable_pointer_tag Tag,
	unsigned BitsRequested = (pointer_tag_traits<Pointer>::template bits_available<>)>
class pointer_tag_pair
{
	static constexpr uintptr_t ptr_mask = static_cast<uintptr_t>(-1) << BitsRequested;

	static constexpr uintptr_t max_value = BitsRequested ? static_cast<uintptr_t>(1) << (BitsRequested - 1) : 0;
	static constexpr uintptr_t tag_mask = BitsRequested ? max_value - 1 : 0;

	uintptr_t m_value;

public:
	using pointer_type = Pointer;
	using tagged_pointer_type = vsm::copy_cv_t<vsm::remove_ptr_t<Pointer>, void>*;
	using tag_type = Tag;

	static constexpr unsigned bits_requested = BitsRequested;


	constexpr pointer_tag_pair()
		: m_value(0)
	{
	}

	template<std::convertible_to<Pointer> P>
		requires (pointer_tag_traits<P>::template bits_available<> >= BitsRequested)
	/*constexpr*/ pointer_tag_pair(P const pointer, Tag const tag)
		: pointer_tag_pair(reinterpret_cast<uintptr_t>(pointer) | static_cast<uintptr_t>(tag))
	{
		vsm_assert(vsm::is_sufficiently_aligned<BitsRequested>(pointer)); //PRECONDITION
		vsm_assert(static_cast<uintptr_t>(tag) <= max_value);
	}

	template<unsigned PromisedAlignment, std::convertible_to<Pointer> P>
		requires (pointer_tag_traits<P>::template bits_available<PromisedAlignment> >= BitsRequested)
	static /*constexpr*/ pointer_tag_pair from_overaligned(P const pointer, Tag const tag)
	{
		vsm_assert(vsm::is_sufficiently_aligned<PromisedAlignment>(pointer)); //PRECONDITION
		vsm_assert(static_cast<uintptr_t>(tag) <= max_value); //PRECONDITION

		return pointer_tag_pair(reinterpret_cast<uintptr_t>(pointer) | static_cast<uintptr_t>(tag));
	}

	static pointer_tag_pair from_tagged(tagged_pointer_type const pointer) noexcept
	{
		return pointer_tag_pair(reinterpret_cast<uintptr_t>(pointer));
	}

	[[nodiscard]] tagged_pointer_type tagged_pointer() const noexcept
	{
		return reinterpret_cast<tagged_pointer_type>(m_value);
	}

	[[nodiscard]] /*constexpr*/ Pointer pointer() const noexcept
	{
		return reinterpret_cast<Pointer>(m_value & ptr_mask);
	}

	[[nodiscard]] constexpr Tag tag() const noexcept
	{
		return static_cast<Tag>(m_value & tag_mask);
	}

	friend constexpr void swap(pointer_tag_pair& lhs, pointer_tag_pair& rhs) noexcept
	{
		auto const lhs_value = lhs.value;
		lhs.value = rhs.value;
		rhs.value = lhs_value;
	}

	friend constexpr auto operator<=>(pointer_tag_pair const&, pointer_tag_pair const&) = default;

private:
	explicit pointer_tag_pair(uintptr_t const value)
		: m_value(value)
	{
	}

	friend atomic<pointer_tag_pair<Pointer, Tag, BitsRequested>>;
	friend atomic_ref<pointer_tag_pair<Pointer, Tag, BitsRequested>>;
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
class atomic_ref<pointer_tag_pair<Pointer, Tag, BitsRequested>> : atomic_ref<uintptr_t>
{
	using base = atomic_ref<uintptr_t>;

public:
	using value_type = typename base::value_type;

	using base::is_always_lock_free;
	using base::required_alignment;

	explicit atomic_ref(value_type& value)
		: base(value.m_value)
	{
	}

	using base::is_lock_free;

#define vsm_detail_fetch_mutate_category const
#define vsm_detail_fetch_mutate_ref (static_cast<base const&>(*this))
#include <vsm/detail/atomic_pointer_tag_pair.hpp>
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
class atomic<pointer_tag_pair<Pointer, Tag, BitsRequested>> : atomic<uintptr_t>
{
	using base = atomic<uintptr_t>;

public:
	using value_type = typename base::value_type;

	using base::is_always_lock_free;

	atomic() = default;

	constexpr atomic(value_type const value) noexcept
		: base(value.m_value)
	{
	}

	using base::is_lock_free;

#define vsm_detail_fetch_mutate_category &
#define vsm_detail_fetch_mutate_ref (ref_type(static_cast<base&>(*this)))
#include <vsm/detail/atomic_pointer_tag_pair.hpp>
};

template<size_t I, typename Pointer, typename Tag, unsigned BitsRequested>
constexpr std::tuple_element_t<I, pointer_tag_pair<Pointer, Tag, BitsRequested>> get(
	pointer_tag_pair<Pointer, Tag, BitsRequested> const pointer)
{
	if constexpr (I == 0)
	{
		return pointer.pointer();
	}
	else
	{
		return pointer.tag();
	}
}


template<detail::taggable_pointer Pointer, detail::taggable_pointer_tag Tag, Tag Max>
using pointer_tag_pair_with_max = pointer_tag_pair<
	Pointer,
	Tag,
	std::bit_width(static_cast<uintptr_t>(Max))>;

} // namespace vsm

template<typename Pointer, typename Tag, unsigned BitsRequested>
struct std::tuple_size<vsm::pointer_tag_pair<Pointer, Tag, BitsRequested>>
{
	static constexpr size_t value = 2;
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
struct std::tuple_size<vsm::pointer_tag_pair<Pointer, Tag, BitsRequested> const>
{
	static constexpr size_t value = 2;
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
struct std::tuple_element<0, vsm::pointer_tag_pair<Pointer, Tag, BitsRequested>>
{
	using type = Pointer;
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
struct std::tuple_element<1, vsm::pointer_tag_pair<Pointer, Tag, BitsRequested>>
{
	using type = Tag;
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
struct std::tuple_element<0, vsm::pointer_tag_pair<Pointer, Tag, BitsRequested> const>
{
	using type = Pointer;
};

template<typename Pointer, typename Tag, unsigned BitsRequested>
struct std::tuple_element<1, vsm::pointer_tag_pair<Pointer, Tag, BitsRequested> const>
{
	using type = Tag;
};
