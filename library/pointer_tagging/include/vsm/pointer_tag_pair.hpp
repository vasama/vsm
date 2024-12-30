#pragma once

#include <vsm/assert.h>
#include <vsm/atomic.hpp>
#include <vsm/concepts.hpp>
#include <vsm/standard/memory.hpp>

#include <bit>
#include <compare>
#include <utility>

namespace vsm {
namespace detail {

template<typename T>
concept taggable_pointee = !std::is_function_v<T>;

template<typename Pointer, typename T>
concept taggable_pointer = std::is_pointer_v<Pointer> && std::convertible_to<Pointer, T*>;

template<typename Tag>
concept taggable_pointer_tag =
	std::unsigned_integral<Tag> ||
	std::is_enum_v<Tag> && std::unsigned_integral<std::underlying_type_t<Tag>>;

template<typename Tag>
concept taggable_pointer_tag_arithmetic =
	// std::unsigned_integral<Tag> &&
	vsm::not_same_as<Tag, bool>;

template<typename Tag>
concept taggable_pointer_tag_bitwise = true;

template<typename T, unsigned Alignment = 0>
consteval unsigned taggable_bits_available()
{
	if constexpr (Alignment != 0)
	{
		return std::countr_zero(Alignment);
	}
	else
	{
		if constexpr (std::is_void_v<T>)
		{
			return 0;
		}
		else
		{
			return std::countr_zero(alignof(T));
		}
	}
}

} // namespace detail

template<
	detail::taggable_pointee T,
	detail::taggable_pointer_tag Tag,
	unsigned BitsRequested = detail::taggable_bits_available<T>()>
class pointer_tag_pair
{
	static constexpr uintptr_t ptr_mask = static_cast<uintptr_t>(-1) << BitsRequested;
	static constexpr uintptr_t tag_mask = (static_cast<uintptr_t>(1) << BitsRequested) - 1;
	static constexpr size_t alignment_required = static_cast<size_t>(1) << BitsRequested;

	uintptr_t m_value;

public:
	using pointer_type = T*;
	using tagged_pointer_type = vsm::copy_cv_t<T, void>*;
	using tag_type = Tag;

	static constexpr unsigned bits_requested = BitsRequested;


	constexpr pointer_tag_pair()
		: m_value(0)
	{
	}

	/*constexpr*/ pointer_tag_pair(decltype(nullptr), Tag const tag)
		: pointer_tag_pair(static_cast<uintptr_t>(tag))
	{
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask); //PRECONDITION
	}

	template<detail::taggable_pointer<T> P>
		requires (BitsRequested <= detail::taggable_bits_available<remove_ptr_t<P>>())
	/*constexpr*/ pointer_tag_pair(P const pointer, Tag const tag)
		: pointer_tag_pair(reinterpret_cast<uintptr_t>(pointer) | static_cast<uintptr_t>(tag))
	{
		vsm_assert(vsm::is_sufficiently_aligned<alignment_required>(pointer)); //PRECONDITION
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask); //PRECONDITION
	}

	template<unsigned PromisedAlignment, detail::taggable_pointer<T> P>
		requires (BitsRequested <= detail::taggable_bits_available<remove_ptr_t<P>, PromisedAlignment>())
	static /*constexpr*/ pointer_tag_pair from_overaligned(P const pointer, Tag const tag)
	{
		vsm_assert(vsm::is_sufficiently_aligned<PromisedAlignment>(pointer)); //PRECONDITION
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask); //PRECONDITION

		return pointer_tag_pair(reinterpret_cast<uintptr_t>(pointer) | static_cast<uintptr_t>(tag));
	}

	[[nodiscard]] static pointer_tag_pair from_tagged(tagged_pointer_type const pointer) noexcept
	{
		return pointer_tag_pair(reinterpret_cast<uintptr_t>(pointer));
	}

	[[nodiscard]] tagged_pointer_type tagged_pointer() const noexcept
	{
		return reinterpret_cast<tagged_pointer_type>(m_value);
	}

	[[nodiscard]] /*constexpr*/ T* pointer() const noexcept
	{
		return reinterpret_cast<T*>(m_value & ptr_mask);
	}

	[[nodiscard]] constexpr Tag tag() const noexcept
	{
		return static_cast<Tag>(m_value & tag_mask);
	}

	friend constexpr void swap(pointer_tag_pair& lhs, pointer_tag_pair& rhs) noexcept
	{
		auto const lhs_value = lhs.m_value;
		lhs.m_value = rhs.m_value;
		rhs.m_value = lhs_value;
	}

	[[nodiscard]] friend constexpr auto operator<=>(
		pointer_tag_pair const&,
		pointer_tag_pair const&) = default;

