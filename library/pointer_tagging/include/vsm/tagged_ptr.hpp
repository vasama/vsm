#pragma once

//TODO: Rename to tagged_ptr

#include <vsm/assert.h>
#include <vsm/hash.hpp>

#include <bit>
#include <limits>
#include <memory>
#include <type_traits>

#include <cstdint>

namespace vsm {
namespace detail {

template<typename T, typename Tag, Tag Max>
class tagged_ptr;

template<typename T>
concept intptr = std::integral<T> && (sizeof(T) == sizeof(uintptr_t));

template<typename T, typename Tag>
consteval Tag tagged_ptr_max_tag()
{
	// NOLINTBEGIN(readability-misleading-indentation)

	/**/ if constexpr (std::is_same_v<Tag, bool>)
	{
		return alignof(T) > 1;
	}
	else if constexpr (std::is_enum_v<Tag>)
	{
		return static_cast<Tag>(tagged_ptr_max_tag<T, std::underlying_type_t<Tag>>());
	}
	else if constexpr (std::is_signed_v<Tag>)
	{
		return static_cast<Tag>(tagged_ptr_max_tag<T, std::make_unsigned_t<Tag>>() / 2);
	}
	else
	{
		uintptr_t max = alignof(T) - 1;
		if (max > std::numeric_limits<Tag>::max())
		{
			max = std::numeric_limits<Tag>::max();
		}
		return static_cast<Tag>(max);
	}

	// NOLINTEND(readability-misleading-indentation)
}

template<typename T, typename Tag>
consteval bool tagged_ptr_check_tag(Tag const max)
{
	// NOLINTBEGIN(readability-misleading-indentation)

	/**/ if constexpr (std::is_same_v<Tag, bool>)
	{
		return tagged_ptr_check_tag<T>(static_cast<unsigned char>(max));
	}
	else if constexpr (std::is_enum_v<Tag>)
	{
		return tagged_ptr_check_tag<T>(static_cast<std::underlying_type_t<Tag>>(max));
	}
	else
	{
		// NOLINTNEXTLINE(misc-redundant-expression)
		return 0 < max && max <= tagged_ptr_max_tag<T, Tag>();
	}

	// NOLINTEND(readability-misleading-indentation)
}

template<typename In, typename InTag, InTag InMax, typename Out, typename OutTag, OutTag OutMax>
auto tagged_ptr_static_cast(tagged_ptr<In, InTag, InMax>, tagged_ptr<Out, OutTag, OutMax>**)
	-> tagged_ptr<Out, OutTag, OutMax>
	requires requires (In* in, InTag in_tag)
	{
		static_cast<Out*>(in);
		static_cast<OutTag>(in_tag);
	}
	&& (static_cast<OutTag>(InMax) <= OutMax);

template<typename In, typename Tag, Tag Max, typename Out>
tagged_ptr<Out, Tag, Max>
tagged_ptr_const_cast(tagged_ptr<In, Tag, Max>, tagged_ptr<Out, Tag, Max>**)
	requires requires (In* in) { const_cast<Out*>(in); };

template<typename T, typename Tag, Tag Max, intptr Integer>
auto tagged_ptr_reinterpret_cast_ptr(tagged_ptr<T, Tag, Max>, Integer**) -> Integer;

template<typename In, typename Tag, Tag Max, typename Out>
tagged_ptr<Out, Tag, Max>
tagged_ptr_reinterpret_cast_ptr(tagged_ptr<In, Tag, Max>, tagged_ptr<Out, Tag, Max>**)
	requires requires (In* in) { reinterpret_cast<Out*>(in); };

template<typename T, typename Tag, Tag Max, intptr Integer>
tagged_ptr<T, Tag, Max>
tagged_ptr_reinterpret_cast_int(Integer**, tagged_ptr<T, Tag, Max>**);

template<typename In, typename Tag, Tag Max, typename Out>
tagged_ptr<Out, Tag, Max>
tagged_ptr_dynamic_cast(tagged_ptr<In, Tag, Max>, tagged_ptr<Out, Tag, Max>**)
	requires requires (In* in) { dynamic_cast<Out*>(in); };

} // namespace detail

template<typename Out, typename In, typename InTag, InTag InMax>
auto static_pointer_cast(detail::tagged_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::tagged_ptr_static_cast(ptr, static_cast<Out**>(nullptr)))
{
	return Out(static_cast<typename Out::element_type*>(ptr.ptr()), ptr.tag());
}

template<typename Out, typename In, typename InTag, InTag InMax>
auto const_pointer_cast(detail::tagged_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::tagged_ptr_const_cast(ptr, static_cast<Out**>(nullptr)))
{
	return Out(ptr.m_value);
}

template<typename Out, typename In, typename InTag, InTag InMax>
auto reinterpret_pointer_cast(detail::tagged_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::tagged_ptr_reinterpret_cast_ptr(ptr, static_cast<Out**>(nullptr)))
{
	return Out(ptr.m_value);
}

template<typename Out, typename In>
auto reinterpret_pointer_cast(In const in)
	-> decltype(detail::tagged_ptr_reinterpret_cast_int(
		static_cast<In**>(nullptr),
		static_cast<Out**>(nullptr)))
{
	return Out(in);
}

template<typename Out, typename In, typename InTag, InTag InMax>
auto dynamic_pointer_cast(detail::tagged_ptr<In, InTag, InMax> const ptr)
	-> decltype(detail::tagged_ptr_dynamic_cast(ptr, static_cast<Out**>(nullptr)))
{
	return Out(dynamic_cast<typename Out::element_type*>(ptr.ptr()), ptr.tag());
}

template<typename T, typename Tag, Tag Max>
class detail::tagged_ptr
{
	// Laundering through integral_constant allows the Visual Studio debugger to see the value.
	static constexpr uintptr_t tag_mask = std::integral_constant<
		uintptr_t,
		(static_cast<uintptr_t>(-1) >> std::countl_zero(static_cast<uintptr_t>(Max)))>::value;

