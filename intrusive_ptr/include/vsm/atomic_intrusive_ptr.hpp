#pragma once

#include <vsm/atomic.hpp>
#include <vsm/intrusive_ptr.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <atomic>

namespace vsm {

template<non_ref T, detail::_intrusive_ptr_nothrow_manager<T> Manager = default_refcount_manager>
class atomic_intrusive_ptr
{
	static_assert(std::is_nothrow_copy_constructible_v<Manager>);
	static_assert(std::is_nothrow_copy_assignable_v<Manager>);

	struct atom
	{
		// The object of which this intrusive pointer is a shared owner.
		T* ptr;

		// The number of temporary references to the shared object. A temporary reference is as good
		// for using the shared object as a reference placed on the shared object itself. Any
		// mutation of the atomic_intrusive_ptr, i.e. replacement of the internal pointer, transfers
		// temporary references to the shared object itself.
		size_t refcount;
	};

	mutable atomic<atom> m_atom = {};
	vsm_no_unique_address Manager m_manager = {};

public:
	atomic_intrusive_ptr() = default;

	explicit atomic_intrusive_ptr(T* const ptr)
		noexcept(std::is_nothrow_default_constructible_v<Manager>)
		requires std::is_default_constructible_v<Manager>
		: m_atom({ ptr, 0 })
	{
		if (ptr != nullptr)
		{
			m_manager.acquire(ptr, 1);
		}
	}

	template<any_cvref_of<Manager> NewManager>
		requires std::is_constructible_v<Manager, NewManager>
	explicit atomic_intrusive_ptr(T* const ptr, NewManager&& manager) noexcept
		: m_atom({ ptr, 0 })
		, m_manager(vsm_forward(manager))
	{
		static_assert(std::is_nothrow_constructible_v<Manager, NewManager>);

		if (ptr != nullptr)
		{
			vsm_as_const(m_manager).acquire(ptr, 1);
		}
	}

	atomic_intrusive_ptr(intrusive_ptr<T, Manager> ptr) noexcept
		: atomic_intrusive_ptr(ptr.release_with_manager())
	{
	}

	atomic_intrusive_ptr(const atomic_intrusive_ptr&) = delete;
	atomic_intrusive_ptr& operator=(const atomic_intrusive_ptr&) = delete;

	~atomic_intrusive_ptr()
	{
		// Destructors never race. Any other thread using this object must have been synchronised
		// with using some other mechanism. Therefore a relaxed load is fine here.
		atom const atom = m_atom.load(std::memory_order::relaxed);

		// Mutations always flush the temporary refcount as the last step. A non-zero refcount could
		// only ever be observed here as a result of a race.
		vsm_assert(atom.refcount == 0);

		if (atom.ptr != nullptr)
		{
			vsm_as_const(m_manager).release(atom.ptr, 1);
		}
	}


	/// @brief Get a raw pointer to the shared object without sychronisation.
	/// @note This function can easily lead to data races, so be extremely careful when using it.
	/// @return A raw pointer to the shared object, if any.
	[[nodiscard]] T* get_relaxed() const noexcept
	{
		atom const atom = m_atom.load(std::memory_order::relaxed);

		// A non-zero refcount could only ever be observed here as a result of a race.
		vsm_assert(atom.refcount == 0);

		return atom.ptr;
	}

	/// @brief Get the held manager object.
	/// @return A reference to the held manager object.
	[[nodiscard]] Manager const& get_manager() const noexcept
	{
		return m_manager;
	}


	/// @brief Get a pointer to the shared object.
	/// @return An intrusive_ptr managing the shared object, if any.
	[[nodiscard]] intrusive_ptr<T, Manager> load() const noexcept
	{
		atom atom = m_atom.load(std::memory_order::acquire);

		// Increment the temporary refcount on the shared atom.
		while (true)
		{
			// If the pointer is null, there is no need to continue.
			if (atom.ptr == nullptr)
			{
				return intrusive_ptr<T, Manager>(nullptr, m_manager);
			}

			vsm_assert_slow(atom.refcount >= 0);

			// Loop until the atom's temporary refcount is succesfully incremented.
			if (m_atom.compare_exchange_weak(
				atom,
				{ atom.ptr, atom.refcount + 1 },
				std::memory_order::release,
				std::memory_order::acquire))
			{
				break;
			}
		}

		// Increment the local atom's refcount to reflect the mutation applied to the shared atom.
		++atom.refcount;

		// Having created a temporary reference, the shared object may now be safely used by this
		// thread.
		T* const ptr = atom.ptr;

		// The shared object's refcount is incremented in order to acquire long term shared
		// ownership.
		m_manager.acquire(ptr, 1);

		// The previously created temporary reference must now be removed.
		while (true)
		{
			// Loop until the shared atom's temporary refcount is succesfully decremented.
			if (m_atom.compare_exchange_weak(
				atom,
				{ ptr, atom.refcount - 1 },
				std::memory_order::release,
				std::memory_order::acquire))
			{
				break;
			}

			if (atom.ptr != ptr || atom.refcount == 0)
			{
				// If the pointer has changed, another thread has transferred this thread's
				// temporary reference to the shared object, meaning this thread must now remove the
				// reference from the shared object instead of the shared atom. If the temporary
				// refcount is 0, other threads must have replaced the shared object, and
				// subsequently restored the original.
				m_manager.release(ptr, 1);

				// With the pointer changed, there is no longer any need to mutate the atom.
				break;
			}
		}

		// This thread is left holding one reference on the shared object.
		return intrusive_ptr<T, Manager>::adopt(ptr, m_manager);
	}