private:
	explicit pointer_tag_pair(uintptr_t const value)
		: m_value(value)
	{
	}

	friend atomic<pointer_tag_pair<T, Tag, BitsRequested>>;
	friend atomic_ref<pointer_tag_pair<T, Tag, BitsRequested>>;
};

template<typename T, typename Tag, unsigned BitsRequested>
class atomic_ref<pointer_tag_pair<T, Tag, BitsRequested>> : atomic_ref<uintptr_t>
{
	using base = atomic_ref<uintptr_t>;

public:
	using value_type = pointer_tag_pair<T, Tag, BitsRequested>;

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

template<typename T, typename Tag, unsigned BitsRequested>
class atomic<pointer_tag_pair<T, Tag, BitsRequested>> : atomic<uintptr_t>
{
	using base = atomic<uintptr_t>;

public:
	using value_type = pointer_tag_pair<T, Tag, BitsRequested>;

	using base::is_always_lock_free;

	atomic() = default;

	constexpr atomic(value_type const value) noexcept
		: base(value.m_value)
	{
	}

	using base::is_lock_free;

// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define vsm_detail_fetch_mutate_category &
#define vsm_detail_fetch_mutate_ref (atomic_ref<uintptr_t>(static_cast<base&>(*this)))
#include <vsm/detail/atomic_pointer_tag_pair.hpp>
};

template<size_t I, typename T, typename Tag, unsigned BitsRequested>
[[nodiscard]] constexpr std::tuple_element_t<I, pointer_tag_pair<T, Tag, BitsRequested>> get(
	pointer_tag_pair<T, Tag, BitsRequested> const pointer)
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

template<detail::taggable_pointee T, detail::taggable_pointer_tag Tag, Tag Max>
using pointer_tag_pair_with_max = pointer_tag_pair<
	T,
	Tag,
	static_cast<unsigned>(std::bit_width(static_cast<uintptr_t>(Max)))>;

} // namespace vsm

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<typename T, typename Tag, unsigned BitsRequested>
struct std::tuple_size<vsm::pointer_tag_pair<T, Tag, BitsRequested>>
{
	static constexpr size_t value = 2;
};

template<typename T, typename Tag, unsigned BitsRequested>
struct std::tuple_size<vsm::pointer_tag_pair<T, Tag, BitsRequested> const>
{
	static constexpr size_t value = 2;
};

template<typename T, typename Tag, unsigned BitsRequested>
struct std::tuple_element<0, vsm::pointer_tag_pair<T, Tag, BitsRequested>>
{
	using type = T*;
};

template<typename T, typename Tag, unsigned BitsRequested>
struct std::tuple_element<1, vsm::pointer_tag_pair<T, Tag, BitsRequested>>
{
	using type = Tag;
};

template<typename T, typename Tag, unsigned BitsRequested>
struct std::tuple_element<0, vsm::pointer_tag_pair<T, Tag, BitsRequested> const>
{
	using type = T*;
};

template<typename T, typename Tag, unsigned BitsRequested>
struct std::tuple_element<1, vsm::pointer_tag_pair<T, Tag, BitsRequested> const>
{
	using type = Tag;
};

// NOLINTEND(bugprone-std-namespace-modification)
