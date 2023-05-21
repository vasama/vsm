#pragma once

#include <stdexec/execution.hpp>

namespace vsm {
namespace detail {

template<typename T>
struct result_value_type
{
	static_assert(sizeof(T) == 0);
};

template<typename T>
struct result_value_type<vsm::result<T>>
{
	using type = T;
};

template<typename T>
using result_value_type_t = typename result_value_type<T>::type;


template<template<typename...> typename Tuple>
struct result_into_error_helper
{
	template<typename... Values>
	using value_tuple = Tuple<result_value_type_t<Values>...>;
};

template<typename Receiver, typename T>
class result_into_error_receiver
{
	Receiver m_receiver;

public:
	result_into_error_receiver(Receiver&& receiver)
		: m_receiver(vsm_move(receiver))
	{
	}

	void set_value(std::same_as<vsm::result<T>> auto&& result) &&
	{
		if (result)
		{
			if constexpr (std::is_void_v<T>)
			{
				stdexec::set_value(vsm_move(m_receiver));
			}
			else
			{
				stdexec::set_value(vsm_move(m_receiver), *vsm_move(result));
			}
		}
		else
		{
			stdexec::set_error(vsm_move(m_receiver), vsm_move(result).error());
		}
	}

	void set_error(auto&& error) && noexcept
	{
		static_assert(sizeof(error) == 0);
	}

	void set_done() && noexcept
	{
		stdexec::set_stopped(vsm_move(m_receiver));
	}
};

template<typename Sender>
class result_into_error_sender
{
	Sender m_sender;

public:
	static constexpr bool sends_stopped = stdexec::completion_signatures_of_t<Sender>::sends_stopped;

	template<template<typename...> typename Variant, template<typename...> typename Tuple>
	using value_types = stdexec::value_types_of_t<Sender, Variant, result_into_error_helper<Tuple>::template value_tuple>;

	template<template<typename...> typename Variant>
	using error_types = Variant<std::error_code>;

	result_into_error_sender(auto&& sender)
		: m_sender(vsm_move(sender))
	{
	}

	template<typename Receiver>
	auto connect(Receiver&& receiver) &&
	{
		using receiver_type = result_into_error_receiver<
			std::remove_cvref_t<Receiver>,
			result_value_type_t<stdexec::__single_sender_value_t<Sender>>>;
		return stdexec::connect(vsm_move(m_sender), receiver_type(vsm_forward(receiver)));
	}
};

struct result_into_error_fn
{
	template<stdexec::__single_typed_sender Sender>
	result_into_error_sender<std::remove_cvref_t<Sender>> operator()(Sender&& sender) const
	{
		return { vsm_forward(sender) };
	}
};


template<template<typename...> typename Tuple>
struct error_into_result_helper
{
	template<typename... Values>
	using value_tuple = Tuple<vsm::result<Values>...>;
};

template<typename Receiver, typename T>
class error_into_result_receiver
{
	Receiver m_receiver;

public:
	error_into_result_receiver(Receiver&& receiver)
		: m_receiver(vsm_move(receiver))
	{
	}

	void set_value(std::same_as<T> auto&& value) &&
	{
		stdexec::set_value(vsm_move(m_receiver), vsm::result<T>(vsm_forward(value)));
	}

	template<typename E>
	void set_error(E&& error) && noexcept
	{
		stdexec::set_error(vsm_move(m_receiver), vsm::result<T>(std::unexpected(vsm_forward(error))));
	}

	void set_done() && noexcept
	{
		stdexec::set_stopped(vsm_move(m_receiver));
	}
};

template<typename Sender>
class error_into_result_sender
{
	Sender m_sender;

public:
	static constexpr bool sends_stopped = stdexec::completion_signatures_of_t<Sender>::sends_stopped;

	template<template<typename...> typename Variant, template<typename...> typename Tuple>
	using value_types = stdexec::value_types_of_t<Sender, Variant, error_into_result_helper<Tuple>::template value_tuple>;

	template<template<typename...> typename Variant>
	using error_types = Variant<>;

	error_into_result_sender(auto&& sender)
		: m_sender(vsm_forward(sender))
	{
	}

	template<typename Receiver>
	auto connect(Receiver&& receiver) &&
	{
		using receiver_type = error_into_result_receiver<
			std::remove_cvref_t<Receiver>,
			stdexec::__single_sender_value_t<Sender>>;
		return stdexec::connect(vsm_move(m_sender), receiver_type(vsm_forward(receiver)));
	}
};

struct error_into_result_fn
{
	template<stdexec::__single_typed_sender Sender>
	error_into_result_sender<std::remove_cvref_t<Sender>> operator()(Sender&& sender) const
	{
		return { vsm_forward(sender) };
	}
};


template<typename Receiver>
class error_into_except_receiver
{
	Receiver m_receiver;

public:
	error_into_except_receiver(Receiver&& receiver)
		: m_receiver(vsm_move(receiver))
	{
	}

	void set_value(auto&&... values) &&
	{
		stdexec::set_value(vsm_move(m_receiver), vsm_forward(values)...);
	}

	void set_error(std::exception_ptr&& error) && noexcept
	{
		stdexec::set_error(vsm_move(m_receiver), vsm_move(error));
	}

	void set_error(std::error_code const error) && noexcept
	{
		if constexpr (requires { stdexec::set_error(vsm_move(m_receiver), error); })
		{
			stdexec::set_error(vsm_move(m_receiver), error);
		}
		else
		{
			stdexec::set_error(vsm_move(m_receiver), std::make_exception_ptr(std::system_error(error, "allio::error_into_except")));
		}
	}

	void set_done() && noexcept
	{
		stdexec::set_stopped(vsm_move(m_receiver));
	}
};

template<typename Sender>
class error_into_except_sender
{
	Sender m_sender;

public:
	static constexpr bool sends_stopped = stdexec::completion_signatures_of_t<Sender>::sends_stopped;

	template<template<typename...> typename Variant, template<typename...> typename Tuple>
	using value_types = stdexec::value_types_of_t<Sender, Variant, Tuple>;

	template<template<typename...> typename Variant>
	using error_types = Variant<std::exception_ptr>;

	error_into_except_sender(Sender&& sender)
		: m_sender(vsm_move(sender))
	{
	}

	template<stdexec::receiver Receiver>
	auto connect(Receiver&& receiver) &&
	{
		using receiver_type = error_into_except_receiver<std::remove_cvref_t<Receiver>>;
		return stdexec::connect(vsm_move(m_sender), receiver_type(vsm_forward(receiver)));
	}
};

struct error_into_except_fn
{
	template<stdexec::sender Sender>
	error_into_except_sender<std::remove_cvref_t<Sender>> operator()(Sender&& sender) const
	{
		return { vsm_forward(sender) };
	}
};

} // namespace detail

inline constexpr detail::result_into_error_fn result_into_error = {};
inline constexpr detail::error_into_result_fn error_into_result = {};
inline constexpr detail::error_into_except_fn error_into_except = {};

} // namespace vsm