	/// @brief Get a pointer to the shared object without sychronisation.
	/// @note This function can easily lead to data races, so be extremely careful when using it.
	/// @return An intrusive_ptr managing the shared object, if any.
	[[nodiscard]] intrusive_ptr<T, Manager> load_relaxed() const noexcept
	{
		return intrusive_ptr<T, Manager>(get_relaxed(), m_manager);
	}


	/// @brief Replace the shared object.
	void store(intrusive_ptr<T, Manager> new_ptr) noexcept
	{
		// This thread is left holding a reference to the previous shared object after the exchange
		// is done. The reference is removed by the destruction of the returned intrusive_ptr.
		(void)exchange(vsm_move(new_ptr));
	}

	/// @brief Replace the shared object without synchronisation.
	/// @note This function can easily lead to data races, so be extremely careful when using it.
	void store_relaxed(intrusive_ptr<T, Manager> new_ptr) noexcept
	{
		(void)exchange_relaxed(vsm_move(new_ptr));
	}


	/// @brief Exchange the shared object.
	[[nodiscard]] intrusive_ptr<T, Manager> exchange(intrusive_ptr<T, Manager> new_ptr) noexcept
	{
		// Releasing the pointer to the new shared object from the local intrusive_ptr and into the
		// desired atom ensures ownership of this atomic_intrusive_ptr. This function will
		// eventually replace the shared atom with the desired atom.
		atom const desired = { new_ptr.release(), 0 };

		while (true)
		{
			atom atom = m_atom.load(std::memory_order::acquire);

			// Increment the temporary refcount on the shared atom.
			while (true)
			{
				// If the pointer is null, it is enough to simply exchange the shared atom.
				if (atom.ptr == nullptr && m_atom.compare_exchange_weak(
					atom,
					desired,
					std::memory_order::release,
					std::memory_order::acquire))
				{
					return intrusive_ptr<T, Manager>(nullptr, m_manager);
				}

				// Loop until the shared atom's local refcount is succesfully incremented.
				if (m_atom.compare_exchange_weak(
					atom,
					{ atom.ptr, atom.refcount + 1 },
					std::memory_order::release,
					std::memory_order::acquire))
				{
					break;
				}
			}

			// Increment the local atom's refcount to reflect the mutation applied to the shared
			// atom.
			++atom.refcount;

			// Having created a temporary reference, the shared object may now be safely used by
			// this thread.
			T* const ptr = atom.ptr;

			// Keep track of how many temporary references have been transferred from the shared
			// atom to the shared object.
			size_t refcount = 1;

			while (true)
			{
				if (atom.refcount > refcount)
				{
					// Any temporary references on the shared atom must be added to the shared
					// object.
					m_manager.acquire(ptr, atom.refcount - refcount);
					refcount = atom.refcount;
				}

				if (m_atom.compare_exchange_weak(
					atom,
					desired,
					std::memory_order::release,
					std::memory_order::acquire))
				{
					if (refcount > atom.refcount)
					{
						// Once the shared atom has been succesfully replaced, in case other threads
						// won the race to remove their temporary references from the shared atom,
						// the extraneous references on the shared object held by this thread are
						// removed.
						m_manager.release(ptr, refcount - atom.refcount);
					}

					// All temporary references have been transferred to the shared object. Any
					// thread with a temporary reference is now holding it on the shared object
					// instead.

					// The temporary reference created by this thread is cancelled out by the
					// release to the caller of the reference on the shared object held by this
					// atomic_intrusive_ptr.
					return intrusive_ptr<T, Manager>::adopt(ptr, vsm_as_const(m_manager));
				}

				if (atom.ptr != ptr)
				{
					// If the pointer in the shared atom has changed, the race to exchange it is
					// lost and this thread must start the operation over again from the beginning.

					// Any temporary references have also been transferred by the racing thread.
					// Having lost the race, the temporary references transferred by this thread
					// must be removed from the shared object. This thread may have been left
					// holding the final reference to the shared object, in which case it will be
					// deleted here.
					m_manager.release(ptr, refcount);

					break;
				}
			}
		}
	}

	/// @brief Exchange the shared object without synchronisation.
	/// @note This function can easily lead to data races, so be extremely careful when using it.
	[[nodiscard]] intrusive_ptr<T, Manager> exchange_relaxed(
		intrusive_ptr<T, Manager> new_ptr) noexcept
	{
		atom const atom = m_atom.exchange({ new_ptr.release(), 0 }, std::memory_order::relaxed);

		// A non-zero refcount could only ever be observed here as a result of a race.
		vsm_assert(atom.refcount == 0);

		return intrusive_ptr<T, Manager>::adopt(atom.ptr, m_manager);
	}

private:
	explicit atomic_intrusive_ptr(std::pair<T*, Manager> pair) noexcept
		: m_atom({ pair.first, static_cast<size_t>(0) })
		, m_manager(vsm_move(pair.second))
	{
	}
};

} // namespace vsm
