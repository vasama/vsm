#pragma once

namespace vsm {

//TODO: Get rid of this
template<typename T>
struct insert_result
{
	/// @brief Matching element in the container.
	T* element;

	/// @brief True if a new element was inserted.
	bool inserted;
};

template<typename Iterator>
struct insert_result2
{
	/// @brief Iterator to the matching element in the container.
	Iterator iterator;

	/// @brief True if a new element was inserted.
	bool inserted;
};

} // namespace vsm
