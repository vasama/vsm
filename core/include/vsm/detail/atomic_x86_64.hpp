#pragma once

#include <vsm/platform.h>

#include <atomic>

namespace vsm::detail {

#pragma push_macro("cmpxchg16")

#if vsm_compiler_msvc
#	pragma intrinsic(_InterlockedCompareExchange128)

#	define cmpxchg16(object, expected, desired) \
		_InterlockedCompareExchange128( \
			(long long*)&(object), \
			((long long const*)(&(desired)))[1], \
			((long long const*)(&(desired)))[0], \
			(long long*)&(expected) \
		)
#else
template<typename T>
vsm_always_inline bool cmpxchg16(T& object, T& expected, T const& desired)
{
	T const old_expected = expected;
	expected = __sync_val_compare_and_swap(&object, expected, desired);
	return old_expected == expected;
}
#endif

template<typename T>
class atomic_ref_x86_64
{
	T* m_object;

public:
	static constexpr bool is_always_lock_free = true;
	static constexpr size_t required_alignment = 16;

	explicit atomic_ref_x86_64(T& object)
		: m_object(&object)
	{
	}


	T load([[maybe_unused]] std::memory_order const memory_order) const noexcept
	{
		T expected = {};
		cmpxchg16(*m_object, expected, expected);
		return expected;
	}

	void store(T const value, [[maybe_unused]] std::memory_order const memory_order) const noexcept
	{
		T& object = *m_object;

		T expected = object;
		while (!cmpxchg16(object, expected, value));
	}

	T exchange(T const value, [[maybe_unused]] std::memory_order const memory_order) const noexcept
	{
		T& object = *m_object;

		T expected = object;
		while (!cmpxchg16(object, expected, value));
		return expected;
	}

	bool compare_exchange_weak(
		T& expected, T const desired,
		[[maybe_unused]] std::memory_order const success,
		[[maybe_unused]] std::memory_order const failure) const noexcept
	{
		return cmpxchg16(*m_object, expected, desired);
	}

	bool compare_exchange_strong(
		T& expected, T const desired,
		[[maybe_unused]] std::memory_order const success,
		[[maybe_unused]] std::memory_order const failure) const noexcept
	{
		return cmpxchg16(*m_object, expected, desired);
	}
};

#pragma pop_macro("cmpxchg16")

} // namespace vsm::detail
