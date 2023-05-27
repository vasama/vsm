#pragma once

#include <vsm/default_allocator.hpp>
#include <vsm/insert_result.hpp>
#include <vsm/math.hpp>
#include <vsm/platform.h>
#include <vsm/type_traits.hpp>

#include <bit>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace vsm::detail::swiss_table {
#include <vsm/detail/swiss_table.ipp>


void memswap(void* lhs, void* rhs, size_t size);

template<typename T, size_t Budget = 1>
using const_t = select_t<std::is_trivially_copyable_v<T> && sizeof(T) <= Budget * sizeof(uintptr_t), T const, T const&>;

template<typename T>
using mutable_t = select_t<std::is_trivially_copyable_v<T> && std::is_empty_v<T>, T, T&>;


template<typename Base, typename Policies>
struct basic_policies_layout : Base
{
	[[no_unique_address]] Policies policies;
};

template<typename Base, typename Allocator>
struct basic_allocator_layout : Base
{
	[[no_unique_address]] Allocator allocator;
};

template<typename Base, typename T, size_t Capacity, size_t ExtraSize = 0>
struct basic_storage_layout : Base
{
	using Base::Base;

	std::byte mutable storage alignas(T)[Capacity * sizeof(T) + ExtraSize];
};

template<typename Base, typename T, size_t ExtraSize>
struct basic_storage_layout<Base, T, 0, ExtraSize> : Base
{
	using Base::Base;

	static constexpr std::byte* storage = nullptr;
};


enum ctrl : int8_t
{
	ctrl_empty          = static_cast<int8_t>(0x80),
	ctrl_tomb           = static_cast<int8_t>(0xFE),
	ctrl_end            = static_cast<int8_t>(0xFF),
};

inline constexpr size_t group_size = 16;
extern ctrl const empty_group[group_size];


struct group
{
	__m128i m_ctrl;

	explicit group(ctrl const* const ctrl)
		: m_ctrl(_mm_loadu_si128(reinterpret_cast<__m128i const*>(ctrl)))
	{
	}

	uint32_t match(ctrl const h2) const
	{
		return _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(h2), m_ctrl));
	}

	uint32_t match_empty() const
	{
		return _mm_movemask_epi8(_mm_sign_epi8(m_ctrl, m_ctrl));
	}

	uint32_t match_free() const
	{
		return _mm_movemask_epi8(_mm_cmpgt_epi8(_mm_set1_epi8(ctrl_end), m_ctrl));
	}

	size_t count_leading_free() const
	{
		__m128i const end = _mm_set1_epi8(ctrl_end);
		uint32_t const mask = _mm_movemask_epi8(_mm_cmpgt_epi8(end, m_ctrl));
		return mask == 0 ? group_size : std::countr_zero(mask);
	}
};

struct probe
{
	size_t mask;
	size_t offset;
	size_t index;

	explicit probe(size_t const hash, size_t const mask)
		: mask(mask)
		, offset(hash & mask)
		, index(0)
	{
	}

	size_t get_offset(size_t const offset) const
	{
		return (this->offset + offset) & mask;
	}

	void next()
	{
		index += group_size;
		offset += index;
		offset &= mask;
	}
};


struct core
{
	std::byte* slots;
	size_t size;
	size_t free;
	size_t capacity;
};

template<typename Policies>
using policies_layout = basic_policies_layout<core, Policies>;

template<typename Policies, typename Allocator>
using allocator_layout = basic_allocator_layout<policies_layout<Policies>, Allocator>;

template<typename T, typename Policies, typename Allocator, size_t Capacity>
using storage_layout = basic_storage_layout<allocator_layout<Policies, Allocator>, T, Capacity, Capacity + group_size + 1>;


typedef bool resize_callback(core& table, size_t element_size, size_t hash);

constexpr size_t get_buffer_size(size_t const capacity, size_t const element_size)
{
	return capacity + 1 + group_size + capacity * element_size;
}

constexpr size_t get_max_size(size_t const capacity)
{
	// Max load factor is 7/8.
	return capacity - capacity / 8;
}

constexpr size_t double_capacity(size_t const capacity)
{
	return capacity * 2 + 1;
}

inline ctrl* get_ctrls(std::byte* const slots, size_t const element_size, size_t const capacity)
{
	return reinterpret_cast<ctrl*>(slots + capacity * element_size);
}

