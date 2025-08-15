#pragma once

#include <vsm/bit_packing.hpp>
#include <vsm/detail/any_ref.hpp>
#include <vsm/standard.hpp>

namespace vsm {
namespace detail {

template<typename T>
struct _any_packed
{
	struct wrapper
	{
		T value;

		operator void const*() const noexcept
		{
			return std::addressof(value);
		}
	};

	static wrapper apply(void const* const context) noexcept
	{
		return wrapper(bit_unpack<T>(context));
	}
};

template<typename... Functions, bool D, size_t C, typename... Fs>
void _any_new_base_constraint_1(_any_new_base<D, C, Functions..., Fs...> const&);

template<typename T, typename... Functions>
concept _any_new_base_constraint = requires
{
	detail::_any_new_base_constraint_1<Functions...>(std::declval<T>());
};

} // namespace detail

template<typename... Functions>
class any_ref : public detail::_any_tag
{
	detail::function_ptr_t const* m_functions;
	void const* m_context;

public:
	template<detail::_non_any T>
		requires detail::_any_type_constraint<T, Functions...>
	constexpr any_ref(T&& object) noexcept
		: m_functions(detail::_any_functions<remove_ref_t<T>, detail::_any_identity, Functions...>)
	{
	}

	template<detail::_non_any T>
		requires
			(sizeof(std::decay_t<T>) <= sizeof(m_context)) &&
			std::is_trivially_copyable_v<std::decay_t<T>> &&
			detail::_any_type_constraint<std::decay_t<T> const&, Functions...>
	explicit any_ref(std::in_place_t, T&& object) noexcept
		: m_functions(detail::_any_functions<std::decay_t<T> const, detail::_any_packed<std::decay_t<T>>, Functions...>)
		, m_context(detail::bit_pack<void const*>(std::decay_t<T>(vsm_forward(object))))
	{
	}

	template<detail::_non_any T, typename... Args>
		requires
			std::constructible_from<T, Args...> &&
			std::is_trivially_copyable_v<T> &&
			(sizeof(T) <= sizeof(m_context)) &&
			detail::_any_type_constraint<T, Functions...>
	explicit any_ref(std::in_place_type_t<T>, Args&&... args)
		: m_functions(detail::_any_functions<T, detail::_any_packed<T>, Functions...>)
		, m_context(detail::bit_pack<void const*>(T(vsm_forward(args)...)))
	{
	}

	template<typename... OtherFunctions>
		requires (sizeof...(OtherFunctions) > 0)
	constexpr any_ref(any_ref<Functions..., OtherFunctions...> const& other)
		: m_functions(other.m_functions)
		, m_context(other.m_context)
	{
	}

	template<detail::_any_new_base_constraint<Functions...> Any>
		//TODO: Add a constraint checking for the correct category
	constexpr any_ref(std::monostate, Any&& any) noexcept
		: m_functions(any.m_functions_and_flags.pointer())
		, m_context(any.m_union.get_storage(any.m_functions_and_flags))
	{
	}


	template<typename F, typename... Args>
		requires detail::_any_call_constraint<F, int&, Args...>
	[[nodiscard]] detail::_any_traits_for<F>::return_type invoke(Args&&... args) const
	{
		return static_cast<typename detail::_any_traits_for<F>::function_type*>(
			m_functions[index_in_pack_v<F, Functions...>])(
			m_context,
			vsm_forward(args)...);
	}

private:
	template<bool OtherDynamic, size_t OtherCapacity, typename... OtherFunctions>
	friend class detail::_any_new_base;

	template<typename... OtherFunctions>
	friend class any_ref;
};

} // namespace vsm
