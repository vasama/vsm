#include <vsm/detail/swiss_table.hpp>
#include <vsm/detail/swiss_table.ipp>

using namespace vsm;

namespace swiss_table = detail::swiss_table;
using namespace swiss_table;

ctrl const swiss_table::empty_group[group_size] =
{
	ctrl_end,   ctrl_empty, ctrl_empty, ctrl_empty,
	ctrl_empty, ctrl_empty, ctrl_empty, ctrl_empty,
	ctrl_empty, ctrl_empty, ctrl_empty, ctrl_empty,
	ctrl_empty, ctrl_empty, ctrl_empty, ctrl_empty,
};


size_t swiss_table::find_free_slot(
	ctrl const* const ctrls,
	size_t const capacity,
	size_t const hash)
{
	probe probe(get_h1(hash), capacity);

	while (true)
	{
		group const group(ctrls + probe.offset);

		if (uint32_t const mask = group.match_free())
		{
			return probe.get_offset(static_cast<size_t>(std::countr_zero(mask)));
		}

		vsm_assert(probe.index < capacity);

		probe.next();
	}
}

static void* insert_slot(
	_table& table,
	size_t const element_size,
	size_t const hash,
	size_t const slot_index)
{
	vsm_assert(slot_index < table.capacity);
	++table.size;
	size_t const capacity = table.capacity;
	ctrl* const ctrls = get_ctrls(table.slots, element_size, capacity);
	set_ctrl(ctrls, capacity, slot_index, get_h2(hash));
	return table.slots + slot_index * element_size;
}

void* swiss_table::insert2(
	_table& table,
	size_t const element_size,
	size_t const hash,
	resize_callback_type* const callback)
{
	ctrl* ctrls = get_ctrls(table.slots, element_size, table.capacity);
	size_t slot_index = find_free_slot(ctrls, table.capacity, hash);
	ctrl const slot_ctrl = ctrls[slot_index];

	if (vsm_unlikely(table.free == 0 && slot_ctrl != ctrl_tomb))
	{
		if (!callback(table, element_size, hash))
		{
			return nullptr;
		}

		ctrls = get_ctrls(table.slots, element_size, table.capacity);
		slot_index = find_free_slot(ctrls, table.capacity, hash);
		vsm_assert(slot_ctrl == ctrl_end || slot_ctrl == ctrls[slot_index]);
	}

	vsm_assert(table.free != 0 || slot_ctrl == ctrl_tomb);
	table.free -= static_cast<size_t>(slot_ctrl != ctrl_tomb);
	return insert_slot(table, element_size, hash, slot_index);
}

void swiss_table::erase_slot(_table& table, size_t const element_size, size_t const slot_index)
{
	size_t const capacity = table.capacity;
	ctrl* const ctrls = get_ctrls(table.slots, element_size, capacity);

	size_t const left_index = slot_index - group_size & capacity;

	uint32_t const left_mask = group(ctrls + left_index).match_empty();
	uint32_t const slot_mask = group(ctrls + slot_index).match_empty();

	size_t const r_zero_left = static_cast<size_t>(std::countr_zero(left_mask));
	size_t const l_zero_slot = static_cast<size_t>(std::countl_zero(slot_mask));

	bool const reuse_slot =
		left_mask != 0 &&
		slot_mask != 0 &&
		(r_zero_left + l_zero_slot) < group_size;

	set_ctrl(ctrls, capacity, slot_index, reuse_slot ? ctrl_empty : ctrl_tomb);

	table.free += static_cast<size_t>(reuse_slot);
	table.size -= 1;
}

static void convert_special_to_empty_and_full_to_tomb(ctrl* const group)
{
	__m128i const ctrl = _mm_loadu_si128(reinterpret_cast<__m128i const*>(group));

	__m128i const msb1 = _mm_set1_epi8(static_cast<char>(static_cast<uint8_t>(0x80)));
	__m128i const lsb0 = _mm_set1_epi8(static_cast<char>(static_cast<uint8_t>(0x7E)));

	__m128i const result = _mm_or_si128(_mm_shuffle_epi8(lsb0, ctrl), msb1);

	_mm_storeu_si128(reinterpret_cast<__m128i*>(group), result);
}

void swiss_table::convert_tomb_to_empty_and_full_to_tomb(ctrl* const ctrls, size_t const capacity)
{
	vsm_assert(ctrls[capacity] == ctrl_end);

	ctrl* const end = ctrls + capacity + 1;
	for (ctrl* group = ctrls; group != end; group += group_size)
	{
		convert_special_to_empty_and_full_to_tomb(group);
	}

	memcpy(end, ctrls, group_size);
	ctrls[capacity] = ctrl_end;
}
