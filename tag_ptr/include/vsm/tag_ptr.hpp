#pragma once

#include <vsm/assert.h>

#include <bit>
#include <limits>
#include <memory>
#include <type_traits>

#include <cstdint>

namespace vsm {
namespace detail {

template<typename T, typename Tag, Tag Max>
class tag_ptr;

template<typename T>
concept intptr = std::integral<T> && (sizeof(T) == sizeof(uintptr_t));

template<typename T, typename Tag>
consteval Tag tag_ptr_max_tag()
{
	/**/ if constexpr (std::is_same_v<Tag, bool>)
	{
		return alignof(T) > 1;
	}
	else if constexpr (std::is_enum_v<Tag>)
	{
		return static_cast<Tag>(tag_ptr_max_tag<T, std::underlying_type_t<Tag>>());
	}
	else if constexpr (std::is_signed_v<Tag>)
	{
		return static_cast<Tag>(tag_ptr_max_tag<T, std::make_unsigned_t<Tag>>() / 2);
	}
	else
	{
		uintptr_t max = alignof(T) - 1;
		if (max > std::numeric_limits<Tag>::max())
		{
			max = std::numeric_limits<Tag>::max();
		}
		return max;
	}
}

template<typename T, typename Tag>
consteval bool tag_ptr_check_tag(Tag const max)
{
	/**/ if constexpr (std::is_same_v<Tag, bool>)
	{
		return tag_ptr_check_tag<T>(static_cast<unsigned char>(max));
	}
	else if constexpr (std::is_enum_v<Tag>)
	{
		return tag_ptr_check_tag<T>(static_cast<std::underlying_type_t<Tag>>(max));
	}
	else
	{
		return max > 0 && max <= tag_ptr_max_tag<T, Tag>();
	}
}

template<typename In, typename InTag, InTag InMax, typename Out, typename OutTag, OutTag OutMax>
auto static_cast_constraint(tag_ptr<In, InTag, InMax>, tag_ptr<Out, OutTag, OutMax>**) -> tag_ptr<Out, OutTag, OutMax>
	requires
		requires (In* in, InTag in_tag)
		{
			static_cast<Out*>(in);
			static_cast<OutTag>(in_tag);
		}
		&& (static_cast<OutTag>(InMax) <= OutMax);

template<typename In, typename Tag, Tag Max, typename Out>
auto const_cast_constraint(tag_ptr<In, Tag, Max>, tag_ptr<Out, Tag, Max>**) -> tag_ptr<Out, Tag, Max>
	requires requires (In* in) { const_cast<Out*>(in); };

template<typename T, typename Tag, Tag Max, intptr Integer>
auto reinterpret_cast_constraint_ptr(tag_ptr<T, Tag, Max>, Integer**) -> Integer;

template<typename In, typename Tag, Tag Max, typename Out>
auto reinterpret_cast_constraint_ptr(tag_ptr<In, Tag, Max>, tag_ptr<Out, Tag, Max>**) -> tag_ptr<Out, Tag, Max>
	requires requires (In* in) { reinterpret_cast<Out*>(in); };

template<typename T, typename Tag, Tag Max, intptr Integer>
auto reinterpret_cast_constraint_int(Integer**, tag_ptr<T, Tag, Max>**) -> tag_ptr<T, Tag, Max>;

template<typename In, typename Tag, Tag Max, typename Out>
auto dynamic_cast_constraint(tag_ptr<In, Tag, Max>, tag_ptr<Out, Tag, Max>**) -> tag_ptr<Out, Tag, Max>
	requires requires (In* in) { dynamic_cast<Out*>(in); };

} // namespace detail

template<typename Out, typename In, typename InTag, InTag InMax>
auto static_pointer_cast(detail::tag_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::static_cast_constraint(ptr, static_cast<Out**>(0)))
{
	return Out(static_cast<typename Out::element_type*>(ptr.ptr()), ptr.tag());
}

template<typename Out, typename In, typename InTag, InTag InMax>
auto const_pointer_cast(detail::tag_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::const_cast_constraint(ptr, static_cast<Out**>(0)))
{
	return Out(ptr.m_value);
}

template<typename Out, typename In, typename InTag, InTag InMax>
auto reinterpret_pointer_cast(detail::tag_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::reinterpret_cast_constraint_ptr(ptr, static_cast<Out**>(0)))
{
	return Out(ptr.m_value);
}

template<typename Out, typename In>
auto reinterpret_pointer_cast(In const in)
	-> decltype(detail::reinterpret_cast_constraint_int(static_cast<In**>(0), static_cast<Out**>(0)))
{
	return Out(in);
}

template<typename Out, typename In, typename InTag, InTag InMax>
auto dynamic_pointer_cast(detail::tag_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::dynamic_cast_constraint(ptr, static_cast<Out**>(0)))
{
	return Out(dynamic_cast<typename Out::element_type*>(ptr.ptr()), ptr.tag());
}

template<typename T, typename Tag, Tag Max>
class detail::tag_ptr
{
	// Laundering through integral_constant allows the Visual Studio debugger to see the value.
	static constexpr uintptr_t tag_mask = std::integral_constant<uintptr_t,
		(static_cast<uintptr_t>(-1) >> std::countl_zero(static_cast<uintptr_t>(Max)))>::value;

