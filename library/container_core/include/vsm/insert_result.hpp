#pragma once

namespace vsm {

template<typename Iterator>
struct insert_result
{
	/// @brief Iterator to the matching element in the container.
	Iterator iterator;

	/// @brief True if a new element was inserted.
	bool inserted;
};

} // namespace vsm
