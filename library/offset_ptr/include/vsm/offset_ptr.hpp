#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>

#include <memory>
#include <limits>

#include <cstdint>

namespace vsm {
namespace detail {

template<typename T>
struct prop_cv_traits;

} // namespace detail

template<typename T, std::integral Offset = ptrdiff_t>
class offset_ptr
{
	Offset m_offset;

public:
	offset_ptr() = default;

	offset_ptr(decltype(nullptr)) noexcept
		: m_offset(0)
	{
	}

	offset_ptr(T* const ptr)
		: m_offset(get_offset(ptr))
	{
	}

	offset_ptr(offset_ptr const& other)
		: m_offset(get_offset(other.get()))
	{
	}

	template<typename OtherT, typename OtherOffset>
		requires std::convertible_to<OtherT*, T*> && losslessly_convertible_to<OtherOffset, Offset>
	offset_ptr(offset_ptr<OtherT, OtherOffset> const& other)
		: m_offset(get_offset(other.get()))
	{
	}

	offset_ptr& operator=(decltype(nullptr)) & noexcept
	{
		m_offset = 0;
		return *this;
	}

	offset_ptr& operator=(T* const that) &
	{
		m_offset = get_offset(that);
		return *this;
	}

	offset_ptr& operator=(offset_ptr const& other) &
	{
		m_offset = get_offset(other.get());
		return *this;
	}

	template<typename OtherT, typename OtherOffset>
		requires std::convertible_to<OtherT*, T*> && losslessly_convertible_to<OtherOffset, Offset>
	offset_ptr& operator=(offset_ptr<OtherT, OtherOffset> const& other) &
	{
		m_offset = get_offset(other.get());
		return *this;
	}


	[[nodiscard]] T* get() const noexcept
	{
		if (m_offset == 0)
		{
			return nullptr;
		}

		return reinterpret_cast<T*>(
			reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset));
	}

	[[nodiscard]] std::add_lvalue_reference_t<T> operator*() const
	{
		vsm_assert(m_offset != 0);

		return *reinterpret_cast<T*>(
			reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset));
	}

	[[nodiscard]] T* operator->() const
	{
		vsm_assert(m_offset != 0);

		return reinterpret_cast<T*>(
			reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset));
	}

	[[nodiscard]] std::add_lvalue_reference_t<T> operator[](ptrdiff_t const offset) const
	{
		vsm_assert(m_offset != 0);

		return reinterpret_cast<T*>(
			reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(m_offset))[offset];
	}


	offset_ptr& operator+=(ptrdiff_t const offset) &
	{
		vsm_assert(m_offset != 0);
		m_offset = get_offset(get() + offset);
		return *this;
	}

	offset_ptr& operator-=(ptrdiff_t const offset) &
	{
		vsm_assert(m_offset != 0);
		m_offset = get_offset(get() - offset);
		return *this;
	}

	[[nodiscard]] friend T* operator+(offset_ptr const& ptr, ptrdiff_t const offset)
	{
		vsm_assert(ptr.m_offset != 0);
		return ptr.get() + offset;
	}

	[[nodiscard]] friend T* operator+(ptrdiff_t const offset, offset_ptr const& ptr)
	{
		vsm_assert(ptr.m_offset != 0);
		return ptr.get() + offset;
	}

	[[nodiscard]] friend T* operator-(offset_ptr const& ptr, ptrdiff_t const offset)
	{
		vsm_assert(ptr.m_offset != 0);
		return ptr.get() - offset;
	}

	[[nodiscard]] friend T* operator-(ptrdiff_t const offset, offset_ptr const& ptr)
	{
		vsm_assert(ptr.m_offset != 0);
		return ptr.get() - offset;
	}

	[[nodiscard]] friend ptrdiff_t operator-(offset_ptr const& lhs, offset_ptr const& rhs)
	{
		return lhs.get() - rhs.get();
	}

	[[nodiscard]] friend ptrdiff_t operator-(offset_ptr const& lhs, T* const rhs)
	{
		return lhs.get() - rhs;
	}

	[[nodiscard]] friend ptrdiff_t operator-(T* const lhs, offset_ptr const& rhs)
	{
		return lhs - rhs.get();
	}

	[[nodiscard]] explicit operator bool() const
	{
		return m_offset != 0;
	}

	[[nodiscard]] friend bool operator==(offset_ptr const& lhs, offset_ptr const& rhs)
	{
		return lhs.get() == rhs.get();
	}

	[[nodiscard]] friend bool operator==(offset_ptr const& ptr, decltype(nullptr))
	{
		return ptr.m_offset == 0;
	}

	[[nodiscard]] friend auto operator<=>(offset_ptr const& lhs, offset_ptr const& rhs)
	{
		return lhs.get() <=> rhs.get();
	}

private:
	[[nodiscard]] Offset get_offset(T* const that) const
	{
		if (that == nullptr)
		{
			return 0;
		}

		intptr_t const offset = static_cast<intptr_t>(
			reinterpret_cast<uintptr_t>(that) -
			reinterpret_cast<uintptr_t>(this));

		using offset_limits = std::numeric_limits<Offset>;
		using intptr_limits = std::numeric_limits<intptr_t>;

		if constexpr (std::is_unsigned_v<Offset>)
		{
			vsm_assert_slow(offset >= 0);
		}
		else if constexpr (intptr_limits::min() < offset_limits::min())
		{
			vsm_assert_slow(offset >= offset_limits::min());
		}

		if constexpr (offset_limits::max() < intptr_limits::max())
		{
			vsm_assert_slow(offset <= offset_limits::max());
		}

		return static_cast<Offset>(offset);
	}
};

template<typename T, std::integral Offset>
struct detail::prop_cv_traits<offset_ptr<T, Offset>>
{
	using get_type = T*;
	using get_const_type = T const*;

	using size_type = size_t;
	using difference_type = ptrdiff_t;

	static get_type get(offset_ptr<T, Offset> const& ptr)
	{
		return ptr.get();
	}

	static get_const_type get_const(offset_ptr<T, Offset> const& ptr)
	{
		return ptr.get();
	}
};

} // namespace vsm
