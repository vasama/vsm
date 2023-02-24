#pragma once

#include <vsm/assert.h>
#include <vsm/atomic.hpp>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>
#include <vsm/utility.hpp>

#include <memory>

namespace vsm {
namespace detail {

template<typename T>
struct propagate_const_traits;


struct intrusive_ptr_acquire_tag {};


struct intrusive_ptr_acquire_cpo
{
	template<typename T>
	vsm_static_operator void operator()(intrusive_ptr_acquire_cpo const cpo, T* const ptr, size_t const count)() vsm_static_operator_const
		requires tag_invocable<intrusive_ptr_release_cpo, T*, size_t>
	{
		tag_invoke(cpo, ptr, count);
	}
};

struct intrusive_ptr_release_cpo
{
	template<typename T>
	vsm_static_operator void operator()(intrusive_ptr_release_cpo const cpo, T* const ptr, size_t const count)() vsm_static_operator_const
		requires tag_invocable<intrusive_ptr_release_cpo, T*, size_t>
	{
		tag_invoke(cpo, ptr, count);
	}
};


struct intrusive_ref_count_base
{
	using type = intrusive_ref_count_base;

	atomic<size_t> m_ref_count = 0;

	intrusive_ref_count_base() = default;

	intrusive_ref_count_base(intrusive_ptr_acquire_tag)
		: m_ref_count(1)
	{
	}
};

struct intrusive_ref_count_mutable_base
{
	using type = intrusive_ref_count_base const;

	atomic<size_t> mutable m_ref_count = 0;

	intrusive_ref_count_mutable_base() = default;

	intrusive_ref_count_mutable_base(intrusive_ptr_acquire_tag)
		: m_ref_count(1)
	{
	}
};

template<typename Base>
class basic_intrusive_ref_count : Base
{
protected:
	using Base::Base;

	basic_intrusive_ref_count(basic_intrusive_ref_count const&)
	{
	}

	basic_intrusive_ref_count& operator=(basic_intrusive_ref_count const&)
	{
		return *this;
	}

	~basic_intrusive_ref_count() = default;

	friend void tag_invoke(intrusive_ptr_acquire_cpo, typename Base::type* const ptr, size_t const count)
	{
		(void)ptr->m_ref_count.fetch_add(count, std::memory_order_relaxed);
	}

	friend void tag_invoke(intrusive_ptr_release_cpo, typename Base::type* const ptr, size_t const count)
	{
		size_t const old_count = ptr->m_ref_count.fetch_sub(count, std::memory_order_acq_rel);

		vsm_assert(count <= old_count);
	
		if (count == old_count)
		{
			delete ptr;
		}
	}
};

} // namespace detail

using intrusive_ref_count = detail::basic_intrusive_ref_count<detail::intrusive_ref_count_base>;
using intrusive_mutable_ref_count = detail::basic_intrusive_ref_count<detail::intrusive_ref_count_mutable_base>;

inline constexpr detail::intrusive_ptr_acquire_cpo intrusive_ptr_acquire = {};
inline constexpr detail::intrusive_ptr_release_cpo intrusive_ptr_release = {};

struct intrusive_ref_count_manager
{
	static void acquire(auto* const object, size_t const count)
	{
		intrusive_ptr_acquire(object, count);
	}

	static void release(auto* const object, size_t const count)
	{
		intrusive_ptr_release(object, count);
	}
};

template<typename T, typename Manager = intrusive_ref_count_manager>
class intrusive_ptr
{
	T* m_ptr;
	[[no_unique_address]] Manager m_manager;

public:
	constexpr intrusive_ptr() noexcept
		: m_ptr(nullptr)
	{
	}

	constexpr intrusive_ptr(decltype(nullptr)) noexcept
		: m_ptr(nullptr)
	{
	}

	explicit constexpr intrusive_ptr(T* const ptr) noexcept
		: m_ptr(ptr)
	{
		if (ptr != nullptr)
		{
			m_manager.acquire(ptr, 1);
		}
	}

	explicit constexpr intrusive_ptr(T* const ptr, any_cvref_of<Manager>&& manager) noexcept
		: m_ptr(ptr)
		, m_manager(vsm_forward(manager))
	{
		if (ptr != nullptr)
		{
			m_manager.acquire(ptr, 1);
		}
	}

	constexpr intrusive_ptr(intrusive_ptr&& other) noexcept
		: m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	constexpr intrusive_ptr(intrusive_ptr const& other) noexcept
		: m_ptr(other.m_ptr)
	{
		if (m_ptr != nullptr)
		{
			m_manager.acquire(m_ptr, 1);
		}
	}

	constexpr intrusive_ptr& operator=(intrusive_ptr&& other) & noexcept
	{
		T* const old_ptr = this->m_ptr;
		T* const new_ptr = other.m_ptr;

		this->m_ptr = nullptr;
		other.m_ptr = nullptr;

		if (old_ptr != nullptr)
		{
			m_manager.release(old_ptr, 1);
		}
		
		this->m_ptr = new_ptr;

		return *this;
	}

	constexpr intrusive_ptr& operator=(intrusive_ptr const& other) & noexcept
	{
		T* const old_ptr = this->m_ptr;
		T* const new_ptr = other.m_ptr;

		this->m_ptr = nullptr;

		if (new_ptr != nullptr)
		{
			m_manager.acquire(new_ptr, 1)
		}

		if (old_ptr != nullptr)
		{
			m_manager.release(old_ptr, 1);
		}
		
		this->m_ptr = new_ptr;

		return *this;
	}

	constexpr ~intrusive_ptr() noexcept
	{
		if (m_ptr != nullptr)
		{
			m_manager.release(m_ptr, 1);
		}
	}


	constexpr T* get() const
	{
		return m_ptr;
	}

	constexpr Manager const& get_manager() const
	{
		return m_manager;
	}

	constexpr T& operator*() const
	{
		vsm_assert(m_ptr != nullptr);
		return *m_ptr;
	}

	constexpr T* operator->() const
	{
		vsm_assert(m_ptr != nullptr);
		return *m_ptr;
	}
	
	
	constexpr void reset(T* const new_ptr)
	{
		T* const old_ptr = m_ptr;
		m_ptr = nullptr;
		
		if (new_ptr != nullptr)
		{
			m_manager.acquire(new_ptr);
		}
		m_ptr = new_ptr;
		
		if (old_ptr != nullptr)
		{
			m_manager.release(old_ptr);
		}
	}


	constexpr T* release()
	{
		T* const ptr = m_ptr;
		m_ptr = nullptr;
		return ptr;
	}

	static intrusive_ptr acquire(T* const ptr)
	{
		return intrusive_ptr(detail::intrusive_ptr_acquire_tag(), ptr);
	}

	static intrusive_ptr acquire(T* const ptr, any_cvref_of<Manager> auto&& manager)
	{
		return intrusive_ptr(detail::intrusive_ptr_acquire_tag(), ptr, vsm_forward(manager));
	}


	friend auto operator<=>(intrusive_ptr const&, intrusive_ptr const&) = default;

private:
	explicit intrusive_ptr(detail::intrusive_ptr_acquire_tag, T* const ptr)
		: m_ptr(ptr)
	{
	}

	explicit intrusive_ptr(detail::intrusive_ptr_acquire_tag, T* const ptr, any_cvref_of<Manager>&& manager)
		: m_ptr(ptr)
		, m_manager(manager)
	{
	}
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

	static pointer pointer_to(T* const ptr)
	{
		return ptr;
	}

	static T* to_address(pointer const& ptr)
	{
		return ptr.get();
	}
};
