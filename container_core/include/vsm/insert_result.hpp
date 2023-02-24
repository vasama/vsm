#pragma once

namespace vsm {

template<typename T>
struct insert_result
{
	/// @brief Matching element in the container.
	T* element;

	/// @brief True if a new element was inserted.
	bool inserted;
};

} // namespace vsm
