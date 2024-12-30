#pragma once

#include <vsm/platform.h>

#include <memory>
#include <new>

namespace vsm {
namespace detail {

template<typename>
struct _self;

template<typename Interface>
struct _self<Interface*>
{
	template<typename T>
	using type = T*;
};

template<typename Interface>
struct _self<Interface const*>
{
	template<typename T>
	using type = T const*;
};

} // namespace detail

class partial
{
#if vsm_compiler_clang && __clang_major__ <= 19

	template<typename T>
	struct access
	{
		using internal_class = T::internal_class;
		using private_class = T::private_class;
	};

public:
	template<typename T>
	using internal_class = access<T>::internal_class;

	template<typename T>
	using private_class = access<T>::private_class;

#else

public:
	template<typename T>
	using internal_class = T::internal_class;

	template<typename T>
	using private_class = T::private_class;

#endif
};

// As of Dec 2024, only GCC permits destroying delete expressions on types with inaccessible
// destructors. For other compilers, the destructor is made public in order to make the use of
// destroying delete possible.
#if vsm_compiler_gcc
#	define vsm_detail_partial_1 protected
#else
#	define vsm_detail_partial_1 public
#endif

// NOLINTBEGIN(bugprone-macro-parentheses)

#define vsm_partial(T) \
	protected: \
		class internal_class; \
		class private_class; \
		T() = default; \
		T(T&&) = default; \
		T(T const&) = default; \
		T& operator=(T&&) = default; \
		T& operator=(T const&) = default; \
	vsm_detail_partial_1: \
		~T() = default; \
	private: \
		friend private_class; \
		friend internal_class; \
		friend ::vsm::partial \

#define vsm_partial_delete(T) \
	public: \
		static void operator delete(T* ptr, ::std::destroying_delete_t) \

#define vsm_partial_internal(T) \
	private: \
		friend T; \
		friend T::private_class

#define vsm_self(T) \
	auto* const self = static_cast<::vsm::detail::_self<decltype(this)>::template type<T>>(this)

// NOLINTEND(bugprone-macro-parentheses)

} // namespace vsm
