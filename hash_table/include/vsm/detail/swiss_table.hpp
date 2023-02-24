#pragma once

#include <vsm/detail/hash_table.hpp>
#include <vsm/insert_result.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/memory.hpp>
#include <vsm/math.hpp>
#include <vsm/platform.h>
#include <vsm/standard.hpp>
#include <vsm/type_traits.hpp>

#include <bit>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace vsm::detail::swiss_table {
#include <vsm/detail/swiss_table.ipp>

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
		return static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(h2), m_ctrl)));
	}

	uint32_t match_empty() const
	{
		return static_cast<uint32_t>(_mm_movemask_epi8(_mm_sign_epi8(m_ctrl, m_ctrl)));
	}

	uint32_t match_free() const
	{
		return static_cast<uint32_t>(
			_mm_movemask_epi8(_mm_cmpgt_epi8(_mm_set1_epi8(ctrl_end), m_ctrl)));
	}

	size_t count_leading_free() const
	{
		__m128i const end = _mm_set1_epi8(ctrl_end);
		uint32_t const mask = static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(end, m_ctrl)));

		return mask == 0
			? group_size
			: static_cast<size_t>(std::countr_one(mask));
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

	size_t get_offset(size_t const additional_offset) const
	{
		return (offset + additional_offset) & mask;
	}

	void next()
	{
		index += group_size;
		offset += index;
		offset &= mask;
	}
};


struct _table
{
	std::byte* slots;
	size_t size;
	size_t free;
	size_t capacity;
};

template<typename P>
using _table_policies = hash_table_policies_layout<_table, P>;

template<typename P, typename A>
using _table_allocator = hash_table_allocator_layout<_table_policies<P>, A>;

template<typename T, typename P, typename A, size_t C>
using _table_storage = hash_table_storage_layout<_table_allocator<P, A>, T, C, C + group_size + 1>;


typedef bool resize_callback_type(_table& table, size_t element_size, size_t hash);

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

template<typename T, typename P, typename A, size_t C>
constexpr std::byte* get_storage(_table_storage<T, P, A, C>& table)
{
	if constexpr (C != 0)
	{
		return table.storage;
	}
	else
	{
		return reinterpret_cast<std::byte*>(const_cast<ctrl*>(empty_group));
	}
}

inline void set_ctrl(
	ctrl* const ctrls,
	size_t const capacity,
	size_t const slot_index,
	ctrl const h2)
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

template<typename A>
std::byte* allocate_storage(A& allocator, size_t const element_size, size_t& capacity)
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

template<typename T, typename P, typename A, size_t C>
void construct(_table_storage<T, P, A, C>& table)
{
	static_assert(is_power_of_two(C + 1));

	table.slots = get_storage(table);
	table.size = 0;
	table.free = C;
	table.capacity = C;

	if constexpr (C != 0)
	{
		init_ctrl(get_ctrls(table.slots, sizeof(T), C), C);
	}
}

template<typename T, typename P, typename A>
void destroy(
	_table_allocator<P, A>& table,
	size_t const element_size,
	void const* const null_storage)
{
	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		if (table.size != 0)
		{
			size_t const capacity = table.capacity;

			std::byte* slots_pos = table.slots;
			ctrl const* ctrls_pos = get_ctrls(slots_pos, element_size, capacity);
			ctrl const* ctrls_end = ctrls_pos + capacity;

			for (; ctrls_pos != ctrls_end; ++ctrls_pos, slots_pos += element_size)
			{
				if (*ctrls_pos >= 0)
				{
					reinterpret_cast<T*>(slots_pos)->~T();
				}
			}
		}
	}

	if (table.slots != null_storage)
	{
		table.allocator.deallocate(allocation(
			table.slots,
			get_buffer_size(table.capacity, element_size)));
	}
}

inline size_t get_probe_index(size_t const slot_index, size_t const capacity, size_t const hash)
{
	return (slot_index - get_h1(hash) & capacity) / group_size;
}

size_t find_free_slot(ctrl const* ctrls, size_t capacity, size_t hash);
void* insert2(_table& table, size_t element_size, size_t hash, resize_callback_type* callback);
void erase_slot(_table& table, size_t element_size, size_t slot_index);
void convert_tomb_to_empty_and_full_to_tomb(ctrl* ctrls, size_t capacity);


