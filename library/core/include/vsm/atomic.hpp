#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/categories.hpp>
#include <vsm/platform.h>
#include <vsm/preprocessor.h>
#include <vsm/utility.hpp>

#if __has_include(vsm_pp_include(vsm/detail/atomic/vsm_arch.hpp))
#	include vsm_pp_include(vsm/detail/atomic/vsm_arch.hpp)
#else
#	define vsm_detail_atomic_ref_base(T) std::atomic_ref<T>
#endif

#include <atomic>
#include <concepts>
#include <utility>

namespace vsm {

template<typename T>
class atomic;

template<typename T>
class atomic_ref;

#if 0
template<enumeration T>
inline constexpr bool enable_atomic_flag_enum = false;
#endif

namespace detail {

#if 0
template<typename E>
concept _atomic_enum = enumeration<E> && enable_atomic_flag_enum<E>;
#endif

template<typename T>
class _atomic;

template<typename T>
class _atomic_ref : vsm_detail_atomic_ref_base(T)
{
	using base = vsm_detail_atomic_ref_base(T);

	static_assert(base::is_always_lock_free);

public:
	using value_type = typename base::value_type;

	using base::is_always_lock_free;
	using base::required_alignment;


	using base::base;

	explicit _atomic_ref(_atomic<T>& atomic)
		: base(atomic.m_storage)
	{
	}


	[[nodiscard]] constexpr bool is_lock_free() const noexcept
	{
		return true;
	}

	[[nodiscard]] vsm_always_inline T load(std::memory_order const order) const noexcept
	{
		return base::load(order);
	}

	vsm_always_inline void store(T const& new_value, std::memory_order const order) const noexcept
	{
		base::store(new_value, order);
	}

	[[nodiscard]] vsm_always_inline T exchange(
		T const& new_value,
		std::memory_order const order) const noexcept
	{
		return base::exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) const noexcept
	{
		return base::compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) const noexcept
	{
		return base::compare_exchange_strong(expected, desired, success, failure);
	}

private:
	friend atomic_ref<T>;
};

#if 0
template<_atomic_enum E>
class _atomic_ref<E> : std::atomic_ref<std::underlying_type_t<E>>
{
	using base = std::atomic_ref<std::underlying_type_t<E>>;
	using type = std::underlying_type_t<E>;

	static_assert(base::is_always_lock_free);

public:
	using value_type = typename base::value_type;

	using base::is_always_lock_free;
	using base::required_alignment;


	using base::base;

	explicit _atomic_ref(_atomic<E>& atomic)
		: base(atomic.m_storage)
	{
	}


	[[nodiscard]] constexpr bool is_lock_free() const noexcept
	{
		return true;
	}

	[[nodiscard]] vsm_always_inline E load(std::memory_order const order) const noexcept
	{
		return static_cast<E>(base::load(order));
	}

	vsm_always_inline void store(E const& new_value, std::memory_order const order) const noexcept
	{
		base::store(static_cast<type>(new_value), order);
	}

	[[nodiscard]] vsm_always_inline E exchange(
		E const& new_value,
		std::memory_order const order) const noexcept
	{
		return static_cast<E>(base::exchange(static_cast<type>(new_value), order));
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		E& expected,
		E const& desired,
		std::memory_order const success,
		std::memory_order const failure) const noexcept
	{
		type underlying_expected = static_cast<type>(expected);
		bool const result = base::compare_exchange_weak(
			underlying_expected,
			static_cast<type>(desired),
			success,
			failure);

		expected = static_cast<E>(underlying_expected);
		return result;
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		E& expected,
		E const& desired,
		std::memory_order const success,
		std::memory_order const failure) const noexcept
	{
		type underlying_expected = static_cast<type>(expected);
		bool const result = base::compare_exchange_strong(
			underlying_expected,
			static_cast<type>(desired),
			success,
			failure);

		expected = static_cast<E>(underlying_expected);
		return result;
	}

	[[nodiscard]] vsm_always_inline E fetch_and(
		E const operand,
		std::memory_order const order) const noexcept
	{
		return static_cast<E>(base::fetch_and(static_cast<type>(operand), order));
	}

	[[nodiscard]] vsm_always_inline E fetch_or(
		E const operand,
		std::memory_order const order) const noexcept
	{
		return static_cast<E>(base::fetch_or(static_cast<type>(operand), order));
	}

	[[nodiscard]] vsm_always_inline E fetch_xor(
		E const operand,
		std::memory_order const order) const noexcept
	{
		return static_cast<E>(base::fetch_xor(static_cast<type>(operand), order));
	}

private:
	friend atomic_ref<E>;
};
#endif

template<typename T>
class _atomic
{
	using ref_type = _atomic_ref<T>;

