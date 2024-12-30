#pragma once

#include <vsm/allocator.hpp>
#include <vsm/detail/function.hpp>

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

template<typename Allocator, typename Callable>
class _function_dynamic
{
	struct storage
	{
		vsm_no_unique_address Callable callable;
		vsm_no_unique_address Allocator allocator;
	};

	struct deleter
	{
		vsm_static_operator void operator()(storage* const object) vsm_static_operator_const
		{
			vsm::delete_via(object, Allocator(vsm_move(object->allocator)));
		}
	};

	std::unique_ptr<storage, deleter> m_storage;

public:
	explicit _function_dynamic(Allocator const& allocator, auto&&... args)
	{
		m_storage.reset(new (allocator) storage
		{
			Callable(vsm_forward(args)...),
			allocator,
		});
	}

	template<typename Self, typename... Args>
	auto operator()(this Self&& self, Args&&... args)
		noexcept(std::is_nothrow_invocable_v<copy_cvref_t<Self, Callable>, Args...>)
	{
		return static_cast<copy_cvref_t<Self, Callable>&&>(self.m_storage->callable)(
			vsm_forward(args)...);
	}
};

} // namespace detail

template<typename Signature, size_t Capacity = detail::function_default_capacity>
class function : detail::_function_call<detail::function_min_capacity(Capacity), Signature>
{
	static constexpr size_t capacity = detail::function_min_capacity(Capacity);
	using base_type = detail::_function_call<capacity, Signature>;

public:
	using base_type::base_type;

	function() = default;

	function(function&&) = default;
	function& operator=(function&&) & = default;

	template<no_cvref_of<function> F>
	constexpr function(F&& f)
		requires
			no_instance_of<remove_cvref_t<F>, std::in_place_type_t> &&
			base_type::template is_invocable<std::decay_t<F>>
	{
		construct<F>(default_allocator(), vsm_forward(f));
	}

	template<typename T, typename... Args>
	explicit constexpr function(std::in_place_type_t<T>, Args&&... args)
		requires
			std::constructible_from<std::decay_t<T>, Args...> &&
			base_type::template is_invocable<std::decay_t<T>>
	{
		construct<T>(default_allocator(), vsm_forward(args)...);
	}

	template<allocator Allocator, no_cvref_of<function> F>
	constexpr function(Allocator const& allocator, F&& f)
		requires
			no_instance_of<remove_cvref_t<F>, std::in_place_type_t> &&
			base_type::template is_invocable<std::decay_t<F>>
	{
		construct<F>(allocator, vsm_forward(f));
	}

	template<allocator Allocator, typename T, typename... Args>
	explicit constexpr function(Allocator const& allocator, std::in_place_type_t<T>, Args&&... args)
		requires
			std::constructible_from<std::decay_t<T>, Args...> &&
			base_type::template is_invocable<std::decay_t<T>>
	{
		construct<T>(allocator, vsm_forward(args)...);
	}

	using base_type::operator bool;
	using base_type::operator();

private:
	template<typename T, typename Allocator, typename... Args>
	void construct(Allocator const& allocator, Args&&... args)
	{
		using value_type = std::decay_t<T>;
		if constexpr (sizeof(value_type) <= Capacity)
		{
			using type = value_type;
			static_assert(alignof(type) <= alignof(std::max_align_t), "not implemented");
			base_type::template construct<type, typename base_type::template view_type<type>>(
				vsm_forward(args)...);
		}
		else
		{
			using type = detail::_function_dynamic<Allocator, value_type>;
			base_type::template construct<type, typename base_type::template view_type<type>>(
				allocator,
				vsm_forward(args)...);
		}
	}
};

} // namespace vsm
