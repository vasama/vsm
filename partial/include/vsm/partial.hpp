#pragma once

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

struct partial
{
	template<typename T>
	using internal_class = typename T::internal_class;

	template<typename T>
	using private_class = typename T::private_class;
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
		friend struct ::vsm::partial \

#define vsm_self(T) \
	auto* const self = static_cast<::vsm::detail::self_traits<decltype(this)>::template type<T>>(this)

} // namespace vsm
