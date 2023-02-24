#pragma once

#include <vsm/any_interface.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

template<typename Interface, typename Context, value_category ValueCategory>
class any_ref_2
{
	detail::any_function_ptr const* m_vptr;
	Context* m_context;

public:
	template<typename T>
	any_ref_2(T&& object)
		: m_vptr(detail::any_table_for<std::remove_cvref_t<T>>(Interface()))
		, m_context(const_cast<Context*>(&object))
	{
	}

	template<typename Operation>
	friend decltype(auto) any_invoke(any_ref_2 const self, auto&&... args)
	{
		using signature_info = detail::any_signature<Operation>;
		static_assert(std::is_convertible_v<Context*, typename signature_info::context_type*>);
		return reinterpret_cast<typename signature_info::signature_type*>(m_vptr[type_list_index<Interface, Operation>])(m_context, vsm_forward(args)...);
	}
};

template<typename Interface>
struct any_ref_1
{
	using type = any_ref_2<Interface, void, value_category::value>;
};

template<typename Interface>
struct any_ref_1<Interface&>
{
	using type = any_ref_2<Interface, void, value_category::lvalue_reference>;
};

template<typename Interface>
struct any_ref_1<Interface&&>
{
	using type = any_ref_2<Interface, void, value_category::rvalue_reference>;
};

template<typename Interface>
struct any_ref_1<Interface const>
{
	using type = any_ref_2<Interface, void const, value_category::const_value>;
};

template<typename Interface>
struct any_ref_1<Interface const&>
{
	using type = any_ref_2<Interface, void const, value_category::const_lvalue_reference>;
};

template<typename Interface>
struct any_ref_1<Intertface const&&>
{
	using type = any_ref_2<Interface, void const, value_category::const_rvalue_reference>;
};

} // namespace detail

} // namespace vsm
