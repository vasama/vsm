#include <vsm/detail/swiss_table.hpp>
#include <vsm/detail/swiss_table.ipp>

using namespace vsm;
using namespace vsm::detail::swiss_table;
namespace swiss_table = detail::swiss_table;

void swiss_table::memswap(void* const lhs, void* const rhs, size_t const size)
{
	static constexpr size_t block_size = 64;

	std::byte* const l = static_cast<std::byte*>(lhs);
	std::byte* const r = static_cast<std::byte*>(rhs);

	std::byte buffer[block_size];
	size_t offset = 0;

	for (size_t const block_end = size - size % block_size; offset < block_end; offset += block_size)
	{
		memcpy(buffer, l + offset, block_size);
		memcpy(l + offset, r + offset, block_size);
		memcpy(r + offset, buffer, block_size);
	}

	if (size_t const remaining = size - offset)
	{
		memcpy(buffer, l + offset, remaining);
		memcpy(l + offset, r + offset, remaining);
		memcpy(r + offset, buffer, remaining);
	}
}

ctrl const swiss_table::empty_group[group_size] =
{
	ctrl_end,   ctrl_empty, ctrl_empty, ctrl_empty,
	ctrl_empty, ctrl_empty, ctrl_empty, ctrl_empty,
	ctrl_empty, ctrl_empty, ctrl_empty, ctrl_empty,
	ctrl_empty, ctrl_empty, ctrl_empty, ctrl_empty,
};


size_t swiss_table::find_free_slot(ctrl const* const ctrls, size_t const capacity, size_t const hash)
{
	probe probe(get_h1(hash), capacity);

	while (true)
	{
		group group(ctrls + probe.offset);

		if (uint32_t const mask = group.match_free())
		{
			return probe.get_offset(std::countr_zero(mask));
		}

		vsm_assert(probe.index < capacity);

		probe.next();
	}
}

void* swiss_table::insert_slot(core& table, size_t const element_size, size_t const hash, size_t const slot_index)
{
	++table.size;
	size_t const capacity = table.capacity;
	ctrl* const ctrls = get_ctrls(table.slots, element_size, capacity);
	set_ctrl(ctrls, capacity, slot_index, get_h2(hash));
	return table.slots + slot_index * element_size;
}

void* swiss_table::insert_impl(core& table, size_t const element_size, size_t const hash, resize_callback* const callback)
{
	ctrl* ctrls = get_ctrls(table.slots, element_size, table.capacity);
	size_t slot_index = find_free_slot(ctrls, table.capacity, hash);
	ctrl const slot_ctrl = ctrls[slot_index];

	if (vsm_unlikely(table.free == 0 && slot_ctrl != ctrl_tomb))
	{
		if (!callback(table, element_size, hash))
		{
			return { nullptr };
		}
		
		ctrls = get_ctrls(table.slots, element_size, table.capacity);
		slot_index = find_free_slot(ctrls, table.capacity, hash);
		vsm_assert(ctrls[slot_index] == slot_ctrl);
	}

	table.free -= static_cast<size_t>(slot_ctrl != ctrl_tomb);
	return insert_slot(table, element_size, hash, slot_index);
}

void swiss_table::remove_slot(core& table, size_t const element_size, size_t const slot_index)
{
	size_t const capacity = table.capacity;
	ctrl* const ctrls = get_ctrls(table.slots, element_size, capacity);

	size_t const left_index = slot_index - group_size & capacity;

	uint32_t const left_mask = group(ctrls + left_index).match_empty();
	uint32_t const slot_mask = group(ctrls + slot_index).match_empty();

	bool const reuse_slot = left_mask != 0 && slot_mask != 0 &&
		(std::countr_zero(left_mask) + std::countl_zero(slot_mask)) < group_size;

	set_ctrl(ctrls, capacity, slot_index, reuse_slot ? ctrl_empty : ctrl_tomb);

	table.free += static_cast<size_t>(reuse_slot);
	table.size -= 1;
}

static void convert_special_to_empty_and_full_to_tomb(ctrl* const group)
{
	__m128i const ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));

	__m128i const msb1 = _mm_set1_epi8(0x80);
	__m128i const lsb0 = _mm_set1_epi8(0x7F);

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
