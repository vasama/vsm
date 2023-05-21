#pragma once

#include <vsm/assert.h>

#include <bit>
#include <limits>
#include <memory>
#include <type_traits>

#include <cstdint>

namespace vsm {

template<typename T, typename Tag, Tag Max>
class incomplete_tag_ptr;

namespace detail {

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
void static_cast_constraint(incomplete_tag_ptr<In, InTag, InMax> const&, incomplete_tag_ptr<Out, OutTag, OutMax>&)
	requires requires (In* in, InTag in_tag) { static_cast<Out*>(in); static_cast<OutTag>(in_tag); } && (static_cast<OutTag>(InMax) <= OutMax);

template<typename In, typename Tag, Tag Max, typename Out>
void const_cast_constraint(incomplete_tag_ptr<In, Tag, Max> const&, incomplete_tag_ptr<Out, Tag, Max>&)
	requires requires (In* in) { const_cast<Out*>(in); };

template<typename T, typename Tag, Tag Max>
void reinterpret_cast_constraint_ptr(incomplete_tag_ptr<T, Tag, Max> const&, intptr_t&);

template<typename T, typename Tag, Tag Max>
void reinterpret_cast_constraint_ptr(incomplete_tag_ptr<T, Tag, Max> const&, uintptr_t&);

template<typename In, typename Tag, Tag Max, typename Out>
void reinterpret_cast_constraint_ptr(incomplete_tag_ptr<In, Tag, Max> const&, incomplete_tag_ptr<Out, Tag, Max>&)
	requires requires (In* in) { reinterpret_cast<Out*>(in); };

template<typename T, typename Tag, Tag Max>
void reinterpret_cast_constraint_int(intptr_t&, incomplete_tag_ptr<T, Tag, Max>&);

template<typename T, typename Tag, Tag Max>
void reinterpret_cast_constraint_int(uintptr_t&, incomplete_tag_ptr<T, Tag, Max>&);

template<typename In, typename Tag, Tag Max, typename Out>
void dynamic_cast_constraint(incomplete_tag_ptr<In, Tag, Max> const&, incomplete_tag_ptr<Out, Tag, Max>&)
	requires requires (In* in) { dynamic_cast<Out*>(in); };

} // namespace detail

template<typename Out, typename In, typename InTag, InTag InMax>
Out static_pointer_cast(incomplete_tag_ptr<In, InTag, InMax> const ptr)
	requires requires (Out& out) { detail::static_cast_constraint(ptr, out); }
{
	return Out(static_cast<typename Out::element_type*>(ptr.ptr()), ptr.tag());
}

template<typename Out, typename In, typename InTag, InTag InMax>
Out const_pointer_cast(incomplete_tag_ptr<In, InTag, InMax> const ptr)
	requires requires (Out& out) { detail::const_cast_constraint(ptr, out); }
{
	return Out(ptr.m_value);
}

template<typename Out, typename In, typename InTag, InTag InMax>
Out reinterpret_pointer_cast(incomplete_tag_ptr<In, InTag, InMax> const ptr)
	requires requires (Out& out) { detail::reinterpret_cast_constraint_ptr(ptr, out); }
{
	return Out(ptr.m_value);
}

template<typename Out, typename In>
Out reinterpret_pointer_cast(In const value)
	requires requires (In& in, Out& out) { detail::reinterpret_cast_constraint_int(in, out); }
{
	return Out(value);
}

template<typename Out, typename In, typename InTag, InTag InMax>
Out dynamic_pointer_cast(incomplete_tag_ptr<In, InTag, InMax> const ptr)
	requires requires (Out& out) { detail::dynamic_cast_constraint(ptr, out); }
{
	return Out(dynamic_cast<typename Out::element_type*>(ptr.ptr()), ptr.tag());
}


template<typename T, typename Tag, Tag Max>
class incomplete_tag_ptr
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
	incomplete_tag_ptr() = default;

	constexpr incomplete_tag_ptr(decltype(nullptr))
		: m_value(0)
	{
	}

	incomplete_tag_ptr(T* const ptr)
		: m_value(reinterpret_cast<uintptr_t>(ptr))
	{
	}

	incomplete_tag_ptr(T* const ptr, Tag const tag)
		: m_value(reinterpret_cast<uintptr_t>(ptr) | static_cast<uintptr_t>(tag))
	{
		vsm_assert(static_cast<uintptr_t>(tag) <= tag_mask);
	}

	template<typename OtherT, typename OtherTag, OtherTag OtherMax>
	incomplete_tag_ptr(incomplete_tag_ptr<OtherT, OtherTag, OtherMax> const& other)
		requires std::is_convertible_v<OtherT*, T*> && std::is_convertible_v<OtherTag, Tag> && (static_cast<Tag>(OtherMax) <= Max)
		: m_value(other.m_value)
	{
	}

	incomplete_tag_ptr(incomplete_tag_ptr const&) = default;
	incomplete_tag_ptr& operator=(incomplete_tag_ptr const&) & = default;


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


	friend bool operator==(incomplete_tag_ptr const&, incomplete_tag_ptr const&) = default;

	friend constexpr bool operator==(incomplete_tag_ptr const& ptr, decltype(nullptr))
	{
		return (ptr.m_value & ptr_mask) == 0;
	}

private:
	explicit incomplete_tag_ptr(uintptr_t const value)
		: m_value(value)
	{
	}

	template<typename OtherT, typename OtherTag, OtherTag OtherMax>
	friend class incomplete_tag_ptr;

	template<typename Out, typename In, typename InTag, InTag InMax>
	friend Out const_pointer_cast(incomplete_tag_ptr<In, InTag, InMax> const ptr)
		requires requires (Out& out) { detail::const_cast_constraint(ptr, out); };

	template<typename Out, typename In, typename InTag, InTag InMax>
	friend Out reinterpret_pointer_cast(incomplete_tag_ptr<In, InTag, InMax> const ptr)
		requires requires (Out& out) { detail::reinterpret_cast_constraint_ptr(ptr, out); };

	template<typename Out, typename In>
	friend Out reinterpret_pointer_cast(In const value)
		requires requires (In& in, Out& out) { detail::reinterpret_cast_constraint_int(in, out); };
};

/// @brief A pointer with a secondary tag value stored in the alignment bits.
/// @tparam T Type of the object pointed to by the pointer.
/// @tparam Tag Type of the secondary tag value.
/// @tparam Max Maximum value of the secondary tag.
/// The default maximum is based on the alignment of @tparam T.
template<typename T, typename Tag, Tag Max = detail::tag_ptr_max_tag<T, Tag>()>
	requires (detail::tag_ptr_check_tag<T>(Max))
using tag_ptr = incomplete_tag_ptr<T, Tag, Max>;

template<typename T>
consteval bool check_incomplete_tag_ptr()
{
	return detail::tag_ptr_check_tag<typename T::element_type>(T::max_tag);
}

} // namespace vsm

template<typename T, typename Tag, Tag Max>
struct std::pointer_traits<vsm::incomplete_tag_ptr<T, Tag, Max>>
{
	using pointer = vsm::incomplete_tag_ptr<T, Tag, Max>;
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