	static constexpr uintptr_t ptr_mask = ~tag_mask;

public:
	using element_type = T;
	using tag_type = Tag;

	static constexpr Tag max_tag = Max;

private:
	uintptr_t m_value;

public:
	tag_ptr() = default;

	constexpr tag_ptr(decltype(nullptr))
		: m_value(0)
	{
	}

	tag_ptr(T* const ptr)
		: m_value(reinterpret_cast<uintptr_t>(ptr))
	{
	}

	tag_ptr(T* const ptr, Tag const tag)
		: m_value(reinterpret_cast<uintptr_t>(ptr) | static_cast<uintptr_t>(tag))
	{
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask);
	}

	template<typename OtherT, typename OtherTag, OtherTag OtherMax>
	tag_ptr(tag_ptr<OtherT, OtherTag, OtherMax> const& other)
		requires std::is_convertible_v<OtherT*, T*> && std::is_convertible_v<OtherTag, Tag> && (static_cast<Tag>(OtherMax) <= Max)
		: m_value(other.m_value)
	{
	}

	tag_ptr(tag_ptr const&) = default;
	tag_ptr& operator=(tag_ptr const&) & = default;


	T* ptr() const
	{
		return reinterpret_cast<T*>(m_value & ptr_mask);
	}

	void set_ptr(T* const ptr)
	{
		m_value = reinterpret_cast<uintptr_t>(ptr) | (m_value & tag_mask);
	}

	Tag tag() const
	{
		return static_cast<Tag>(m_value & tag_mask);
	}

	void set_tag(Tag const tag)
	{
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask);
		m_value = (m_value & ptr_mask) | static_cast<uintptr_t>(tag);
	}

	void set(T* const ptr, Tag const tag)
	{
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask);
		m_value = reinterpret_cast<uintptr_t>(ptr) | static_cast<uintptr_t>(tag);
	}

	bool is_zero() const
	{
		return m_value == 0;
	}


	T& operator*() const
	{
		return *reinterpret_cast<T*>(m_value & ptr_mask);
	}

	T* operator->() const
	{
		return reinterpret_cast<T*>(m_value & ptr_mask);
	}

	T& operator[](ptrdiff_t const index) const
	{
		return reinterpret_cast<T*>(m_value & ptr_mask)[index];
	}


	explicit operator bool() const
	{
		return (m_value & ptr_mask) != 0;
	}


	friend bool operator==(tag_ptr const&, tag_ptr const&) = default;

	friend constexpr bool operator==(tag_ptr const& ptr, decltype(nullptr))
	{
		return (ptr.m_value & ptr_mask) == 0;
	}

private:
	explicit tag_ptr(uintptr_t const value)
		: m_value(value)
	{
	}

	template<typename OtherT, typename OtherTag, OtherTag OtherMax>
	friend class tag_ptr;

	template<typename Out, typename In, typename InTag, InTag InMax>
	friend auto vsm::const_pointer_cast(detail::tag_ptr<In, InTag, InMax> const ptr)
		-> decltype(detail::const_cast_constraint(ptr, static_cast<Out**>(0)));

	template<typename Out, typename In, typename InTag, InTag InMax>
	friend auto vsm::reinterpret_pointer_cast(detail::tag_ptr<In, InTag, InMax> const ptr)
		-> decltype(detail::reinterpret_cast_constraint_ptr(ptr, static_cast<Out**>(0)));

	template<typename Out, typename In>
	friend auto vsm::reinterpret_pointer_cast(In const in)
		-> decltype(detail::reinterpret_cast_constraint_int(static_cast<In**>(0), static_cast<Out**>(0)));
};

template<typename T, typename Tag, Tag Max>
using incomplete_tag_ptr = detail::tag_ptr<T, Tag, Max>;

/// @brief A pointer with a secondary tag value stored in the alignment bits.
/// @tparam T Type of the object pointed to by the pointer.
/// @tparam Tag Type of the secondary tag value.
/// @tparam Max Maximum value of the secondary tag.
/// The default maximum is based on the alignment of @tparam T.
template<typename T, typename Tag, Tag Max = detail::tag_ptr_max_tag<T, Tag>()>
	requires (detail::tag_ptr_check_tag<T>(Max))
using tag_ptr = detail::tag_ptr<T, Tag, Max>;

template<typename T>
consteval bool check_incomplete_tag_ptr()
{
	return detail::tag_ptr_check_tag<typename T::element_type>(T::max_tag);
}

} // namespace vsm

template<typename T, typename Tag, Tag Max>
struct std::pointer_traits<vsm::tag_ptr<T, Tag, Max>>
{
	using pointer = vsm::tag_ptr<T, Tag, Max>;
	using element_type = T;
	using difference_type = ptrdiff_t;

	template<typename U>
	using rebind = vsm::tag_ptr<T, Tag, Max>;

	static pointer pointer_to(T* const ptr)
	{
		return ptr;
	}

	static T* to_address(pointer const ptr)
	{
		return ptr.ptr();
	}
};
