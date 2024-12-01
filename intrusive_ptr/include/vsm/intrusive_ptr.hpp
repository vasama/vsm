#pragma once

#include <vsm/allocator.hpp>
#include <vsm/assert.h>
#include <vsm/atomic.hpp>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>
#include <vsm/utility.hpp>

#include <memory>

namespace vsm {
namespace detail {

struct intrusive_ptr_adopt_t {};


struct intrusive_ptr_deleter_cpo
{
	template<typename Manager, typename T>
	friend void tag_invoke(intrusive_ptr_deleter_cpo, Manager const& manager, T* const ptr)
		requires requires { manager.deleter(ptr); }
	{
		manager.deleter(ptr);
	}

	template<typename Manager, typename T>
	vsm_static_operator void operator()(Manager const& manager, T* const ptr) vsm_static_operator_const
		requires tag_invocable<intrusive_ptr_deleter_cpo, Manager const&, T*>
	{
		vsm::tag_invoke(intrusive_ptr_deleter_cpo(), manager, ptr);
	}
};

struct intrusive_ptr_acquire_cpo
{
	template<typename Manager, typename T>
	vsm_static_operator void operator()(
		Manager const& manager,
		T* const ptr,
		size_t const count) vsm_static_operator_const noexcept
		requires tag_invocable<intrusive_ptr_acquire_cpo, Manager const&, T*, size_t>
	{
		vsm::tag_invoke(intrusive_ptr_acquire_cpo(), manager, ptr, count);
	}
};

struct intrusive_ptr_release_cpo
{
	template<typename Manager, typename T>
	vsm_static_operator void operator()(
		Manager const& manager,
		T* const ptr,
		size_t const count) vsm_static_operator_const noexcept
		requires tag_invocable<intrusive_ptr_release_cpo, Manager const&, T*, size_t>
	{
		vsm::tag_invoke(intrusive_ptr_release_cpo(), manager, ptr, count);
	}
};


template<typename Counter>
struct intrusive_refcount_base
{
	template<typename T>
	using type = T;

	Counter m_refcount;

	explicit intrusive_refcount_base(size_t const refcount)
		: m_refcount(refcount)
	{
	}
};

template<typename Counter>
struct intrusive_refcount_mutable_base
{
	template<typename T>
	using type = T const;

	Counter mutable m_refcount;

	explicit intrusive_refcount_mutable_base(size_t const refcount)
		: m_refcount(refcount)
	{
	}
};

template<typename Base>
class basic_intrusive_refcount : Base
{
protected:
	basic_intrusive_refcount() noexcept
		: Base(0)
	{
	}

	explicit basic_intrusive_refcount(size_t const refcount) noexcept
		: Base(refcount)
	{
	}

	basic_intrusive_refcount(basic_intrusive_refcount const&) noexcept
	{
	}

	basic_intrusive_refcount& operator=(basic_intrusive_refcount const&) noexcept
	{
		return *this;
	}

	~basic_intrusive_refcount() = default;


	template<typename Manager, std::derived_from<basic_intrusive_refcount> T>
	friend void tag_invoke(
		intrusive_ptr_acquire_cpo,
		Manager const& manager,
		typename Base::template type<T>* const ptr,
		size_t const count) noexcept
	{
		(void)ptr->m_refcount.fetch_add(count, std::memory_order_relaxed);
	}

