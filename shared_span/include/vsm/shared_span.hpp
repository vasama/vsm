#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

#include <memory>
#include <span>

namespace vsm {

template<non_ref T>
class shared_span
{
	using data_t = std::shared_ptr<T>;
	using span_t = std::span<T>;

	data_t m_data;
	size_t m_size;

public:
	using element_type                  = T;
	using value_type                    = std::remove_cv_t<T>;
	using size_type                     = size_t;
	using difference_type               = ptrdiff_t;
	using pointer                       = T*;
	using const_pointer                 = T const*;
	using reference                     = T&;
	using const_reference               = T const&;
	using iterator                      = T*;
	using const_iterator                = T const*;
	using reverse_iterator              = std::reverse_iterator<iterator>;
	using const_reverse_iterator        = std::reverse_iterator<const_iterator>;


	constexpr shared_span()
		: m_data(nullptr)
		, m_size(0)
	{
	}

	template<any_cvref_of<data_t> DataT>
	explicit shared_span(DataT&& data, size_t const size)
		: m_data(vsm_forward(data))
		, m_size(size)
	{
	}

	template<typename U>
	explicit shared_span(std::shared_ptr<U>&& ptr, span_t const span)
		: m_data(vsm_forward(ptr), span.data())
		, m_size(span.size())
	{
	}


	[[nodiscard]] iterator begin() const
	{
		return m_data.get();
	}

	[[nodiscard]] iterator end() const
	{
		return m_data.get() + m_size;
	}

	[[nodiscard]] const_iterator cbegin() const
	{
		return m_data.get();
	}

	[[nodiscard]] const_iterator cend() const
	{
		return m_data.get() + m_size;
	}

	[[nodiscard]] reverse_iterator rbegin() const
	{
		return reverse_iterator(end());
	}

	[[nodiscard]] reverse_iterator rend() const
	{
		return reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator crbegin() const
	{
		return const_reverse_iterator(cend());
	}

	[[nodiscard]] const_reverse_iterator crend() const
	{
		return const_reverse_iterator(cbegin());
	}


	[[nodiscard]] T& front() const
	{
		vsm_assert(m_size != 0);
		return *m_data;
	}

	[[nodiscard]] T& back() const
	{
		vsm_assert(m_size != 0);
		return *(m_data.get() + m_size - 1);
	}

	[[nodiscard]] T& at(size_t const index) const
	{
		return span_t(*this).at(index);
	}

	[[nodiscard]] T& operator[](size_t const index) const
	{
		vsm_assert(index < m_size);
		return m_data.get()[index];
	}

	[[nodiscard]] T* data() const
	{
		return m_data.get();
	}


	[[nodiscard]] size_t size() const
	{
		return m_size;
	}

	[[nodiscard]] bool empty() const
	{
		return m_size != 0;
	}


	template<any_cvref_of<shared_span> Self>
	[[nodiscard]] shared_span first(this Self&& self, size_t const count)
	{
		return shared_span(vsm_forward(self).m_data, span_t(self).first(count));
	}

	template<any_cvref_of<shared_span> Self>
	[[nodiscard]] shared_span last(this Self&& self, size_t const count)
	{
		return shared_span(vsm_forward(self).m_data, span_t(self).last(count));
	}

	template<any_cvref_of<shared_span> Self>
	[[nodiscard]] shared_span subspan(
		this Self&& self,
		size_t const offset,
		size_t const count = static_cast<size_t>(-1))
	{
		return shared_span(vsm_forward(self).m_data, span_t(self).subspan(offset, count));
	}


	[[nodiscard]] data_t get_shared_ptr() &&
	{
		m_size = 0;
		return vsm_move(m_data);
	}

	template<vsm::any_ref_of<shared_span const> Self>
	[[nodiscard]] data_t const& get_shared_ptr(this Self&& self)
	{
		return vsm_forward(self).m_data;
	}
};

} // namespace vsm
