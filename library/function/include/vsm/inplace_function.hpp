#pragma once

#include <vsm/detail/function.hpp>

namespace vsm {

template<typename Signature, size_t Capacity>
class inplace_function : detail::_function_call<Capacity, Signature>
{
	using base_type = detail::_function_call<Capacity, Signature>;

public:
	using base_type::base_type;

	inplace_function() = default;

	inplace_function(inplace_function&&) = default;
	inplace_function& operator=(inplace_function&&) & = default;

	template<no_cvref_of<inplace_function> F>
	constexpr inplace_function(F&& f)
		requires
			no_instance_of<remove_cvref_t<F>, std::in_place_type_t> &&
			base_type::template is_invocable<std::decay_t<F>> &&
			(sizeof(std::decay_t<F>) <= Capacity)
	{
		using type = std::decay_t<F>;
		static_assert(alignof(type) <= alignof(std::max_align_t), "not implemented");
		this->template construct<type, typename base_type::template view_type<type>>(
			vsm_forward(f));
	}

	template<typename T, typename... Args>
	explicit constexpr inplace_function(std::in_place_type_t<T>, Args&&... args)
		requires
			std::constructible_from<std::decay_t<T>, Args...> &&
			base_type::template is_invocable<std::decay_t<T>> &&
			(sizeof(std::decay_t<T>) <= Capacity)
	{
		using type = std::decay_t<T>;
		static_assert(alignof(type) <= alignof(std::max_align_t), "not implemented");
		this->template construct<type, typename base_type::template view_type<type>>(
			vsm_forward(args)...);
	}

	using base_type::operator bool;
	using base_type::operator();
};

} // namespace vsm
