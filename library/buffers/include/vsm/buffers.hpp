#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/exception_code.hpp>
#include <vsm/exceptions.hpp>
#include <vsm/result.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>

#include <ranges>
#include <span>
#include <system_error>

namespace vsm {

template<typename Range>
concept mutable_contiguous_range =
	std::ranges::contiguous_range<Range> &&
	std::ranges::sized_range<Range> &&
	vsm::non_cv<std::ranges::range_value_t<Range>>;

template<typename Range, typename T>
concept mutable_contiguous_range_of =
	mutable_contiguous_range<Range> &&
	std::is_same_v<std::ranges::range_value_t<Range>, T>;

template<typename Container>
concept mutable_contiguous_container =
	mutable_contiguous_range<Container> &&
	requires (Container container, size_t const size)
	{
		requires vsm::non_cvref<typename std::remove_cvref_t<Container>::value_type>;
		{ container.data() } -> std::same_as<typename std::remove_cvref_t<Container>::value_type*>;
		{ container.size() } -> std::same_as<size_t>;
		{ container.capacity() } -> std::same_as<size_t>;
		{ container.resize(size) } -> std::same_as<void>;
	};

template<typename Container, typename T>
concept mutable_contiguous_container_of =
	mutable_contiguous_range_of<Container, T> &&
	mutable_contiguous_container<Container>;

namespace detail {

template<typename Container>
void default_resize_container(Container& container, size_t const size)
{
	if constexpr (requires { container.resize_default(size); })
	{
		container.resize_default(size);
	}
	else
	{
		container.resize(size);
	}
}

} // namespace detail

struct resize_buffer_t
{
	template<mutable_contiguous_container Container>
	friend result<std::span<typename Container::value_type>> tag_invoke(
		resize_buffer_t,
		Container& container,
		size_t const min_size,
		size_t const max_size) noexcept
	{
		vsm_except_try
		{
			if (container.size() > max_size)
			{
				container.resize(max_size);
			}
			else
			{
				if (container.size() < min_size)
				{
					detail::default_resize_container(container, min_size);
				}

				if (min_size < max_size && container.size() < container.capacity())
				{
					// Resize the container further, up to min(max_size, capacity).
					detail::default_resize_container(
						container,
						std::min(max_size, container.capacity()));
				}
			}
		}
		vsm_except_catch (std::bad_alloc const&)
		{
			return vsm::unexpected(exception_code::bad_alloc);
		}

		return std::span<typename Container::value_type>(container.data(), container.size());
	}

	template<typename Container>
		requires vsm::tag_invocable<resize_buffer_t, Container&, size_t, size_t>
	[[nodiscard]] vsm_static_operator auto operator()(
		Container& container,
		size_t const min_size,
		size_t const max_size) vsm_static_operator_const noexcept
	{
		vsm_assert(min_size <= max_size); //PRECONDITION
		return vsm::tag_invoke(resize_buffer_t(), container, min_size, max_size);
	}
};
inline constexpr resize_buffer_t resize_buffer = {};

template<typename Buffer>
concept resizable_buffer = requires (Buffer& buffer, size_t const size)
{
	vsm::resize_buffer(buffer, size, size);
};

template<resizable_buffer Buffer>
using resizable_buffer_value_t =
	typename decltype(vsm::resize_buffer(std::declval<Buffer&>(), 0, 0))::value_type::value_type;

template<typename Buffer, typename T>
concept resizable_buffer_of =
	resizable_buffer<Buffer> &&
	std::is_same_v<resizable_buffer_value_t<Buffer>, T>;

} // namespace vsm