template<typename T, typename Policies, typename Allocator, size_t Capacity>
constexpr std::byte* get_storage(storage_layout<T, Policies, Allocator, Capacity>& table)
{
	if constexpr (Capacity != 0)
	{
		return table.storage;
	}
	else
	{
		return reinterpret_cast<std::byte*>(const_cast<ctrl*>(empty_group));
	}
}

inline void set_ctrl(ctrl* const ctrls, size_t const capacity, size_t const slot_index, ctrl const h2)
{
	ctrls[slot_index] = h2;
	ctrls[(slot_index - group_size & capacity) + group_size] = h2;
}

inline void init_ctrl(ctrl* const ctrls, size_t const capacity)
{
	size_t const ctrl_size = capacity + 1 + group_size;
	vsm_assume(ctrl_size % group_size == 0);
	memset(ctrls, ctrl_empty, ctrl_size);
	ctrls[capacity] = ctrl_end;
}

template<typename Allocator>
std::byte* allocate_storage(Allocator& allocator, size_t const element_size, size_t& capacity)
{
	auto const new_allocation = allocator.allocate(get_buffer_size(capacity, element_size));

	while (true)
	{
		size_t const new_capacity = double_capacity(capacity);

		if (new_allocation.size < get_buffer_size(new_capacity, element_size))
		{
			break;
		}

		capacity = new_capacity;
	}

	return static_cast<std::byte*>(new_allocation.buffer);
}

template<typename T, typename Policies, typename Allocator, size_t Capacity>
void construct(storage_layout<T, Policies, Allocator, Capacity>& table)
{
	table.slots = get_storage(table);
	table.size = 0;
	table.free = get_max_size(Capacity);
	table.capacity = Capacity;

	if constexpr (Capacity != 0)
	{
		init_ctrl(get_ctrls(table.slots, sizeof(T), Capacity), Capacity);
	}
}

template<typename T, typename Policies, typename Allocator>
void destruct_impl(allocator_layout<Policies, Allocator>& table, size_t const element_size, void const* const default_storage)
{
	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		if (table.size != 0)
		{
			std::byte* const slots = table.slots;
			size_t const capacity = table.capacity;

			ctrl const* const ctrls = get_ctrls(slots, element_size, capacity);
			for (size_t slot_index = 0; slot_index < capacity; ++slot_index)
			{
				if (ctrls[slot_index] >= 0)
				{
					reinterpret_cast<T*>(slots + slot_index * element_size)->~T();
				}
			}
		}
	}

	if (table.slots != default_storage)
	{
		table.allocator.deallocate(allocation(table.slots, get_buffer_size(table.capacity, element_size)));
	}
}

inline size_t get_probe_index(size_t const slot_index, size_t const capacity, size_t const hash)
{
	return (slot_index - get_h1(hash) & capacity) / group_size;
}

size_t find_free_slot(ctrl const* ctrls, size_t capacity, size_t hash);
void* insert_slot(core& table, size_t element_size, size_t hash, size_t slot_index);
void* insert_impl(core& table, size_t element_size, size_t hash, resize_callback* callback);
void remove_slot(core& table, size_t element_size, size_t slot_index);
void convert_tomb_to_empty_and_full_to_tomb(ctrl* ctrls, size_t capacity);


#pragma push_macro("get_key")
#define get_key(slot) (*reinterpret_cast<Key const*>(slot))