	alignas(ref_type::required_alignment) mutable T m_storage = {};

public:
	using value_type = typename ref_type::value_type;

	static constexpr bool is_always_lock_free = true;


	_atomic()
		: m_storage{}
	{
	}

	_atomic(T const value) noexcept
		requires std::is_fundamental_v<T>
		: m_storage(value)
	{
	}

	template<std::convertible_to<T> U = T>
	_atomic(U&& value) noexcept
		requires (!std::is_fundamental_v<T>)
		: m_storage(vsm_forward(value))
	{
	}

	template<typename... Args>
	explicit _atomic(std::in_place_t, Args&&... args)
		requires std::constructible_from<T, Args...>
		: m_storage(vsm_forward(args)...)
	{
	}

	_atomic(_atomic const&) = delete;
	_atomic& operator=(_atomic const&) = delete;


	[[nodiscard]] constexpr bool is_lock_free() const noexcept
	{
		return true;
	}

	[[nodiscard]] vsm_always_inline T load(std::memory_order const order) const noexcept
	{
		return ref_type(m_storage).load(order);
	}

	vsm_always_inline void store(T const& new_value, std::memory_order const order) & noexcept
	{
		return ref_type(m_storage).store(new_value, order);
	}

	[[nodiscard]] vsm_always_inline T exchange(
		T const& new_value,
		std::memory_order const order) & noexcept
	{
		return ref_type(m_storage).exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) & noexcept
	{
		return ref_type(m_storage).compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) & noexcept
	{
		return ref_type(m_storage).compare_exchange_strong(expected, desired, success, failure);
	}

private:
	friend _atomic_ref<T>;
};

} // namespace detail

/// @brief Stripped down version of std::atomic_ref<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type referred to by the atomic_ref.
template<typename T>
class atomic_ref : public detail::_atomic_ref<T>
{
	using base = detail::_atomic_ref<T>;

public:
	using base::base;
};

template<std::integral T>
class atomic_ref<T> : public detail::_atomic_ref<T>
{
	using base = detail::_atomic_ref<T>;

public:
	using base::base;

	[[nodiscard]] vsm_always_inline T fetch_add(
		T const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_add(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_sub(
		T const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_sub(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_and(
		T const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_and(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_or(
		T const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_or(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_xor(
		T const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_xor(operand, order);
	}
};

template<object_pointer P>
class atomic_ref<P> : public detail::_atomic_ref<P>
{
	using base = detail::_atomic_ref<P>;

public:
	using base::base;

	[[nodiscard]] vsm_always_inline P fetch_add(
		ptrdiff_t const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_add(operand, order);
	}

	[[nodiscard]] vsm_always_inline P fetch_sub(
		ptrdiff_t const operand,
		std::memory_order const order) const noexcept
	{
		return base::fetch_sub(operand, order);
	}
};

template<typename T>
atomic_ref(T&) -> atomic_ref<T>;

template<typename T>
atomic_ref(atomic<T>&) -> atomic_ref<T>;


/// @brief Stripped down version of std::atomic<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type stored by the atomic object.
template<typename T>
class atomic : public detail::_atomic<T>
{
public:
	using detail::_atomic<T>::_atomic;
};

template<std::integral T>
class atomic<T> : public detail::_atomic<T>
{
	using ref_type = atomic_ref<T>;

public:
	using detail::_atomic<T>::_atomic;

	[[nodiscard]] vsm_always_inline T fetch_add(
		T const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_add(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_sub(
		T const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_sub(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_and(
		T const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_and(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_or(
		T const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_or(operand, order);
	}

	[[nodiscard]] vsm_always_inline T fetch_xor(
		T const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_xor(operand, order);
	}
};

template<object_pointer P>
class atomic<P> : public detail::_atomic<P>
{
	using ref_type = atomic_ref<P>;

public:
	using detail::_atomic<P>::_atomic;

	[[nodiscard]] vsm_always_inline P fetch_add(
		ptrdiff_t const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_add(operand, order);
	}

	[[nodiscard]] vsm_always_inline P fetch_sub(
		ptrdiff_t const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_sub(operand, order);
	}
};

#if 0
template<detail::_atomic_enum E>
class atomic<E> : public detail::_atomic<E>
{
	using ref_type = atomic_ref<E>;

public:
	using detail::_atomic<E>::_atomic;

	[[nodiscard]] vsm_always_inline E fetch_and(
		E const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_and(operand, order);
	}

	[[nodiscard]] vsm_always_inline E fetch_or(
		E const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_or(operand, order);
	}

	[[nodiscard]] vsm_always_inline E fetch_xor(
		E const operand,
		std::memory_order const order) & noexcept
	{
		return ref_type(*this).fetch_xor(operand, order);
	}
};
#endif

} // namespace vsm
