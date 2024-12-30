#pragma once

namespace vsm {

template<typename T>
[[nodiscard]] T* start_lifetime_as(void* const storage) noexcept
{
	return reinterpret_cast<T*>(storage);
}

template<typename T>
[[nodiscard]] T const* start_lifetime_as(void const* const storage) noexcept
{
	return reinterpret_cast<T const*>(storage);
}

template<typename T>
[[nodiscard]] T volatile* start_lifetime_as(void volatile* const storage) noexcept
{
	return reinterpret_cast<T*>(storage);
}

template<typename T>
[[nodiscard]] T const volatile* start_lifetime_as(void const volatile* const storage) noexcept
{
	return reinterpret_cast<T const*>(storage);
}

template<typename T>
[[nodiscard]] T* start_lifetime_as_array(void* const storage, size_t) noexcept
{
	return reinterpret_cast<T*>(storage);
}

template<typename T>
[[nodiscard]] T const* start_lifetime_as_array(void const* const storage, size_t) noexcept
{
	return reinterpret_cast<T const*>(storage);
}

template<typename T>
[[nodiscard]] T volatile* start_lifetime_as_array(void volatile* const storage, size_t) noexcept
{
	return reinterpret_cast<T*>(storage);
}

template<typename T>
[[nodiscard]] T const volatile* start_lifetime_as_array(void const volatile* const storage, size_t) noexcept
{
	return reinterpret_cast<T const*>(storage);
}

} // namespace vsm