#pragma push_macro("get_key")
#define get_key(slot) (*std::launder(reinterpret_cast<K const*>(slot)))

template<typename K, typename P>
void reuse_tombs(_table_policies<P>& table, size_t const element_size)
{
	P const& p = table.policies;

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
		size_t const hash = p.hasher(normalize_key(p.key_selector(get_key(old_slot))));

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

	table.free = get_max_size(capacity) - table.size;
}

template<typename K, typename P>
void rehash(_table_policies<P> const& old_table, size_t const element_size, _table& new_table)
{
	P const& p = old_table.policies;

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
			size_t const hash = p.hasher(normalize_key(p.key_selector(get_key(old_slot))));

			size_t const new_slot_index = find_free_slot(new_ctrls, new_capacity, hash);
			set_ctrl(new_ctrls, new_capacity, new_slot_index, get_h2(hash));

			void* const new_slot = new_slots + new_slot_index * element_size;
			memcpy(new_slot, old_slot, element_size);
		}
	}
}

template<typename K, typename P, typename A>
void resize(
	_table_allocator<P, A>& table,
	size_t const element_size,
	size_t new_capacity,
	void const* const null_storage)
{
	vsm_assert(is_power_of_two(new_capacity + 1));
	vsm_assert(get_max_size(new_capacity) >= table.size);

	std::byte* const new_slots = allocate_storage(table.allocator, element_size, new_capacity);
	ctrl* const new_ctrls = get_ctrls(new_slots, element_size, new_capacity);

	init_ctrl(new_ctrls, new_capacity);
	size_t const size = table.size;

	_table new_table =
	{
		.slots = new_slots,
		.size = size,
		.free = get_max_size(new_capacity) - size,
		.capacity = new_capacity,
	};

	if (size != 0)
	{
		rehash<K>(table, element_size, new_table);
	}

	if (table.slots != null_storage)
	{
		vsm_assert(table.slots != nullptr);

		table.allocator.deallocate(allocation(
			table.slots,
			get_buffer_size(table.capacity, element_size)));
	}

	static_cast<_table&>(table) = new_table;
}

template<typename K, typename P, typename A>
void reserve(
	_table_allocator<P, A>& table,
	size_t const element_size,
	size_t const min_capacity,
	void const* const null_storage)
{
	if (min_capacity != 0)
	{
		resize<K>(table, element_size, std::bit_ceil(min_capacity + 1) - 1, null_storage);
	}
}

template<typename K, typename P, typename A>
bool resize_callback2(
	_table_allocator<P, A>& table,
	size_t const element_size,
	size_t const hash,
	void const* const null_storage)
{
	size_t const capacity = table.capacity;

	if (capacity == 0)
	{
		resize<K>(table, element_size, group_size - 1, null_storage);
	}
	else if (table.size <= get_max_size(capacity) / 2)
	{
		reuse_tombs<K>(table, element_size);
	}
	else
	{
		resize<K>(table, element_size, capacity * 2 + 1, null_storage);
	}

	return true;
}

template<typename T, typename K, typename P, typename A, size_t C>
bool resize_callback(_table& table, size_t const element_size, size_t const hash)
{
	auto& full_table = static_cast<_table_storage<T, P, A, C>&>(table);
	return resize_callback2<K>(full_table, element_size, hash, get_storage(full_table));
}

