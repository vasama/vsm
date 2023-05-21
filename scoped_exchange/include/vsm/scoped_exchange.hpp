#pragma once

#include <algorithm>

namespace vsm {

template<typename T>
class scoped_exchange
{
	T& m_object;
	T m_old_value;

public:
	explicit scoped_exchange(T& object, std::convertible_to<T> auto&& new_value)
		: m_object(object)
		, m_old_value(vsm_forward(new_value))
	{
		using std::swap;
		swap(m_object, m_old_value);
	}
	
	scoped_exchange(scoped_exchange const&) = delete;
	scoped_exchange& operator=(scoped_exchange const&) = delete;
	
	~scoped_exchange()
	{
		using std::swap;
		swap(m_object, m_old_value);
	}
};

template<typename T>
scoped_exchange(T&, auto&&...) -> scoped_exchange<T>;

} // namespace vsm
