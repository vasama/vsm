#pragma once

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
public:
	template<typename T>
	using internal_class = T::internal_class;

	template<typename T>
	using private_class = T::private_class;
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
	auto* const self = static_cast<::vsm::detail::_self<decltype(this)>::template type<T>>(this)

} // namespace vsm
