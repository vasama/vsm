#pragma once

#include <vsm/buffers.hpp>

namespace vsm {

template<vsm::non_cvref T>
class any_buffer_ref
{
	using resize_buffer_t = vsm::result<std::span<T>>(
		void* object,
		size_t min_size,
		size_t max_size) noexcept;

	uintptr_t m_data_or_resize;
	uintptr_t m_size_or_object;

public:
	template<mutable_contiguous_range_of<T> Range>
		requires std::is_same_v<std::ranges::range_value_t<Range>, T>
	any_buffer_ref(Range&& range) noexcept
		: m_data_or_resize(reinterpret_cast<uintptr_t>(std::ranges::data(range)))
		, m_size_or_object(static_cast<uintptr_t>(std::ranges::size(range)) << 1 | 1)
	{
	}

	template<resizable_buffer_of<T> Buffer>
	any_buffer_ref(Buffer&& buffer) noexcept
		: m_data_or_resize(reinterpret_cast<uintptr_t>(&_resize_buffer<remove_ref_t<Buffer>>))
		, m_size_or_object(reinterpret_cast<uintptr_t>(std::addressof(buffer)))
	{
	}

	[[nodiscard]] vsm::result<std::span<std::byte>> resize(
		size_t const min_size,
		size_t const max_size) const noexcept
	{
		if (m_size_or_object & 1)
		{
			return _resize_span(
				reinterpret_cast<T*>(m_data_or_resize),
				m_size_or_object >> 1,
				min_size,
				max_size);
		}
		else
		{
			return reinterpret_cast<resize_buffer_t*>(m_data_or_resize)(
				reinterpret_cast<void*>(m_size_or_object),
				min_size,
				max_size);
		}
	}

private:
	static vsm::result<std::span<T>> _resize_span(
		T* const data,
		size_t const size,
		size_t const min_size,
		size_t const max_size) noexcept
	{
		if (min_size > size)
		{
			return vsm::unexpected(std::make_error_code(std::errc::no_buffer_space));
		}
		else
		{
			return std::span<T>(data, std::min(size, max_size));
		}
	}

	template<typename Buffer>
	static vsm::result<std::span<T>> _resize_buffer(
		void* const object,
		size_t const min_size,
		size_t const max_size) noexcept
	{
		return vsm::resize_buffer(*static_cast<Buffer*>(object), min_size, max_size);
	}

	[[nodiscard]] friend vsm::result<std::span<T>> tag_invoke(
		resize_buffer_t,
		any_buffer_ref const& ref,
		size_t const min_size,
		size_t const max_size) noexcept
	{
		return ref.resize(min_size, max_size);
	}
};

using any_byte_buffer_ref = any_buffer_ref<std::byte>;

} // namespace vsm
