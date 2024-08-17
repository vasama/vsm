#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/exceptions.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>
#include <vsm/utility.hpp>

#include <exception>
#include <memory>

namespace vsm {
namespace casting {

struct member_t
{
	explicit member_t() = default;

	template<non_cvref To, typename From>
	static constexpr bool is(From const& from)
		requires requires (member_t const& tag)
		{
			{ To::is(tag, from) } -> std::convertible_to<bool>;
		}
	{
		return To::is(member_t(), from);
	}

	template<non_cvref To, typename From>
	static constexpr To const* cast(From const& from)
		requires requires (member_t const& tag)
		{
			{ To::cast(tag, from) } -> std::convertible_to<To const*>;
		}
	{
		return To::cast(member_t(), from);
	}

	template<non_cvref To, typename From>
	static constexpr To const* try_cast(From const& from)
		requires requires (member_t const& tag)
		{
			{ To::try_cast(tag, from) } -> std::convertible_to<To const*>;
		}
	{
		return To::try_cast(member_t(), from);
	}
};


template<non_cvref To, non_cvref From>
struct is_traits;

template<non_cvref To, non_cvref From>
struct cast_traits;

template<non_cvref To, non_cvref From>
struct try_cast_traits;

namespace detail {

template<typename T>
concept _pointer = requires
{
	typename std::pointer_traits<T>::pointer;
};

template<typename To, typename From>
concept _castable = requires (From const& from)
{
	is_traits<To, From>::test(from);
};

template<typename To, typename From>
concept _castable_pointer =
	_pointer<From> &&
	_castable<To, remove_cv_t<typename std::pointer_traits<From>::element_type>>;

template<typename To, typename From>
concept _explicitly_convertible = requires
{
	static_cast<To*>(static_cast<From*>(0));
};

template<typename Fancy, typename To, typename From>
Fancy to_pointer(From&& from, To* const to)
{
	if constexpr (std::is_same_v<Fancy, To*>)
	{
		return to;
	}
	else if constexpr (requires { Fancy(vsm_forward(from), to); })
	{
		return Fancy(vsm_forward(from), to);
	}
	else if constexpr (requires { std::pointer_traits<Fancy>::to_pointer(to); })
	{
		return std::pointer_traits<Fancy>::to_pointer(to);
	}
	else
	{
		return to;
	}
}


template<typename To, typename From>
concept _has_member_is = requires (From const& from) { member_t::is<To>(from); };

template<typename To, typename From>
concept _has_member_cast = requires (From const& from) { member_t::cast<To>(from); };

template<typename To, typename From>
concept _has_member_try_cast = requires (From const& from) { member_t::try_cast<To>(from); };

} // namespace detail

template<non_cvref To, non_cvref From>
[[nodiscard]] constexpr bool is(From const& from)
	requires detail::_castable<To, From>
{
	return is_traits<To, From>::test(*std::addressof(from));
}

template<non_cvref To, non_cvref From>
[[nodiscard]] constexpr bool is(From const& p_from)
	requires detail::_castable_pointer<To, From>
{
	using pointer_traits = std::pointer_traits<remove_cvref_t<From>>;
	using from_type = typename pointer_traits::element_type;

	return static_cast<From const&>(p_from)
		? is_traits<To, from_type>::test(*std::to_address(p_from))
		: false;
}


template<non_cvref To, typename From>
[[nodiscard]] constexpr copy_cvref_t<From&&, To> cast(From&& from)
	requires detail::_castable<To, remove_cvref_t<From>>
{
	using from_type = remove_cvref_t<From>;
	using to_element_type = copy_cv_t<remove_ref_t<From>, To>;
	using traits_type = cast_traits<To, from_type>;

	vsm_assert(
		is_traits<To, from_type>::test(static_cast<from_type const&>(from)) &&
		"The object is not an instance of the requested type.");

	To const* const p_to = traits_type::cast(static_cast<from_type const&>(from));
	return static_cast<copy_cvref_t<From&&, To>>(const_cast<to_element_type&>(*p_to));
}

template<non_cvref To, typename From>
[[nodiscard]] constexpr auto cast(From&& p_from)
	requires detail::_castable_pointer<To, remove_cvref_t<From>>
{
	using pointer_traits = std::pointer_traits<remove_cvref_t<From>>;
	using from_element_type = typename pointer_traits::element_type;
	using from_type = remove_cv_t<from_element_type>;
	using to_element_type = copy_cv_t<from_element_type, To>;
	using to_ptr_type = typename pointer_traits::template rebind<To>;

	if (static_cast<From const&>(p_from))
	{
		from_element_type& from = *std::to_address(static_cast<From const&>(p_from));
		from_type const& c_from = from;

		vsm_assert(
			is_traits<To, from_type>::test(c_from) &&
			"The object is not an instance of the requested type.");

		To const* const p_to = cast_traits<To, from_type>::cast(c_from);

		return detail::to_pointer<to_ptr_type>(
			vsm_forward(p_from),
			const_cast<to_element_type*>(p_to));
	}

	return to_ptr_type{};
}


class bad_cast : std::exception
{
};

template<non_cvref To, typename From>
[[nodiscard]] constexpr copy_cvref_t<From&&, To> try_cast(From&& from)
	requires detail::_castable<To, remove_cvref_t<From>>
{
	using from_type = remove_cvref_t<From>;
	using to_element_type = copy_cv_t<remove_ref_t<From>, To>;
	using traits_type = try_cast_traits<To, from_type>;

	if (To const* const p_to = traits_type::try_cast(static_cast<from_type const&>(from)))
	{
		return static_cast<copy_cvref_t<From&&, To>>(const_cast<to_element_type&>(*p_to));
	}

	vsm_except_throw_or_terminate(bad_cast());
}

template<non_cvref To, typename From>
[[nodiscard]] constexpr auto try_cast(From&& p_from)
	requires detail::_castable_pointer<To, remove_cvref_t<From>>
{
	using pointer_traits = std::pointer_traits<remove_cvref_t<From>>;
	using from_element_type = typename pointer_traits::element_type;
	using from_type = remove_cv_t<from_element_type>;
	using to_element_type = copy_cv_t<from_element_type, To>;
	using to_ptr_type = typename pointer_traits::template rebind<to_element_type>;

	if (static_cast<From const&>(p_from))
	{
		from_element_type& from = *std::to_address(static_cast<From const&>(p_from));
		from_type const& c_from = from;

		if constexpr (requires { try_cast_traits<To, from_type>::try_cast(c_from); })
		{
			if (To const* const p_to = try_cast_traits<To, from_type>::try_cast(c_from))
			{
				return detail::to_pointer<to_ptr_type>(
					vsm_forward(p_from),
					const_cast<to_element_type*>(p_to));
			}
		}
		else
		{
			static_assert(
				requires { cast_traits<To, from_type>::cast(c_from); },
				"Type implements is but neither cast nor try_cast.");

			if (is_traits<To, from_type>::test(c_from))
			{
				To const* const p_to = cast_traits<To, from_type>::cast(c_from);

				return detail::to_pointer<to_ptr_type>(
					vsm_forward(p_from),
					const_cast<to_element_type*>(p_to));
			}
		}
	}

	return to_ptr_type{};
}


template<typename To, typename From>
struct is_traits<To const, From> : is_traits<To, From>
{
};

template<typename To, std::derived_from<To> From>
struct is_traits<To, From>
{
	static constexpr bool test(From const&)
	{
		return true;
	}
};

template<typename To, typename From>
	requires detail::_has_member_is<To, From>
struct is_traits<To, From>
{
	static constexpr bool test(From const& from)
	{
		return member_t::is<To>(from);
	}
};

template<typename To, typename From>
	requires detail::_has_member_try_cast<To, From>
struct is_traits<To, From>
{
	static constexpr bool test(From const& from)
	{
		return member_t::try_cast<To>(from) != nullptr;
	}
};


template<typename To, typename From>
	requires detail::_explicitly_convertible<To, From>
struct cast_traits<To, From>
{
	static To const* cast(From const& from)
	{
		return static_cast<To const*>(std::addressof(from));
	}
};

template<typename To, typename From>
	requires
		detail::_has_member_cast<To, From> ||
		detail::_has_member_try_cast<To, From>
struct cast_traits<To, From>
{
	static To const* cast(From const& from)
	{
		if constexpr (detail::_has_member_cast<To, From>)
		{
			return member_t::cast<To>(from);
		}
		else
		{
			return member_t::try_cast<To>(from);
		}
	}
};


template<typename To, typename From>
	requires detail::_has_member_try_cast<To, From>
struct try_cast_traits<To, From>
{
	static To const* try_cast(From const& from)
	{
		return member_t::try_cast<To>(from);
	}
};

} // namespace casting

using casting::is;
using casting::cast;
using casting::try_cast;

} // namespace vsm
