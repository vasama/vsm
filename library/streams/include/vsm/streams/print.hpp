#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/platform.h>
#include <vsm/standard.hpp>
#include <vsm/streams/streams.hpp>
#include <vsm/tag_invoke.hpp>

#include <charconv>
#include <string_view>

namespace vsm::streams {

struct print_t;

namespace detail {

template<sink Sink, std::integral T>
vsm::result<size_t> print_integral(Sink& sink, T const value, int const radix) noexcept
{
	static_assert(vsm::nothrow_tag_invocable<print_t, Sink&, std::string_view>);
	constexpr size_t const max_string_size = sizeof(T) * 8;

	if constexpr (direct_sink<Sink&>)
	{
		if (auto const r = sink.direct_write_acquire(max_string_size))
		{
			auto const [ec, end] = std::to_chars(r->data(), r->data() + r->size(), value, radix);
			vsm_assert(!ec);

			sink.direct_write_release(static_cast<size_t>(end - r->data()));
			return {};
		}
	}

	char buffer[max_string_size];
	auto const [ec, end] = std::to_chars(buffer, buffer + max_string_size, value, radix);
	vsm_assert(!ec);

	return vsm::tag_invoke(print_t(), sink, std::string_view(buffer, end));
}

template<sink Sink, std::floating_point T, typename... Args>
vsm::result<size_t> print_floating_point(Sink& sink, T const value, Args const... args) noexcept;

} // namespace detail

struct print_t
{
	explicit print_t() = default;

	template<sink Sink>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		char const character) noexcept
	{
		return streams::write(sink, std::as_bytes(std::span(&character, 1)));
	}

	template<sink Sink, std::convertible_to<std::string_view> String>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		String const& string) noexcept
	{
		std::string_view const string_view(string);
		return streams::write(
			sink,
			std::as_bytes(std::span(string_view)));
	}

	template<sink Sink, size_t Size>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		char const(&string)[Size]) noexcept
	{
		static_assert(vsm::nothrow_tag_invocable<print_t, Sink&, std::string_view>);
		return vsm::tag_invoke(print_t(), sink, std::string_view(string));
	}

	template<sink Sink, std::same_as<bool> T>
	friend vsm::result<size_t> tag_invoke(print_t, Sink& sink, T const value) noexcept
	{
		static_assert(vsm::nothrow_tag_invocable<print_t, Sink&, std::string_view>);
		return vsm::tag_invoke(
			print_t(),
			sink,
			value ? std::string_view("true") : std::string_view("false"));
	}

	template<sink Sink, std::integral T>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		T const value) noexcept
	{
		return detail::print_integral(sink, value, /* radix: */ 10);
	}

	template<sink Sink, std::floating_point T>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		T const value) noexcept
	{
		return detail::print_floating_point(sink, value);
	}

	template<sink Sink, vsm::pointer T>
	friend vsm::result<size_t> tag_invoke(print_t, Sink& sink, T) noexcept = delete;

	template<sink Sink, vsm::pointer T>
		requires vsm::any_cv_of<vsm::remove_ptr_t<T>, void>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		T const ptr) noexcept
	{
		static_assert(vsm::nothrow_tag_invocable<print_t, Sink&, uintptr_t>);
		return vsm::tag_invoke(print_t(), sink, reinterpret_cast<uintptr_t>(ptr));
	}

	template<sink Sink, vsm::enumeration T>
	vsm_always_inline friend vsm::result<size_t> tag_invoke(
		print_t,
		Sink& sink,
		T const value) noexcept
	{
		static_assert(vsm::nothrow_tag_invocable<print_t, Sink&, std::underlying_type_t<T>>);
		return vsm::tag_invoke(print_t(), sink, static_cast<std::underlying_type_t<T>>(value));
	}

	template<sink Sink, typename T>
		requires vsm::tag_invocable<print_t, Sink&, T const&>
	[[nodiscard]] vsm_always_inline vsm_static_operator vsm::result<size_t> operator()(
		Sink&& sink,
		T const& value) vsm_static_operator_const noexcept
	{
		static_assert(vsm::nothrow_tag_invocable<print_t, Sink&, T const>);
		return vsm::tag_invoke(print_t(), sink, value);
	}
};
inline constexpr print_t print{};


namespace operators {

template<typename Sink, vsm::non_cvref T>
/* discardable */ constexpr Sink&& operator<<(Sink&& sink, T const& value)
{
	if (auto const r = streams::print(sink, value))
	{
		return vsm_forward(sink);
	}
	else
	{
		throw std::system_error(r.error());
	}
}

#if 0
template<typename Source, vsm::non_cvref T>
/* discardable */ constexpr Source&& operator>>(Source&& source, T& value)
{
	auto r = streams::read<T>(source);
	if (r)
	{
		value = *vsm_move(r);
		return vsm_forward(source);
	}
	else
	{
		throw std::system_error(r.error());
	}
}
#endif

} // namespace operators
} // namespace vsm::streams
