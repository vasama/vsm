#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>

#include <limits>

#include <cstdint>

namespace vsm {

template<typename T, std::integral Offset = uintptr_t>
class offset_ptr
{
	using offset_type = vsm::select_t<sizeof(Offset) >= sizeof(uintptr_t), uintptr_t, Offset>;

	offset_type m_offset;

public:
	offset_ptr() = default;

	offset_ptr(decltype(nullptr))
		: m_offset(0)
	{
	}

	offset_ptr(T* const that)
		: m_offset(get_offset(that))
	{
	}

	offset_ptr(offset_ptr const& other)
		: m_offset(get_offset(other.get()))
	{
	}

	template<typename OtherT, typename OtherOffset>
	offset_ptr(offset_ptr<OtherT, OtherOffset> const& other)
		: m_offset(get_offset(other.get()))
	{
	}

	offset_ptr& operator=(decltype(nullptr)) &
	{
		m_offset = 0;
		return *this;
	}

	offset_ptr& operator=(T* const that) &
	{
		m_offset = get_offset(that);
	}

	offset_ptr& operator=(offset_ptr const& other) &
	{
		m_offset = get_offset(other.get());
		return *this;
	}

	template<typename OtherT, typename OtherOffset>
	offset_ptr& operator(offset_ptr<OtherT, OtherOffset> const& other)
	{
		m_offset = get_offset(other.get());
		return *this;
	}


	T* get() const
	{
		return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset));
	}

	T& operator*() const
	{
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset));
	}

	T* operator->() const
	{
		return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset));
	}


	explicit operator bool() const
	{
		return m_offset != 0;
	}

	friend auto operator<=>(offset_ptr const& lhs, offset_ptr const& rhs)
	{
		return lhs.get() <=> rhs.get();
	}

private:
	offset_type get_offset(T* const that) const
	{
		if (that == nullptr)
		{
			return 0;
		}

		intptr_t const offset = static_cast<intptr_t>(
			reinterpret_cast<uintptr_t>(that) -
			reinterpret_cast<uintptr_t>(this));

		if constexpr (!std::is_same_v<offset_type, uintptr_t>)
		{
			vsm_assert(offset >= std::numeric_limits<offset_type>::min());
			vsm_assert(offset <= std::numeric_limits<offset_type>::max());
		}

		return static_cast<offset_type>(offset);
	}
};

} // namespace vsm