	static constexpr uintptr_t ptr_mask = ~tag_mask;

public:
	using element_type = T;
	using tag_type = Tag;

	static constexpr Tag max_tag = Max;

private:
	uintptr_t m_value;

public:
	tagged_ptr() = default;

	constexpr tagged_ptr(decltype(nullptr))
		: m_value(0)
	{
	}

	tagged_ptr(T* const ptr)
		: m_value(reinterpret_cast<uintptr_t>(ptr))
	{
	}

	tagged_ptr(T* const ptr, Tag const tag)
		: m_value(reinterpret_cast<uintptr_t>(ptr) | static_cast<uintptr_t>(tag))
	{
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask);
	}

	template<typename OtherT, typename OtherTag, OtherTag OtherMax>
	tagged_ptr(tagged_ptr<OtherT, OtherTag, OtherMax> const& other)
		requires
			std::convertible_to<OtherT*, T*> &&
			losslessly_convertible_to<OtherTag, Tag> &&
			(static_cast<Tag>(OtherMax) <= Max)
		: m_value(other.m_value)
	{
	}

	tagged_ptr(tagged_ptr const&) = default;
	tagged_ptr& operator=(tagged_ptr const&) & = default;


	[[nodiscard]] T* ptr() const
	{
		return reinterpret_cast<T*>(m_value & ptr_mask);
	}

	void set_ptr(T* const ptr)
	{
		m_value = reinterpret_cast<uintptr_t>(ptr) | (m_value & tag_mask);
	}

	[[nodiscard]] Tag tag() const
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

	[[nodiscard]] bool is_zero() const
	{
		return m_value == 0;
	}


	[[nodiscard]] std::add_lvalue_reference_t<T> operator*() const
	{
		if constexpr (!std::is_void_v<T>)
		{
			return *reinterpret_cast<T*>(m_value & ptr_mask);
		}
	}

	[[nodiscard]] T* operator->() const
	{
		return reinterpret_cast<T*>(m_value & ptr_mask);
	}

	[[nodiscard]] std::add_lvalue_reference_t<T> operator[](ptrdiff_t const index) const
	{
		return reinterpret_cast<T*>(m_value & ptr_mask)[index];
	}


	[[nodiscard]] explicit operator bool() const
	{
		return (m_value & ptr_mask) != 0;
	}


	[[nodiscard]] friend bool operator==(tagged_ptr const&, tagged_ptr const&) = default;

	[[nodiscard]] friend constexpr bool operator==(tagged_ptr const& ptr, decltype(nullptr))
	{
		return (ptr.m_value & ptr_mask) == 0;
	}

private:
	explicit tagged_ptr(uintptr_t const value)
		: m_value(value)
	{
	}

	template<typename OtherT, typename OtherTag, OtherTag OtherMax>
	friend class tagged_ptr;

	// NOLINTBEGIN(readability-avoid-const-params-in-decls)

	template<typename Out, typename In, typename InTag, InTag InMax>
	friend auto vsm::const_pointer_cast(detail::tagged_ptr<In, InTag, InMax> const ptr)
		-> decltype(detail::tagged_ptr_const_cast(ptr, static_cast<Out**>(nullptr)));

	template<typename Out, typename In, typename InTag, InTag InMax>
	friend auto vsm::reinterpret_pointer_cast(detail::tagged_ptr<In, InTag, InMax> const ptr)
		-> decltype(detail::tagged_ptr_reinterpret_cast_ptr(ptr, static_cast<Out**>(nullptr)));

	template<typename Out, typename In>
	friend auto vsm::reinterpret_pointer_cast(In const in)
		-> decltype(detail::tagged_ptr_reinterpret_cast_int(
			static_cast<In**>(nullptr),
			static_cast<Out**>(nullptr)));

	// NOLINTEND(readability-avoid-const-params-in-decls)
};

template<typename T, typename Tag, Tag Max>
inline constexpr bool is_trivially_hashable_v<detail::tagged_ptr<T, Tag, Max>> = true;

template<typename T, typename Tag, Tag Max>
using incomplete_tagged_ptr = detail::tagged_ptr<T, Tag, Max>;

/// @brief A pointer with a secondary tag value stored in the alignment bits.
/// @tparam T Type of the object pointed to by the pointer.
/// @tparam Tag Type of the secondary tag value.
/// @tparam Max Maximum value of the secondary tag.
/// The default maximum is based on the alignment of @tparam T.
template<typename T, typename Tag, Tag Max = detail::tagged_ptr_max_tag<T, Tag>()>
	requires (detail::tagged_ptr_check_tag<T>(Max))
using tagged_ptr = detail::tagged_ptr<T, Tag, Max>;

template<typename T>
[[nodiscard]] consteval bool check_incomplete_tagged_ptr()
{
	return detail::tagged_ptr_check_tag<typename T::element_type>(T::max_tag);
}

} // namespace vsm

template<typename T, typename Tag, Tag Max>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::pointer_traits<vsm::tagged_ptr<T, Tag, Max>>
{
	using pointer = vsm::tagged_ptr<T, Tag, Max>;
	using element_type = T;
	using difference_type = ptrdiff_t;

	template<typename U>
	using rebind = vsm::tagged_ptr<T, Tag, Max>;

	static pointer pointer_to(T* const ptr)
	{
		return ptr;
	}

	static T* to_address(pointer const ptr)
	{
		return ptr.ptr();
	}
};
