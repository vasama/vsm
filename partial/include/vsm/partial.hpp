#pragma once

#include <memory>
#include <new>

namespace vsm {
namespace detail {

template<typename>
struct self_traits;

template<typename Interface>
struct self_traits<Interface*>
{
	template<typename T>
	using type = T*;
};

template<typename Interface>
struct self_traits<Interface const*>
{
	template<typename T>
	using type = T const*;
};

} // namespace detail

class partial
{
#if vsm_compiler_clang
#	if __clang_major__ >= 19
#		error Remove this workaround.
#	endif

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

#define vsm_partial(T) \
	protected: \
		class internal_class; \
		class private_class; \
		T() = default; \
		T(T&&) = delete; \
		T(T const&) = delete; \
		T& operator=(T&&) = delete; \
		T& operator=(T const&) = delete; \
		~T() = default; \
	private: \
		friend ::vsm::partial \

#define vsm_partial_delete(T) \
	public: \
		static void operator delete(T* ptr, ::std::destroying_delete_t); \
	private: \
		friend ::std::default_delete<T> \

#define vsm_partial_internal(T) \
	private: \
		friend T; \
		friend T::private_class

#define vsm_self(T) \
	auto* const self = static_cast<::vsm::detail::self_traits<decltype(this)>::template type<T>>(this)

} // namespace vsm
