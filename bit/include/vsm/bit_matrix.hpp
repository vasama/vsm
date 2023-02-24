#pragma once

#include <vsm/assert.h>
#include <vsm/bit_ptr.hpp>

#include <mdspan>

namespace vsm {

template<
	std::unsigned_integral Word,
	typename Extents,
	typename LayoutPolicy = std::layout_right,
	typename AccessorPolicy = std::default_accessor<Word>>
class bit_mdspan
{
public:
	using word_type                     = Word;

	using extents_type                  = Extents;
	using layout_type                   = LayoutPolicy;
	using accessor_type                 = AccessorPolicy;
	using mapping_type                  = layout_type::template mapping<extents_type>;
	using element_type                  = copy_cv_t<Word, bool>;
	using value_type                    = remove_cv_t<element_type>;
	using index_type                    = extents_type::index_type;
	using size_type                     = extents_type::size_type;
	using data_handle_type              = extents_type::data_handle_type;
	//TODO: Wrap in bit_ref
	using reference                     = extents_type::reference;


	[[nodiscard]] static constexpr rank_type rank() noexcept
	{
		return extents_type::rank();
	}

	[[nodiscard]] static constexpr rank_type rank_dynamic() noexcept
	{
		return extents_type::rank_dynamic();
	}

	[[nodiscard]] static constexpr size_t static_extent(rank_type const index) noexcept
	{
		return extents_type::static_extent(index);
	}

	[[nodiscard]] static constexpr index_type extent(rank_type const index) const noexcept
	{
		return this->m_map.extents().extent(index);
	}


	bit_mdspan() = default;


	[[nodiscard]] constexpr size_type size() const noexcept
	{
		
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		extents_type const& extents = this->m_map.extents();
		for (rank_type index = 0; index < extents_type::rank(); ++index)
		{
			if (extents.extent(index) == 0)
			{
				return true;
			}
		}
		return false;
	}

	[[nodiscard]] constexpr extents_type const& extents() const noexcept
	{
		return this->m_map.extents();
	}

	[[nodiscard]] constexpr data_handle_type const& data_handle() const noexcept
	{
		return this->m_ptr;
	}

	[[nodiscard]] constexpr mapping_type const& mapping() const noexcept
	{
		return this->m_map;
	}

	[[nodiscard]] constexpr accessor_type const& accessor() const noexcept
	{
		return this->m_accessor;
	}

	[[nodiscard]] static constexpr bool is_always_unique() noexcept
	{
		constexpr bool result = mapping_type::is_always_unique();
		return result;
	}

	[[nodiscard]] static constexpr bool is_always_exhaustive() noexcept
	{
		constexpr bool result = mapping_type::is_always_exhaustive();
		return result;
	}

	[[nodiscard]] static constexpr bool is_always_strided() noexcept
	{
		constexpr bool result = mapping_type::is_always_strided();
		return result;
	}

	[[nodiscard]] constexpr bool is_unique() const
		noexcept(noexcept(this->m_map.is_unique()))
	{
		return this->m_map.is_unique();
	}

	[[nodiscard]] constexpr bool is_exhaustive() const
		noexcept(noexcept(this->m_map.is_exhaustive()))
	{
		return this->m_map.is_exhaustive();
	}

	[[nodiscard]] constexpr bool is_strided() const
		noexcept(noexcept(this->m_map.is_strided()))
	{
		return this->m_map.is_strided();
	}

	[[nodiscard]] constexpr index_type stride(rank_type const index) const
		noexcept(noexcept(this->m_map.stride(index)))
	{
		return this->m_map.stride(index);
	}
};

} // namespace vsm