template<typename K, typename UserK, typename P>
size_t find2(
	_table_policies<P> const& table,
	size_t const element_size,
	size_t const hash,
	input_t<UserK> key)
{
	P const& p = table.policies;

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
			size_t const slot_index = probe.get_offset(static_cast<size_t>(std::countr_zero(mask)));
			void* const slot = slots + slot_index * element_size;

			if (vsm_likely(p.comparator(key, p.key_selector(get_key(slot)))))
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

template<typename K, typename UserK, size_t element_size, typename P>
void* find(_table_policies<P> const& table, size_t const hash, input_t<UserK> key)
{
	size_t const slot_index = find2<K, UserK>(table, element_size, hash, key);

	if (slot_index != static_cast<size_t>(-1))
	{
		return table.slots + slot_index * element_size;
	}

	return nullptr;
}

template<typename K, typename UserK, size_t element_size, typename P>
insert_result2<void*> insert(
	_table_policies<P>& table,
	size_t const hash,
	input_t<UserK> key,
	resize_callback_type* const callback)
{
	size_t const slot_index = find2<K, UserK>(table, element_size, hash, key);

	if (slot_index != static_cast<size_t>(-1))
	{
		return { table.slots + slot_index * element_size, false };
	}

	return { insert2(table, element_size, hash, callback), true };
}

template<typename K, typename UserK, size_t element_size, typename P>
void* erase(_table_policies<P>& table, size_t const hash, input_t<UserK> key)
{
	size_t const slot_index = find2<K, UserK>(table, element_size, hash, key);

	if (slot_index != static_cast<size_t>(-1))
	{
		erase_slot(table, element_size, slot_index);
		return table.slots + slot_index * element_size;
	}

	return nullptr;
}

#pragma pop_macro("get_key")


struct sentinel {};

template<typename T>
class iterator_1;

template<typename T>
class iterator_n;

template<typename T, typename Base>
class iterator_base : Base
{
public:
	using value_type = T;
	using difference_type = ptrdiff_t;

	iterator_base() = default;

	iterator_base(sentinel)
	{
		Base::m_slot = nullptr;
	}

	explicit iterator_base(Base const& base)
		: Base(base)
	{
	}

	[[nodiscard]] T& operator*() const
	{
		return *std::launder(reinterpret_cast<T*>(Base::m_slot));
	}

	[[nodiscard]] T* operator->() const
	{
		return std::launder(reinterpret_cast<T*>(Base::m_slot));
	}

	[[nodiscard]] friend bool operator==(iterator_base const& lhs, sentinel)
	{
		return lhs.Base::m_slot == nullptr;
	}

	[[nodiscard]] friend bool operator==(iterator_base const& lhs, iterator_base const& rhs)
	{
		return lhs.Base::m_slot == rhs.Base::m_slot;
	}

private:
	template<typename>
	friend class iterator_1;

	template<typename>
	friend class iterator_n;
};

struct iterator_n_base
{
	std::byte* m_slot;
	ctrl const* m_ctrl;

	template<size_t element_size>
	void skip_free_slots()
	{
		while (*m_ctrl < ctrl_end)
		{
			auto const shift = group(m_ctrl).count_leading_free();

			m_slot += shift * element_size;
			m_ctrl += shift;
		}

		if (*m_ctrl == ctrl_end)
		{
			m_slot = nullptr;
		}
	}

	template<size_t element_size>
	void advance()
	{
		m_slot += element_size;
		m_ctrl += 1;

		skip_free_slots<element_size>();
	}

	template<size_t element_size>
	static iterator_n_base begin(std::byte* const slots, size_t const capacity)
	{
		iterator_n_base it =
		{
			slots,
			get_ctrls(slots, element_size, capacity),
		};
		it.skip_free_slots<element_size>();
		return it;
	}
};

template<typename T>
class iterator_n : public iterator_base<T, iterator_n_base>
{
	using base = iterator_base<T, iterator_n_base>;

public:
	using base::base;

	template<cv_convertible_to<T> U>
	iterator_n(iterator_n<U> const& iterator)
		: base(static_cast<iterator_n_base const&>(iterator))
	{
	}

	iterator_n& operator++() &
	{
		iterator_n_base::advance<sizeof(T)>();
		return *this;
	}

	[[nodiscard]] iterator_n operator++(int) &
	{
		auto it = *this;
		iterator_n_base::advance<sizeof(T)>();
		return it;
	}
};

struct iterator_1_base
{
	void* m_slot;
};

template<typename T>
class iterator_1 : public iterator_base<T, iterator_1_base>
{
	using base = iterator_base<T, iterator_1_base>;

public:
	using base::base;

	template<cv_convertible_to<T> U>
	iterator_1(iterator_1<U> const& iterator)
		: base(static_cast<iterator_1_base const&>(iterator))
	{
	}

	template<cv_convertible_to<T> U>
	iterator_1(iterator_n<U> const& iterator)
	{
		iterator_1_base::m_slot = iterator.m_slot;
	}

	explicit iterator_1(T* const element)
	{
		iterator_1_base::m_slot = const_cast<void*>(static_cast<void const*>(element));
	}

	iterator_1& operator++() &
	{
		iterator_n_base::m_slot = nullptr;
		return *this;
	}

	[[nodiscard]] iterator_1 operator++(int) &
	{
		auto it = *this;
		iterator_n_base::m_slot = nullptr;
		return it;
	}
};


template<typename T, typename Key, typename Policies, typename Allocator, size_t Capacity>
class table
{
	static constexpr size_t storage_capacity = Capacity == 0
		? 0
		: std::bit_ceil(Capacity + 1) - 1;

	static_assert(Capacity == 0 || storage_capacity + 1 >= alignof(T));

#	pragma push_macro("destroy_type")
#	define destroy_type select_t<std::is_trivially_destructible_v<T>, std::byte, T>

	_table_storage<T, Policies, Allocator, storage_capacity> m;

public:
	static constexpr bool is_ordered = false;

	using element_type                  = T;
	using key_type                      = Key;
	using policies_type                 = Policies;
	using allocator_type                = Allocator;
	using size_type                     = size_t;
	using difference_type               = ptrdiff_t;
	using reference                     = T&;
	using const_reference               = T const&;
	using pointer                       = T*;
	using const_pointer                 = T const*;
	using iterator                      = iterator_n<T>;
	using const_iterator                = iterator_n<T const>;
	using single_iterator               = iterator_1<T>;
	using const_single_iterator         = iterator_1<T const>;
	using insert_result                 = vsm::insert_result2<single_iterator>;


	table()
	{
		construct(m);
	}

	explicit table(any_cvref_of<Allocator> auto&& allocator)
		: m(vsm_forward(allocator))
	{
		construct(m);
	}

	table(table&& other) noexcept;
	table& operator=(table&& other) & noexcept;

	~table()
	{
		destroy<destroy_type>(m, sizeof(T), get_storage(m));
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
		if (get_max_size(m.capacity) < min_capacity)
		{
			swiss_table::reserve<Key>(m, sizeof(T), min_capacity, get_storage(m));
		}
	}

	void shrink_to_fit();


	template<typename K>
	[[nodiscard]] single_iterator find(K const& key)
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		auto const element = static_cast<T*>(swiss_table::find<Key, user_key_type, sizeof(T)>(
			m,
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k));

		return single_iterator(element);
	}

	template<typename K>
	[[nodiscard]] const_single_iterator find(K const& key) const
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		auto const element = static_cast<T*>(swiss_table::find<Key, user_key_type, sizeof(T)>(
			m,
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k));

		return const_single_iterator(element);
	}

	template<typename K>
	[[nodiscard]] insert_result insert(K const& key)
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		auto const r = swiss_table::insert<Key, user_key_type, sizeof(T)>(
			m,
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k,
			swiss_table::resize_callback<T, Key, Policies, Allocator, storage_capacity>);

		return { single_iterator(static_cast<T*>(r.iterator)), r.inserted };
	}


	template<typename K>
	[[nodiscard]] size_t erase(K const& key)
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		T* const object = static_cast<T*>(swiss_table::erase<Key, user_key_type, sizeof(T)>(
			m,
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k));

		if (object == nullptr)
		{
			return 0;
		}

		object->~T();
		return 1;
	}

	void erase(const_single_iterator const iterator)
	{
		iterator->~T();

		erase_slot(
			m,
			sizeof(T),
			std::to_address(iterator) - reinterpret_cast<T*>(m.slots));
	}

	void clear();


	[[nodiscard]] iterator begin()
	{
		return iterator(iterator_n_base::begin<sizeof(T)>(m.slots, m.capacity));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(iterator_n_base::begin<sizeof(T)>(m.slots, m.capacity));
	}

	[[nodiscard]] sentinel end() const
	{
		return {};
	}

#	pragma pop_macro("destroy_type")
};

#include <vsm/detail/swiss_table.ipp>
} // namespace vsm::detail::swiss_table