	template<typename Manager, std::derived_from<basic_intrusive_refcount> T>
	friend void tag_invoke(
		intrusive_ptr_release_cpo,
		Manager const& manager,
		typename Base::template type<T>* const ptr,
		size_t const count) noexcept
	{
		size_t const old_count = ptr->m_refcount.fetch_sub(count, std::memory_order_acq_rel);

		vsm_assert(count <= old_count);

		if (count == old_count)
		{
			intrusive_ptr_deleter_cpo()(manager, ptr);
		}
	}
};


template<typename From, typename To>
concept _intrusive_ptr_convertible =
	std::convertible_to<From*, To*> && (
		std::is_same_v<remove_cv_t<From>, remove_cv_t<To>> ||
		std::has_virtual_destructor_v<remove_cv_t<To>> ||
		has_destroying_delete_v<remove_cv_t<To>>);


template<typename Manager, typename T>
concept _intrusive_ptr_manager = requires (Manager const& manager, T* const ptr, size_t const count)
{
	{ manager.acquire(ptr, count) };
	{ manager.release(ptr, count) } noexcept;
};

template<typename Manager, typename T>
concept _intrusive_ptr_nothrow_manager =
	_intrusive_ptr_manager<Manager, T> &&
	requires (Manager const& manager, T* const ptr, size_t const count)
	{
		{ manager.acquire(ptr, count) } noexcept;
	};

} // namespace detail

template<typename Counter>
using basic_intrusive_refcount =
	detail::basic_intrusive_refcount<detail::intrusive_refcount_base<Counter>>;

template<typename Counter>
using basic_intrusive_mutable_refcount =
	detail::basic_intrusive_refcount<detail::intrusive_refcount_mutable_base<Counter>>;

using intrusive_refcount = basic_intrusive_refcount<atomic<size_t>>;
using intrusive_mutable_refcount = basic_intrusive_mutable_refcount<atomic<size_t>>;

inline constexpr detail::intrusive_ptr_deleter_cpo intrusive_ptr_deleter = {};
inline constexpr detail::intrusive_ptr_acquire_cpo intrusive_ptr_acquire = {};
inline constexpr detail::intrusive_ptr_release_cpo intrusive_ptr_release = {};

struct default_refcount_manager
{
	template<typename T>
	void deleter(T* const object) const noexcept
	{
		std::default_delete<T>()(object);
	}

	template<typename T>
	void acquire(T* const object, size_t const count) const
		noexcept(noexcept(intrusive_ptr_acquire(*this, object, count)))
	{
		intrusive_ptr_acquire(*this, object, count);
	}

	template<typename T>
	void release(T* const object, size_t const count) const
		noexcept(noexcept(intrusive_ptr_release(*this, object, count)))
	{
		intrusive_ptr_release(*this, object, count);
	}
};

template<allocator Allocator>
class basic_refcount_manager
{
	vsm_no_unique_address Allocator m_allocator = {};

public:
	basic_refcount_manager()
		requires std::is_default_constructible_v<Allocator> = default;

	explicit basic_refcount_manager(any_cvref_of<Allocator> auto&& allocator)
	{
	}

	[[nodiscard]] Allocator const& get_allocator() const noexcept
	{
		return m_allocator;
	}

	template<typename T>
	void deleter(T* const object) const noexcept
	{
		vsm::delete_via(m_allocator, object);
	}

	template<typename T>
	void acquire(T* const object, size_t const count) const noexcept
	{
		intrusive_ptr_acquire(*this, object, count);
	}

	template<typename T>
	void release(T* const object, size_t const count) const noexcept
	{
		intrusive_ptr_release(*this, object, count);
	}
};

template<non_ref T, detail::_intrusive_ptr_manager<T> Manager = default_refcount_manager>
class intrusive_ptr
{
	static_assert(std::is_nothrow_move_constructible_v<Manager>);
	static_assert(std::is_nothrow_move_assignable_v<Manager>);
	static_assert(std::is_nothrow_swappable_v<Manager>);

	static constexpr bool has_nothrow_acquire =
		noexcept(std::declval<Manager const&>().acquire(static_cast<T*>(0), size_t(1)));

	T* m_ptr;
	vsm_no_unique_address Manager m_manager;

public:
	constexpr intrusive_ptr() noexcept
		: m_ptr(nullptr)
	{
	}

	constexpr intrusive_ptr(decltype(nullptr))
		noexcept(std::is_nothrow_default_constructible_v<Manager>)
		requires std::is_default_constructible_v<Manager>
		: m_ptr(nullptr)
	{
	}

	template<any_cvref_of<Manager> NewManager>
		requires std::is_constructible_v<Manager, NewManager>
	constexpr intrusive_ptr(decltype(nullptr), NewManager&& manager)
		noexcept(has_nothrow_acquire && std::is_nothrow_constructible_v<Manager, NewManager>)
		: m_ptr(nullptr)
		, m_manager(vsm_forward(manager))
	{
	}

	template<detail::_intrusive_ptr_convertible<T> U = T>
		requires std::is_default_constructible_v<Manager>
	explicit constexpr intrusive_ptr(U* const ptr)
		noexcept(has_nothrow_acquire && std::is_nothrow_default_constructible_v<Manager>)
		: m_ptr(ptr)
	{
		if (ptr != nullptr)
		{
			vsm_as_const(m_manager).acquire(ptr, static_cast<size_t>(1));
		}
	}