template<typename Key, typename Policies>
void reuse_tombs(policies_layout<Policies>& table, size_t const element_size)
{
	Policies const& p = table.policies;

	std::byte* const slots = table.slots;
	size_t const capacity = table.capacity;

	ctrl* const ctrls = get_ctrls(slots, element_size, capacity);
	convert_tomb_to_empty_and_full_to_tomb(ctrls, capacity);

	for (size_t old_slot_index = 0; old_slot_index < capacity; ++old_slot_index)
	{
		if (ctrls[old_slot_index] != ctrl_tomb)
		{
			continue;
		}

		void* const old_slot = slots + old_slot_index * element_size;
		size_t const hash = p.hash(p.select(get_key(old_slot)));

		size_t const new_slot_index = find_free_slot(ctrls, capacity, hash);

		size_t const old_probe_index = get_probe_index(old_slot_index, capacity, hash);
		size_t const new_probe_index = get_probe_index(new_slot_index, capacity, hash);

		if (vsm_likely(new_probe_index == old_probe_index))
		{
			set_ctrl(ctrls, capacity, old_slot_index, get_h2(hash));
			continue;
		}

		void* const new_slot = slots + new_slot_index * element_size;

		if (ctrls[new_slot_index] == ctrl_empty)
		{
			set_ctrl(ctrls, capacity, new_slot_index, get_h2(hash));
			memcpy(new_slot, old_slot, element_size);
			set_ctrl(ctrls, capacity, old_slot_index, ctrl_empty);
		}
		else
		{
			vsm_assert(ctrls[new_slot_index] == ctrl_tomb);
			set_ctrl(ctrls, capacity, new_slot_index, get_h2(hash));
			memswap(new_slot, old_slot, element_size);
			--old_slot_index;
		}
	}
}

template<typename Key, typename Policies>
void rehash(policies_layout<Policies> const& old_table, size_t const element_size, core& new_table)
{
	Policies const& p = old_table.policies;

	std::byte* const old_slots = old_table.slots;
	size_t const old_capacity = old_table.capacity;

	std::byte* const new_slots = new_table.slots;
	size_t const new_capacity = new_table.capacity;

	ctrl* const old_ctrls = get_ctrls(old_slots, element_size, old_capacity);
	ctrl* const new_ctrls = get_ctrls(new_slots, element_size, new_capacity);

	for (size_t old_slot_index = 0; old_slot_index < old_capacity; ++old_slot_index)
	{
		if (old_ctrls[old_slot_index] >= 0)
		{
			void* const old_slot = old_slots + old_slot_index * element_size;
			size_t const hash = p.hash(p.select(get_key(old_slot)));

			size_t const new_slot_index = find_free_slot(new_ctrls, new_capacity, hash);
			set_ctrl(new_ctrls, new_capacity, new_slot_index, get_h2(hash));

			void* const new_slot = new_slots + new_slot_index * element_size;
			memcpy(new_slot, old_slot, element_size);
		}
	}
}

template<typename Key, typename Policies, typename Allocator>
void resize(allocator_layout<Policies, Allocator>& table, size_t const element_size, size_t new_capacity, void const* const null_storage)
{
	vsm_assert(is_power_of_two(new_capacity + 1));
	vsm_assert(get_max_size(new_capacity) >= table.size);

	std::byte* const new_slots = allocate_storage(table.allocator, element_size, new_capacity);
	ctrl* const new_ctrls = get_ctrls(new_slots, element_size, new_capacity);

	init_ctrl(new_ctrls, new_capacity);
	size_t const size = table.size;

	core new_table =
	{
		.slots = new_slots,
		.size = size,
		.free = get_max_size(new_capacity) - size,
		.capacity = new_capacity,
	};

	if (size != 0)
	{
		rehash<Key>(table, element_size, new_table);
	}

	if (table.slots != null_storage)
	{
		table.allocator.deallocate(allocation(table.slots, get_buffer_size(table.capacity, element_size)));
	}

	static_cast<core&>(table) = new_table;
}

template<typename Key, typename T, typename Policies, typename Allocator, size_t Capacity>
void reserve(storage_layout<T, Policies, Allocator, Capacity>& table, size_t const element_size, size_t const min_capacity)
{
	if (get_max_size(table.capacity) < min_capacity)
	{
		resize<Key>(table, element_size, min_capacity, get_storage(table));
	}
}

template<typename Key, typename Policies, typename Allocator>
bool resize_callback_2(allocator_layout<Policies, Allocator>& table, size_t const element_size, size_t const hash, void const* const null_storage)
{
	size_t const capacity = table.capacity;

	if (capacity == 0)
	{
		resize<Key>(table, element_size, group_size - 1, null_storage);
	}
	else if (table.size <= get_max_size(capacity) / 2)
	{
		reuse_tombs<Key>(table, element_size);
	}
	else
	{
		resize<Key>(table, element_size, capacity * 2 + 1, null_storage);
	}

	return true;
}

