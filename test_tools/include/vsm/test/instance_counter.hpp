#pragma once

#include <type_traits>

namespace vsm::test {
namespace detail {

using instance_count_type = std::make_signed_t<size_t>;

template<typename T>
inline instance_count_type g_instance_count;

} // namespace detail

template<typename T>
detail::instance_count_type instance_count()
{
	return detail::g_instance_count<T>;
}

template<typename T>
class instance_counter
{
public:
	friend auto operator<=>(instance_counter const&, instance_counter const&) = default;

protected:
	instance_counter()
	{
		++detail::g_instance_count<T>;
	}

	instance_counter(instance_counter const&)
	{
		++detail::g_instance_count<T>;
	}
	
	instance_counter& operator=(instance_counter const&) &
	{
		return *this;
	}

	~instance_counter()
	{
		--detail::g_instance_count<T>;
	}
};

} // namespace vsm::test