	template<any_cvref_of<Manager> NewManager, detail::_intrusive_ptr_convertible<T> U = T>
		requires std::is_constructible_v<Manager, NewManager>
	explicit constexpr intrusive_ptr(U* const ptr, NewManager&& manager)
		noexcept(has_nothrow_acquire && std::is_nothrow_constructible_v<Manager, NewManager>)
		: m_ptr(ptr)
		, m_manager(vsm_forward(manager))
	{
		if (ptr != nullptr)
		{
			vsm_as_const(m_manager).acquire(ptr, static_cast<size_t>(1));
		}
	}

	constexpr intrusive_ptr(intrusive_ptr&& other) noexcept
		: m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	template<detail::_intrusive_ptr_convertible<T> U = T>
	constexpr intrusive_ptr(intrusive_ptr<U, Manager>&& other) noexcept
		: m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	constexpr intrusive_ptr(intrusive_ptr const& other)
		noexcept(has_nothrow_acquire)
		: m_ptr(other.m_ptr)
	{
		if (m_ptr != nullptr)
		{
			vsm_as_const(m_manager).acquire(m_ptr, static_cast<size_t>(1));
		}
	}

	template<detail::_intrusive_ptr_convertible<T> U = T>
		requires std::is_copy_constructible_v<Manager>
	constexpr intrusive_ptr(intrusive_ptr<U, Manager> const& other)
		noexcept(has_nothrow_acquire && std::is_nothrow_copy_constructible_v<Manager>)
		: m_ptr(other.m_ptr)
		, m_manager(other.m_manager)
	{
		if (m_ptr != nullptr)
		{
			vsm_as_const(m_manager).acquire(m_ptr, static_cast<size_t>(1));
		}
	}

	constexpr intrusive_ptr& operator=(decltype(nullptr)) & noexcept
	{
		if (T* const old_ptr = m_ptr)
		{
			m_ptr = nullptr;
			vsm_as_const(m_manager).release(old_ptr, static_cast<size_t>(1));
		}

		return *this;
	}

	constexpr intrusive_ptr& operator=(intrusive_ptr&& other) & noexcept
	{
		intrusive_ptr(vsm_move(other)).swap(*this);
		return *this;
	}

	template<detail::_intrusive_ptr_convertible<T> U = T>
	constexpr intrusive_ptr& operator=(intrusive_ptr<U, Manager>&& other) & noexcept
	{
		intrusive_ptr(vsm_move(other)).swap(*this);
		return *this;
	}

	constexpr intrusive_ptr& operator=(intrusive_ptr const& other) &
		noexcept(has_nothrow_acquire && std::is_nothrow_copy_assignable_v<Manager>)
		requires std::is_copy_assignable_v<Manager>
	{
		intrusive_ptr(vsm_move(other)).swap(*this);
		return *this;
	}

	template<detail::_intrusive_ptr_convertible<T> U = T>
		requires std::is_copy_assignable_v<Manager>
	constexpr intrusive_ptr& operator=(intrusive_ptr<U, Manager> const& other) &
		noexcept(has_nothrow_acquire && std::is_nothrow_copy_assignable_v<Manager>)
	{
		intrusive_ptr(vsm_move(other)).swap(*this);
		return *this;
	}

	constexpr ~intrusive_ptr()
	{
		if (m_ptr != nullptr)
		{
			vsm_as_const(m_manager).release(m_ptr, static_cast<size_t>(1));
		}
	}


	[[nodiscard]] constexpr T* get() const noexcept
	{
		return m_ptr;
	}

	[[nodiscard]] constexpr Manager const& get_manager() const noexcept
	{
		return m_manager;
	}


	[[nodiscard]] constexpr T& operator*() const
	{
		vsm_assert(m_ptr != nullptr);
		return *m_ptr;
	}

	[[nodiscard]] constexpr T* operator->() const
	{
		vsm_assert(m_ptr != nullptr);
		return m_ptr;
	}


	constexpr void reset(decltype(nullptr) = nullptr) & noexcept
	{
		if (T* const ptr = m_ptr)
		{
			m_ptr = nullptr;
			vsm_as_const(m_manager).release(ptr, static_cast<size_t>(1));
		}
	}

	constexpr void reset(T* const new_ptr) &
		noexcept(has_nothrow_acquire)
	{
		if (new_ptr != nullptr)
		{
			vsm_as_const(m_manager).acquire(new_ptr, static_cast<size_t>(1));
		}

		T* const old_ptr = m_ptr;
		m_ptr = new_ptr;

		if (old_ptr != nullptr)
		{
			vsm_as_const(m_manager).release(old_ptr, static_cast<size_t>(1));
		}
	}