template<typename T, typename Key, typename Policies, typename Allocator, size_t Capacity>
bool resize_callback_1(core& table, size_t const element_size, size_t const hash)
{
	auto& full_table = static_cast<storage_layout<T, Policies, Allocator, Capacity>&>(table);
	return resize_callback_2<Key>(full_table, element_size, hash, get_storage(full_table));
}

template<typename Key, typename UserKey, typename Policies>
size_t find_impl(policies_layout<Policies> const& table, size_t const element_size, size_t const hash, const_t<UserKey> key)
{
	Policies const& p = table.policies;

	size_t const capacity = table.capacity;
	std::byte* const slots = table.slots;
	ctrl* const ctrls = get_ctrls(slots, element_size, capacity);

	ctrl const h2 = get_h2(hash);
	probe probe(get_h1(hash), capacity);

	while (true)
	{
		group const group(ctrls + probe.offset);

		for (uint32_t mask = group.match(h2); mask != 0; mask &= mask - 1)
		{
			size_t const slot_index = probe.get_offset(std::countr_zero(mask));
			void* const slot = slots + slot_index * element_size;

			if (vsm_likely(p.compare(key, p.select(get_key(slot)))))
			{
				return slot_index;
			}
		}

		if (vsm_likely(group.match_empty() != 0))
		{
			return static_cast<size_t>(-1);
		}

		probe.next();
	}
}

template<typename Key, typename UserKey, size_t element_size, typename Policies>
void* find(policies_layout<Policies> const& table, size_t const hash, const_t<UserKey> key)
{
	size_t const slot_index = find_impl<Key, UserKey>(table, element_size, hash, key);

	if (slot_index != static_cast<size_t>(-1))
	{
		return table.slots + slot_index * element_size;
	}
	
	return nullptr;
}

template<typename Key, typename UserKey, size_t element_size, typename Policies>
insert_result<void> insert(policies_layout<Policies>& table, size_t const hash, const_t<UserKey> key, resize_callback* const callback)
{
	size_t const slot_index = find_impl<Key, UserKey>(table, element_size, hash, key);

	if (slot_index != static_cast<size_t>(-1))
	{
		return { table.slots + slot_index * element_size, false };
	}

	return { insert_impl(table, element_size, hash, callback), true };
}

template<typename Key, typename UserKey>
void* remove(core& table, size_t const element_size, size_t const hash, const_t<UserKey> key)
{
	size_t const slot_index = find_impl<Key, UserKey>(table, element_size, hash, key);

	if (slot_index != static_cast<size_t>(-1))
	{
		remove_slot(table, element_size, slot_index);
		return table.slots + slot_index * element_size;
	}

	return nullptr;
}

#pragma pop_macro("get_key")


struct iterator_core
{
	ctrl const* m_ctrl;
	std::byte* m_slot;

	template<size_t element_size>
	void skip_free_slots()
	{
		while (*m_ctrl < ctrl_end)
		{
			auto const shift = group(m_ctrl).count_leading_free();
			
			m_ctrl += shift;
			m_slot += shift * element_size;
		}
	}

	template<size_t element_size>
	void advance()
	{
		m_ctrl += 1;
		m_slot += element_size;
		skip_free_slots<element_size>();
	}

	template<size_t element_size>
	static iterator_core begin(std::byte* const slots, size_t const capacity)
	{
		iterator_core it;
		it.m_ctrl = slots;
		it.m_slot = get_ctrls(slots, element_size, capacity);
		it.skip_free_slots<element_size>();
		return it;
	}

	static iterator_core end(std::byte* const slots, size_t const capacity)
	{
		iterator_core it;
		it.m_slot = slots + capacity;
		return it;
	}
};

template<typename T>
class basic_iterator : iterator_core
{
public:
	basic_iterator() = default;

	explicit basic_iterator(iterator_core const iterator)
		: iterator_core(iterator)
	{
	}

	[[nodiscard]] T& operator*() const
	{
		return *reinterpret_cast<T*>(m_slot);
	}

	[[nodiscard]] T* operator->() const
	{
		return reinterpret_cast<T*>(m_slot);
	}

	basic_iterator& operator++() &
	{
		advance<sizeof(T)>();
		return *this;
	}

	[[nodiscard]] basic_iterator& operator++(int) &
	{
		auto result = *this;
		advance<sizeof(T)>();
		return result;
	}

