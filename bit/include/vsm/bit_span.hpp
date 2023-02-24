#pragma once

#include <vsm/assert.h>
#include <vsm/bit_ptr.hpp>
#include <vsm/bit_ref.hpp>

namespace vsm {

template<typename Word>
class bit_span
{
	bit_ptr<Word> m_data;
	size_t m_size = 0;

public:
	using word_type                     = Word;
	using size_type                     = size_t;
	using difference_type               = ptrdiff_t;
	using pointer                       = bit_ptr<Word>;
	using const_pointer                 = pointer;
	using reference                     = bit_ref<Word>;
	using const_reference               = reference;
	using iterator                      = pointer;
	using const_iterator                = const_pointer;


	bit_span() = default;

	explicit constexpr bit_span(pointer const data, size_t const size) noexcept
		: m_data(data)
		, m_size(size)
	{
	}

	bit_span& operator=(bit_span const&) & = default;


	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return m_size;
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return m_size == 0;
	}

	[[nodiscard]] constexpr pointer data() const noexcept
	{
		return m_data;
	}

	[[nodiscard]] constexpr reference front() const noexcept
	{
		vsm_assert(m_size != 0);
		return *m_data;
	}

	[[nodiscard]] constexpr reference back() const noexcept
	{
		vsm_assert(m_size != 0);
		return m_data[m_size - 1];
	}

	[[nodiscard]] constexpr reference operator[](size_t const index) const noexcept
	{
		vsm_assert(index < m_size);
		return m_data + index;
	}

	[[nodiscard]] constexpr bit_span subspan(size_t const index) const noexcept
	{
		vsm_assert(index <= m_size);
		return bit_span(m_data + index, m_size - index);
	}

	[[nodiscard]] constexpr bit_span subspan(size_t const index, size_t const size) const noexcept
	{
		vsm_assert(index <= m_size);
		return bit_span(m_data + index, std::min(size, m_size - index));
	}

	[[nodiscard]] constexpr pointer begin() const noexcept
	{
		return m_data;
	}

	[[nodiscard]] constexpr pointer end() const noexcept
	{
		return m_data + m_size;
	}

	auto operator<=>(bit_span const&) const = default;
};

} // namespace vsm