	template<any_cvref_of<Manager> NewManager>
		requires std::is_constructible_v<Manager, NewManager>
	constexpr void reset(T* const new_ptr, NewManager&& new_manager) &
		noexcept(has_nothrow_acquire && std::is_nothrow_constructible_v<Manager, NewManager>)
	{
		intrusive_ptr(new_ptr, vsm_forward(new_manager)).swap(*this);
	}


	[[nodiscard]] constexpr T* release() noexcept
	{
		T* const ptr = m_ptr;
		m_ptr = nullptr;
		return ptr;
	}

	[[nodiscard]] constexpr std::pair<T*, Manager> release_with_manager() noexcept
	{
		T* const ptr = m_ptr;
		m_ptr = nullptr;
		return { ptr, vsm_move(m_manager) };
	}


	[[nodiscard]] static intrusive_ptr adopt(T* const ptr)
		noexcept(std::is_nothrow_default_constructible_v<Manager>)
		requires std::is_default_constructible_v<Manager>
	{
		return intrusive_ptr(detail::intrusive_ptr_adopt_t(), ptr);
	}

	template<any_cvref_of<Manager> NewManager>
		requires std::is_constructible_v<Manager, NewManager>
	[[nodiscard]] static intrusive_ptr adopt(T* const ptr, NewManager&& manager)
		noexcept(std::is_nothrow_constructible_v<Manager, NewManager>)
	{
		return intrusive_ptr(detail::intrusive_ptr_adopt_t(), ptr, vsm_forward(manager));
	}


	void swap(intrusive_ptr& other) noexcept
	{
		std::swap(this->m_ptr, other.m_ptr);

		using std::swap;
		swap(this->m_manager, other.m_manager);
	}

	friend void swap(intrusive_ptr& lhs, intrusive_ptr& rhs) noexcept
	{
		std::swap(lhs.m_ptr, rhs.m_ptr);

		using std::swap;
		swap(lhs.m_manager, rhs.m_manager);
	}


	[[nodiscard]] explicit operator bool() const
	{
		return m_ptr != nullptr;
	}


	[[nodiscard]] friend bool operator==(intrusive_ptr const& lhs, intrusive_ptr const& rhs)
	{
		return lhs.m_ptr == rhs.m_ptr;
	}

	[[nodiscard]] friend bool operator==(intrusive_ptr const& lhs, T const* const rhs)
	{
		return lhs.m_ptr == rhs;
	}

	[[nodiscard]] friend bool operator==(intrusive_ptr const& ptr, decltype(nullptr))
	{
		return ptr.m_ptr == nullptr;
	}

	[[nodiscard]] friend auto operator<=>(intrusive_ptr const& lhs, intrusive_ptr const& rhs)
	{
		return std::compare_three_way()(lhs.m_ptr, rhs.m_ptr);
	}

	[[nodiscard]] friend auto operator<=>(intrusive_ptr const& lhs, T const* const rhs)
	{
		return std::compare_three_way()(lhs.m_ptr, rhs);
	}

	[[nodiscard]] friend auto operator<=>(intrusive_ptr const& ptr, decltype(nullptr))
	{
		return std::compare_three_way()(ptr.m_ptr, static_cast<T*>(nullptr));
	}

private:
	explicit intrusive_ptr(detail::intrusive_ptr_adopt_t, T* const ptr)
		: m_ptr(ptr)
	{
	}

	explicit intrusive_ptr(
		detail::intrusive_ptr_adopt_t,
		T* const ptr,
		any_cvref_of<Manager> auto&& manager)
		: m_ptr(ptr)
		, m_manager(manager)
	{
	}

	template<non_ref U, detail::_intrusive_ptr_manager<U>>
	friend class intrusive_ptr;
};

} // namespace vsm

template<typename T, typename Manager>
struct std::pointer_traits<vsm::intrusive_ptr<T, Manager>>
{
	using pointer = vsm::intrusive_ptr<T, Manager>;
	using element_type = T;
	using difference_type = ptrdiff_t;

	template<typename U>
	using rebind = vsm::intrusive_ptr<U, Manager>;

	[[nodiscard]] static pointer pointer_to(T* const ptr)
	{
		return ptr;
	}

	[[nodiscard]] static T* to_address(pointer const& ptr)
	{
		return ptr.get();
	}
};
