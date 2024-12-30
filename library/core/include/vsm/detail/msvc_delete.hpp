#pragma once

#include <vsm/platform.h>

#include <concepts>
#include <new>

namespace vsm::detail {

template<typename T>
class _msvc_delete_arg
{
	T const m_value;

public:
	vsm_always_inline _msvc_delete_arg(T const value)
		: m_value(value)
	{
	}

	template<std::same_as<T> U>
	vsm_always_inline operator U() const
	{
		return m_value;
	}
};

template<bool>
struct _msvc_delete_void
{
	template<typename T>
	using type = void;
};

template<typename T>
void _msvc_delete(T* const p_object)
{
	using void_t = _msvc_delete_void<sizeof(T) != 0>::template type<T>;
	_msvc_delete_arg const p = const_cast<void_t*>(static_cast<void const*>(p_object));
	_msvc_delete_arg const s = sizeof(T);
	_msvc_delete_arg const a = static_cast<std::align_val_t>(alignof(T));

	if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
	{
		if constexpr (requires { ::operator delete(p, s, a); })
		{
			::operator delete(p, s, a);
		}
		else if constexpr (requires { ::operator delete(p, a); })
		{
			::operator delete(p, a);
		}
		else if constexpr (requires { ::operator delete(p, s); })
		{
			::operator delete(p, s);
		}
		else if constexpr (requires { ::operator delete(p); })
		{
			::operator delete(p);
		}
		else
		{
			static_assert(sizeof(p_object) == 0, "No matching operator delete.");
		}
	}
	else
	{
		if constexpr (requires { ::operator delete(p, s); })
		{
			::operator delete(p, s);
		}
		else if constexpr (requires { ::operator delete(p); })
		{
			::operator delete(p);
		}
		else if constexpr (requires { ::operator delete(p, s, a); })
		{
			::operator delete(p, s, a);
		}
		else if constexpr (requires { ::operator delete(p, a); })
		{
			::operator delete(p, a);
		}
		else
		{
			static_assert(sizeof(p_object) == 0, "No matching operator delete.");
		}
	}
}

} // namespace vsm::detail

#define vsm_qualified_delete(p) \
	([]<typename T>(T* q) { q->~T(); ::vsm::detail::_msvc_delete(q); }(p))
