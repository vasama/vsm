#include <vsm/detail/swiss_table.hpp>

using namespace vsm;
using namespace vsm::detail;

#if vsm_arch_x86

vsm_no_sanitize_address
void _swiss_table_group_x86::convert_special_to_empty_and_full_to_tomb(
	_swiss_table_ctrl* const group)
{
	__m128i const ctrl = _swiss_table_group_x86::load(group);

	__m128i const msb1 = _mm_set1_epi8(static_cast<char>(static_cast<uint8_t>(0x80)));
	__m128i const lsb0 = _mm_set1_epi8(static_cast<char>(static_cast<uint8_t>(0x7E)));

	__m128i const result = _mm_or_si128(_mm_shuffle_epi8(lsb0, ctrl), msb1);

	_swiss_table_group_x86::store(group, result);
}

#endif


using group_array = std::array<_swiss_table_ctrl, _swiss_table_group_size>;

static consteval group_array make_empty_group() noexcept
{
	group_array group;

	for (_swiss_table_ctrl& ctrl : group)
	{
		ctrl = _swiss_table_ctrl::empty;
	}
	group.front() = _swiss_table_ctrl::end;

	return group;
}
constinit group_array const detail::_swiss_table_empty_group = make_empty_group();


size_t detail::_swiss_table_find_free(
	_swiss_table_ctrl const* const ctrl,
	size_t const capacity,
	size_t const hash)
{
	_swiss_table_probe probe(_swiss_table_hash_1(hash), capacity);

	while (true)
	{
		_swiss_table_group const group(ctrl + probe.offset);

		if (auto const mask = group.match_free())
		{
			return probe.get_offset(mask.countr_zero());
		}

		vsm_assert(probe.index < capacity);
		probe.next();
	}
}

// Converts
// 1. _swiss_table_ctrl::tomb to _swiss_table_ctrl::empty, and
// 2. any value indicating an element to _swiss_table_ctrl::tomb.
void detail::_swiss_table_refresh_1(_swiss_table_ctrl* const ctrl, size_t const capacity)
{
	vsm_assert(_swiss_table_ctrl_get(ctrl, capacity) == _swiss_table_ctrl::end);

	_swiss_table_ctrl* const ctrl_end = ctrl + capacity + 1;

	for (_swiss_table_ctrl* pos = ctrl; pos != ctrl_end; pos += _swiss_table_group_size)
	{
		_swiss_table_group::convert_special_to_empty_and_full_to_tomb(pos);
	}

	// Duplicate the first group after the end:
	vsm_memcpy_no_sanitize_address(ctrl_end, ctrl, _swiss_table_group_size);

	// Mark the slot before the duplicate group:
	_swiss_table_ctrl_set_1(ctrl, capacity, _swiss_table_ctrl::end);
}

void* detail::_swiss_table_insert_1(
	_swiss_table& table,
	size_t const sizeof_t,
	size_t const hash,
	_swiss_table_resize_t* const resize)
{
	size_t capacity = table.m_capacity;

	unsigned char* data = table._get_ptr();
	_swiss_table_ctrl* ctrl = _swiss_table_ctrl_ptr(data, capacity, sizeof_t);

	size_t slot_index = _swiss_table_find_free(ctrl, capacity, hash);
	_swiss_table_ctrl const slot_ctrl = _swiss_table_ctrl_get(ctrl, slot_index);

	if (vsm_unlikely(table.m_free == 0 && slot_ctrl != _swiss_table_ctrl::tomb))
	{
		resize(table);

		// The table cannot be local after it was just resized.
		vsm_assert(!table.m_is_local);

		capacity = table.m_capacity;

		data = table._get_storage_ptr();
		ctrl = _swiss_table_ctrl_ptr(data, capacity, sizeof_t);

		slot_index = _swiss_table_find_free(ctrl, capacity, hash);

		// No need to update slot_ctrl. Either it was end, or the new one must be equal.
		vsm_assert(
			slot_ctrl == _swiss_table_ctrl::end ||
			slot_ctrl == _swiss_table_ctrl_get(ctrl, slot_index));
	}

	// If the selected slot was not a tomb, there must be free space available.
	vsm_assert(table.m_free != 0 || slot_ctrl == _swiss_table_ctrl::tomb);

	table.m_size += 1;
	table.m_free -= static_cast<size_t>(slot_ctrl != _swiss_table_ctrl::tomb);

	_swiss_table_ctrl_set_2(ctrl, capacity, slot_index, _swiss_table_hash_2(hash));

	unsigned char* const slot = data + slot_index * sizeof_t;

#if vsm_has_address_sanitizer
	__asan_unpoison_memory_region(slot, sizeof_t);
#endif

	return slot;
}

void detail::_swiss_table_erase_1(
	_swiss_table& table,
	size_t const sizeof_t,
	size_t const slot_index)
{
	size_t const capacity = table.m_capacity;

	unsigned char* const data = table._get_ptr();
	_swiss_table_ctrl* const ctrl = _swiss_table_ctrl_ptr(data, capacity, sizeof_t);

#if vsm_has_address_sanitizer
	__asan_poison_memory_region(data + slot_index * sizeof_t, sizeof_t);
#endif

	size_t const left_index = (slot_index - _swiss_table_group_size) & capacity;

	auto const left_group_mask = _swiss_table_group(ctrl + left_index).match_empty();
	auto const slot_group_mask = _swiss_table_group(ctrl + slot_index).match_empty();

	size_t const left_group_r_zero = left_group_mask.countr_zero();
	size_t const slot_group_l_zero = slot_group_mask.countl_zero();

	// The slot may be reused (marked as empty instead of tomb), if doing so does not create a full
	// empty group that might cause subsequent probing to terminate early.
	bool const may_reuse_slot =
		left_group_mask &&
		slot_group_mask &&
		(left_group_r_zero + slot_group_l_zero) < _swiss_table_group_size;

	_swiss_table_ctrl_set_2(
		ctrl,
		capacity,
		slot_index,
		may_reuse_slot ? _swiss_table_ctrl::empty : _swiss_table_ctrl::tomb);

	table.m_size -= 1;
	table.m_free += static_cast<size_t>(may_reuse_slot);
}