	[[nodiscard]] friend bool operator==(basic_iterator const& lhs, basic_iterator const& rhs)
	{
		return lhs.m_slot == rhs.m_slot;
	}
};


template<typename T, typename Key, typename Policies, typename Allocator, size_t Capacity>
class hash_table
{
	static constexpr bool has_local_storage = Capacity > 0;

	static constexpr size_t storage_capacity = Capacity == 0
		? 0
		: std::bit_ceil(Capacity) - 1;

	static_assert(Capacity == 0 || storage_capacity + 1 >= alignof(T));

#	pragma push_macro("destroy_type")
#	define destroy_type select_t<std::is_trivially_destructible_v<T>, std::byte, T>

	storage_layout<T, Policies, Allocator, Capacity> m;

public:
	using value_type                                        = T;
	using allocator_type                                    = Allocator;
	using size_type                                         = size_t;
	using difference_type                                   = ptrdiff_t;
	using reference                                         = T&;
	using const_reference                                   = T const&;
	using pointer                                           = T*;
	using const_pointer                                     = T const*;
	using iterator                                          = basic_iterator<T>;
	using const_iterator                                    = basic_iterator<T const>;


	hash_table()
	{
		construct(m);
	}

	hash_table(hash_table&& other);
	hash_table& operator=(hash_table&& other) &;

	~hash_table()
	{
		destruct_impl<destroy_type>(m, sizeof(T), get_storage(m));
	}


	[[nodiscard]] bool empty() const
	{
		return m.size == 0;
	}

	[[nodiscard]] size_t size() const
	{
		return m.size;
	}

	[[nodiscard]] size_t capacity() const
	{
		return m.capacity;
	}

	void reserve(size_t const min_capacity)
	{
		swiss_table::reserve(m, min_capacity);
	}

	void shrink_to_fit();


	template<typename K>
	[[nodiscard]] T* find(K const& key)
	{
		if (m.size == 0)
		{
			return nullptr;
		}

		decltype(auto) k = m.policies.select(key);
		using k_type = std::remove_reference_t<decltype(k)>;

		return reinterpret_cast<T*>(swiss_table::find<Key, k_type, sizeof(T)>(
			m,
			m.policies.hash(k),
			k));
	}

	template<typename K>
	[[nodiscard]] T const* find(K const& key) const
	{
		if (m.size == 0)
		{
			return nullptr;
		}

		decltype(auto) k = m.policies.select(key);
		using k_type = std::remove_reference_t<decltype(k)>;

		return reinterpret_cast<T*>(swiss_table::find<Key, k_type, sizeof(T)>(
			m,
			m.policies.hash(k),
			k));
	}

	template<typename K>
	[[nodiscard]] insert_result<T> insert(K const& key)
	{
		decltype(auto) k = m.policies.select(key);
		using k_type = std::remove_reference_t<decltype(k)>;

		policies_layout<Policies>& p = m;
		auto const r = swiss_table::insert<Key, k_type, sizeof(T)>(
			p,
			m.policies.hash(k),
			k,
			&swiss_table::resize_callback_1<T, Key, Policies, Allocator, Capacity>);

		return { reinterpret_cast<T*>(r.element), r.inserted };
	}

	template<typename K>
	[[nodiscard]] T* remove(K const& key)
	{
		if (m.size == 0)
		{
			return nullptr;
		}

		decltype(auto) k = m.policies.select(key);
		using k_type = std::remove_reference_t<decltype(k)>;

		return reinterpret_cast<T*>(swiss_table::remove<Key, k_type, sizeof(T)>(
			m,
			m.policies.hash(k),
			k));
	}


	[[nodiscard]] iterator begin()
	{
		return iterator_core::begin<sizeof(T)>(m.ctrl, m.capacity);
	}

	[[nodiscard]] const_iterator begin() const
	{
		return iterator_core::begin<sizeof(T)>(m.ctrl, m.capacity);
	}

	[[nodiscard]] iterator end()
	{
		return iterator_core::end(m.ctrl, m.capacity);
	}

	[[nodiscard]] const_iterator end() const
	{
		return iterator_core::end(m.ctrl, m.capacity);
	}


#	pragma pop_macro("destroy_type")
};

#include <vsm/detail/swiss_table.ipp>
} // namespace vsm::detail::swiss_table
