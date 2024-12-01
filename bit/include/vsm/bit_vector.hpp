#pragma once

#include <vsm/bit_ptr.hpp>
#include <vsm/vector.hpp>

namespace vsm {

template<detail::_bit_word Word>
[[nodiscard]] constexpr size_t bit_word_size(bit_size_t const bit_size)
{
	return static_cast<size_t>((bit_size + CHAR_BIT - 1) / CHAR_BIT);
}

template<detail::_bit_word Word, size_t Capacity, allocator Allocator = default_allocator>
class small_bit_vector
{
	small_vector<Word, bit_word_size<Capacity>, Allocator> m_vector;

public:
	using value_type                    = bool;
	using allocator_type                = Allocator;
	using size_type                     = bit_size_t;
	using difference_type               = bit_ptrdiff_t;
	using reference                     = bit_ref<Word>;
	using const_reference               = bit_ref<Word const>;
	using pointer                       = bit_ptr<Word>;
	using const_pointer                 = bit_ptr<Word const>;
	using iterator                      = pointer;
	using const_iterator                = const_pointer;
	using reverse_iterator              = std::reverse_iterator<iterator>;
	using const_reverse_iterator        = std::reverse_iterator<const_iterator>;

	[[nodiscard]] reference at(size_type const index)
	{
		
	}

private:
};

template<detail::_bit_word Word, allocator Allocator = default_allocator>
using bit_vector = small_bit_vector<Word, 0, Allocator>;

} // namespace vsm
