#pragma once

#include <vsm/platform.h>
#include <vsm/type_traits.hpp>

#include <atomic>

namespace vsm::detail {

#pragma push_macro("cmpxchg16")

#if vsm_compiler_msvc
#	pragma intrinsic(_InterlockedCompareExchange128)

#	define cmpxchg16(object, expected, desired) \
		_InterlockedCompareExchange128( \
			(long long*)std::addressof(object), \
			((long long const*)(std::addressof(desired)))[1], \
			((long long const*)(std::addressof(desired)))[0], \
			(long long*)std::addressof(expected) \
		)
#else
template<typename T>
inline vsm_always_inline bool cmpxchg16(T& object, T& expected, T const& desired)
{
	// Read the bits of the expected value.
	__uint128_t const old_expected = __builtin_bit_cast(__uint128_t, expected);

	// Compare-exchange the bits at object, returning previous bits.
	__uint128_t const old_object = __sync_val_compare_and_swap(
		reinterpret_cast<__uint128_t*>(std::addressof(object)),
		old_expected,
		__builtin_bit_cast(__uint128_t, desired));

	// Update the bits of the user's expected value.
	expected = __builtin_bit_cast(T, old_object);

	// The compare-exchange succeeded if the bits match.
	return old_object == old_expected;
}
#endif

template<typename T>
class atomic_ref_base
{
#if vsm_compiler_clang
// LLVM produces bad code for cmpxchg16b in some cases:
// https://github.com/llvm/llvm-project/issues/119959
#define vsm_detail_atomic_inline vsm_never_inline
#else
#define vsm_detail_atomic_inline
#endif

	static_assert(std::is_trivially_copyable_v<T>);

	T* m_object;

public:
	static constexpr bool is_always_lock_free = true;
	static constexpr size_t required_alignment = 16;

	explicit atomic_ref_base(T& object)
		: m_object(std::addressof(object))
	{
	}

	vsm_clang_diagnostic(push)
	vsm_clang_diagnostic(ignored "-Wold-style-cast")


	vsm_detail_atomic_inline T load(
		[[maybe_unused]] std::memory_order const memory_order) const noexcept
	{
		T expected = {};
		cmpxchg16(*m_object, expected, expected);
		return expected;
	}

	vsm_detail_atomic_inline void store(
		T const value,
		[[maybe_unused]] std::memory_order const memory_order) const noexcept
	{
		T& object = *m_object;

		T expected = object;
		while (!cmpxchg16(object, expected, value));
	}

	vsm_detail_atomic_inline T exchange(
		T const value,
		[[maybe_unused]] std::memory_order const memory_order) const noexcept
	{
		T& object = *m_object;

		T expected = object;
		while (!cmpxchg16(object, expected, value));
		return expected;
	}

	vsm_detail_atomic_inline bool compare_exchange_weak(
		T& expected,
		T const desired,
		[[maybe_unused]] std::memory_order const success,
		[[maybe_unused]] std::memory_order const failure) const noexcept
	{
		return cmpxchg16(*m_object, expected, desired);
	}

	vsm_detail_atomic_inline bool compare_exchange_strong(
		T& expected,
		T const desired,
		[[maybe_unused]] std::memory_order const success,
		[[maybe_unused]] std::memory_order const failure) const noexcept
	{
		return cmpxchg16(*m_object, expected, desired);
	}

	vsm_clang_diagnostic(pop)

#undef vsm_detail_atomic_inline
};

#define vsm_detail_atomic_ref_base(T) \
	select_t<sizeof(T) == 16, detail::atomic_ref_base<T>, std::atomic_ref<T>>

#pragma pop_macro("cmpxchg16")

} // namespace vsm::detail
