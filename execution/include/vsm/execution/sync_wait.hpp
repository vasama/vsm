#pragma once

#include <vsm/assert.h>
#include <vsm/exceptions.hpp>
#include <vsm/execution/execution.hpp>
#include <vsm/standard.hpp>

#include <exception>
#include <optional>
#include <tuple>

namespace vsm::execution {
namespace detail::_sync_wait {

template<typename Result>
using variant = std::variant<
	std::monostate
	, Result,
	, std_execution::set_stopped_t
#if vsm_config_exceptions
	, std::exception_ptr
#endif
>;

template<typename Context, typename Variant>
struct context_and_variant
{
	Context& context;
	Variant variant;
};

template<typename Context>
auto _get_env(Context& context)
{
	if constexpr (requires { context.get_env() })
	{
		return context.get_env();
	}
	else
	{
		return std_execution::empty_env();
	}
}

template<typename Context, typename Sender, typename Continuation>
using _result2 =
	std_execution::__try_value_types_of_t<
		Sender,
		decltype(_get_env(std::declval<Context&>())),
		std_execution::__transform<std_execution::__q<std_execution::__decay_t>, Continuation>,
		std_execution::__q<std_execution::__msingle>>;

template<typename Context, typename Sender>
using _result1 =
	std_execution::__mtry_eval<
		_result2,
		Context,
		Sender,
		std_execution::__q<std::tuple>>;

template<typename Context, typename Variant>
struct receiver
{
	class type
	{
		context_and_variant<Context, Variant>* m_context_and_variant;

	public:
		using is_receiver = void;

		explicit type(context_and_variant<Context, Variant>& context_and_variant)
			: m_context_and_variant(&context_and_variant)
		{
		}

		friend void tag_invoke(std_execution::set_value_t, type&& self, auto&&... values) noexcept
		{
			vsm_assert(self.m_context_and_variant->variant.index() == 0);
			vsm_except_try
			{
				self.m_context_and_variant->variant.template emplace<1>(vsm_forward(values)...);
			}
			vsm_except_catch (...)
			{
				self._set_error(std::current_exception());
			}
		}

#if vsm_config_exceptions
		friend void tag_invoke(std_execution::set_error_t, type&& self, auto&& error) noexcept
		{
			vsm_assert(self.m_context_and_variant->variant.index() == 0);
			self._set_error(vsm_forward(error));
		}
#endif

		friend void tag_invoke(std_execution::set_stopped_t, type&& self) noexcept
		{
			vsm_assert(self.m_context_and_variant->variant.index() == 0);
			self.m_context_and_variant->variant.template emplace<2>();
		}

		friend decltype(auto) tag_invoke(std_execution::get_env_t, type const& self) noexcept
		{
			return self.m_context_and_variant->context.get_env();
		}

	private:
#if vsm_config_exceptions
		void _set_error(auto&& error) noexcept
		{
			using error_type = std::decay_t<decltype(error)&&>;
			if constexpr (std::is_same_v<error_type, std::exception_ptr>)
			{
				m_context_and_variant->variant.template emplace<3>(vsm_forward(error));
			}
			else
			{
				vsm_except_try
				{
					if constexpr (std::is_same_v<error_type, std::error_code>)
					{
						m_context_and_variant->variant.template emplace<3>(std::make_exception_ptr(std::system_error(error)));
					}
					else
					{
						m_context_and_variant->variant.template emplace<3>(std::make_exception_ptr(vsm_forward(error)));
					}
				}
				vsm_except_catch(...)
				{
					m_context_and_variant->variant.template emplace<3>(std::current_exception());
				}
			}
		}
#endif
	};
};

template<typename Context, typename Sender>
decltype(auto) _transform_sender(Context& context, Sender&& sender)
{
	if constexpr (requires { context.transform_sender(vsm_forward(sender)); })
	{
		return context.transform_sender(vsm_forward(sender));
	}
	else
	{
		return vsm_forward(sender);
	}
}

template<typename Context, typename Sender>
std::optional<_result1<Context, Sender&&>> _impl(Context& context, Sender&& sender)
{
	using variant_type = variant<_result1<Context, Sender&&>>;
	context_and_variant<Context, variant_type> context_and_variant{ context };

	auto operation_state = std_execution::connect(
		_transform_sender(context, vsm_forward(sender)),
		typename receiver<Context, variant_type>::type(context_and_variant));

	std_execution::start(operation_state);

	context.run_while([&]{ context_and_variant.variant.index() == 0; });

	if (context_and_variant.variant.index() == 2)
	{
		return std::nullopt;
	}

#if vsm_config_exceptions
	if (context_and_variant.variant.index() == 3)
	{
		std::rethrow_exception(std::get<3>(vsm_move(context_and_variant.variant)));
	}
#endif

	return std::get<1>(vsm_move(context_and_variant.variant));
}

struct sync_wait_t
{
	//TODO: Constrain using sender_in
	template<typename Context, std_execution::sender Sender>
	vsm_static_operator auto operator()(Context& context, Sender&& sender) vsm_static_operator_const
		-> std::optional<_result1<Context, Sender&&>>
	{
		return _impl(Context, vsm_forward(sender));
	}
};

} // namespace detail::_sync_wait

using detail::_sync_wait::sync_wait_t;
inline constexpr sync_wait_t sync_wait = {};

} // namespace vsm::execution
