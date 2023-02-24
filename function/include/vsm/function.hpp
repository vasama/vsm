#pragma once

#include <vsm/static_function.hpp>

#include <memory>

namespace vsm {
namespace detail {

consteval size_t function_min_capacity(size_t const capacity)
{
	if (capacity < sizeof(void*))
	{
		return sizeof(void*);
	}
	return capacity;
}

template<typename Signature, int Category, size_t Capacity>
class function_impl;

template<typename Result, typename... Params, int Category, size_t Capacity>
class function_impl<Result(Params...), Category, Capacity>
	: static_function_impl<Result(Params...), Category, function_min_capacity(Capacity)>
{
	using signature_type = Result(Params...);
	using static_function_type = static_function_impl<Result(Params...), Category, function_min_capacity(Capacity)>;

	template<typename Callable, typename Allocator>
	class dynamic_callable
	{
		class dynamic_storage
		{
			Allocator allocator;
			Callable callable;

			friend class dynamic_callable;
		};

		using allocator_type = typename std::allocator_traits<Allocator>::template rebind<dynamic_storage>;

		dynamic_storage* m_storage;

	public:
		explicit dynamic_callable(auto&& allocator, auto&& callable)
		{
			allocator_type rebound_allocator = allocator_type(vsm_forward(allocator));
			//TODO: alignment
			m_storage = new (rebound_allocator.allocate(sizeof(dynamic_storage)))
				dynamic_storage{ { vsm_move(rebound_allocator) }, vsm_forward(callable) };
		}

		explicit dynamic_callable(auto&& allocator, std::in_place_t, auto&&... args)
		{
			allocator_type rebound_allocator = allocator_type(vsm_forward(allocator));
			//TODO: alignment
			m_storage = new (rebound_allocator.allocate(sizeof(dynamic_storage)))
				dynamic_storage{ { vsm_move(rebound_allocator) }, { vsm_forward(args)... } };
		}

		// Intentionally trivially copyable despite non-trivial destructor.
		// static_function relocation drops the source object after copying it.
		dynamic_callable(dynamic_callable const&) = default;
		dynamic_callable& operator=(dynamic_callable const&) = default;

		~dynamic_callable()
		{
			allocator_type rebound_allocator = allocator_type(vsm_move(m_storage->allocator));
			m_storage->~dynamic_storage();
			rebound_allocator.deallocate(m_storage, sizeof(dynamic_storage));
		}
	};

public:
	function_impl() = default;

	template<no_cvref_of<function_impl> Callable, typename Allocator>
	explicit constexpr function_impl(Callable&& callable, Allocator&& allocator)
		requires
			detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...> &&
			(sizeof(Callable) <= Capacity)
		: static_function_type(vsm_forward(callable))
	{
	}

	template<no_cvref_of<function_impl> Callable, typename Allocator>
	explicit constexpr function_impl(Callable&& callable, Allocator&& allocator)
		requires
			detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...> &&
			(sizeof(Callable) > Capacity)
		: static_function_type(std::in_place_type<dynamic_callable<std::decay_t<Callable>, std::remove_cvref_t<Allocator>>>, vsm_forward(allocator), vsm_forward(callable))
	{
	}

	template<int OtherCategory, size_t OtherCapacity>
	constexpr function_impl(static_function_impl<signature_type, OtherCategory, OtherCapacity>&& src)
		requires (OtherCapacity <= Capacity && (OtherCategory & Category) == Category)
		: static_function_type(vsm_move(src))
	{
	}

	template<int OtherCategory, size_t OtherCapacity>
	constexpr function_impl(function_impl<signature_type, OtherCategory, OtherCapacity>&& src)
		requires (OtherCapacity <= Capacity && (OtherCategory & Category) == Category)
		: static_function_type(static_cast<typename function_impl<signature_type, OtherCategory, OtherCapacity>::static_function_type&&>(src))
	{
	}

	template<typename Callable>
	constexpr function_impl(std::in_place_type_t<Callable>) = delete;

	template<typename Callable, typename... Args>
	constexpr function_impl(std::in_place_type_t<Callable> const in_place_type, Args&&... args)
		requires
			detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...> &&
			(sizeof(Callable) <= Capacity)
		: static_function_type(in_place_type, vsm_forward(callable))
	{
	}

	template<typename Callable, typename... Args>
	constexpr function_impl(std::in_place_type_t<Callable>, Args&&... args)
		requires
			detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...> &&
			(sizeof(Callable) > Capacity)
		: static_function_type(std::in_place_type<dynamic_callable<std::decay_t<Callable>, std::remove_cvref_t<std::allocator>>>, std::in_place, vsm_forward(args)...)
	{
	}


	using static_function_type::operator bool;
	using static_function_type::operator();

private:
	template<typename OtherSignature, int OtherCategory, size_t OtherCapacity>
	friend class function_impl;
};

} // namespace detail

template<typename Signature, size_t Capacity = detail::function_default_capacity>
using function = detail::function_impl<
	typename detail::function_traits<Signature>::signature,
	detail::function_traits<Signature>::category,
	Capacity>;

} // namespace vsm
