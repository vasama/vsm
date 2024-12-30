#pragma once

#include <vsm/allocator.hpp>
#include <vsm/concepts.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/insert_result.hpp>
#include <vsm/key_value_pair.hpp>
#include <vsm/platform.h>
#include <vsm/relocate.hpp>
#include <vsm/sanitizer/address.h>
#include <vsm/standard.hpp>
#include <vsm/standard/stdexcept.hpp>
#include <vsm/utility.hpp>

#include <array>
#include <bit>
#include <limits>

namespace vsm::detail {

struct _swiss_table_placeholder_t
{
	explicit _swiss_table_placeholder_t() = default;
};

enum class _swiss_table_ctrl : int8_t
{
	empty           = static_cast<int8_t>(0x80),
	tomb            = static_cast<int8_t>(0xFE),
	end             = static_cast<int8_t>(0xFF),
};


template<std::unsigned_integral Integer, Integer Multiplier = 1>
class _swiss_table_bitmask
{
public:
	class iterator
	{
		Integer m_bits;

	public:
		using value_type = size_t;
		using difference_type = ptrdiff_t;

		iterator() = default;

		explicit iterator(Integer const bits)
			: m_bits(bits)
		{
		}

		[[nodiscard]] size_t operator*() const
		{
			return static_cast<size_t>(std::countr_zero(m_bits)) / Multiplier;
		}

		iterator& operator++() &
		{
			m_bits &= m_bits - 1;
			return *this;
		}

		[[nodiscard]] iterator operator++(int) &
		{
			auto result = *this;
			m_bits &= m_bits - 1;
			return result;
		}

		[[nodiscard]] friend bool operator==(iterator const&, iterator const&) = default;
	};

private:
	Integer m_bits;

public:
	explicit _swiss_table_bitmask(Integer const bits)
		: m_bits(bits)
	{
	}


	[[nodiscard]] explicit operator bool() const
	{
		return m_bits != 0;
	}

	[[nodiscard]] size_t countl_zero() const
	{
		return static_cast<size_t>(std::countl_zero(m_bits)) / Multiplier;
	}

	[[nodiscard]] size_t countr_zero() const
	{
		return static_cast<size_t>(std::countr_zero(m_bits)) / Multiplier;
	}

	[[nodiscard]] iterator begin() const
	{
		return iterator(m_bits);
	}

	[[nodiscard]] iterator end() const
	{
		return iterator(0);
	}
};

template<std::unsigned_integral Integer>
class _swiss_table_group_generic
{
	using iterator_type = _swiss_table_bitmask<Integer, CHAR_BIT>;

	static constexpr Integer lsb_bytes = ~static_cast<Integer>(0) / 0xFF;
	static constexpr Integer msb_bytes = lsb_bytes << (CHAR_BIT - 1);
	static constexpr Integer non_msb_bytes = msb_bytes - lsb_bytes;

	Integer m_ctrl;

public:
	static constexpr size_t size = sizeof(Integer);

	explicit _swiss_table_group_generic(_swiss_table_ctrl const* const ctrl)
		: m_ctrl(_swiss_table_group_generic::load(ctrl))
	{
	}

	// Matches hash exactly.
	[[nodiscard]] iterator_type match(_swiss_table_ctrl const hash_2) const
	{
		// See https://graphics.stanford.edu/~seander/bithacks.html##ValueInWord

		static constexpr Integer m = non_msb_bytes;
		Integer const z = m_ctrl ^ (lsb_bytes * static_cast<uint8_t>(hash_2));
		return iterator_type(~((z & m) + m | m | z));
	}

	// Matches _swiss_table_ctrl::empty.
	[[nodiscard]] iterator_type match_empty() const
	{
		return iterator_type(m_ctrl & ~(m_ctrl << 6) & msb_bytes);
	}

	// Matches _swiss_table_ctrl::empty or _swiss_table_ctrl::tomb.
	[[nodiscard]] iterator_type match_free() const
	{
		return iterator_type(m_ctrl & ~(m_ctrl << 7) & msb_bytes);
	}

	// Matches _swiss_table_ctrl::empty, _swiss_table_ctrl::tomb, or _swiss_table_ctrl::end.
	[[nodiscard]] size_t count_leading_free_or_end() const
	{
		size_t const mask = (m_ctrl | ~(m_ctrl >> 7)) & lsb_bytes;
		return static_cast<size_t>(std::countr_zero(mask)) / CHAR_BIT;
	}

	static void convert_special_to_empty_and_full_to_tomb(_swiss_table_ctrl* const group)
	{
		Integer bits = _swiss_table_group_generic::load(group);

		bits = bits & msb_bytes;
		bits = ~bits + (bits >> 7) & ~lsb_bytes;

		_swiss_table_group_generic::store(group, bits);
	}

private:
	vsm_no_sanitize_address
	[[nodiscard]] static vsm_always_inline Integer load(
		_swiss_table_ctrl const* const ctrl) noexcept
	{
		Integer integer;
		vsm_memcpy_no_sanitize_address(&integer, ctrl, sizeof(Integer));
		return integer;
	}

	vsm_no_sanitize_address
	[[nodiscard]] static vsm_always_inline void store(
		_swiss_table_ctrl* const ctrl,
		Integer const integer) noexcept
	{
		vsm_memcpy_no_sanitize_address(ctrl, &integer, sizeof(Integer));
	}
};

#if vsm_arch_x86
class _swiss_table_group_x86
{
	using iterator_type = _swiss_table_bitmask<uint16_t>;

	__m128i m_ctrl;

public:
	static constexpr size_t size = 16;

	explicit _swiss_table_group_x86(_swiss_table_ctrl const* const ctrl)
		: m_ctrl(_swiss_table_group_x86::load(ctrl))
	{
	}

	[[nodiscard]] iterator_type match(_swiss_table_ctrl const hash_2) const
	{
		__m128i const v_hash_2 = _mm_set1_epi8(static_cast<int8_t>(hash_2));
		return iterator_type(static_cast<uint16_t>(
			_mm_movemask_epi8(_mm_cmpeq_epi8(v_hash_2, m_ctrl))));
	}

	[[nodiscard]] iterator_type match_empty() const
	{
		return iterator_type(static_cast<uint16_t>(
			_mm_movemask_epi8(_mm_sign_epi8(m_ctrl, m_ctrl))));
	}

	[[nodiscard]] iterator_type match_free() const
	{
		__m128i const v_end = _mm_set1_epi8(static_cast<int8_t>(_swiss_table_ctrl::end));
		return iterator_type(static_cast<uint16_t>(
			_mm_movemask_epi8(_mm_cmpgt_epi8(v_end, m_ctrl))));
	}

	[[nodiscard]] size_t count_leading_free_or_end() const
	{
		__m128i const v_end = _mm_set1_epi8(static_cast<int8_t>(_swiss_table_ctrl::end));
		uint32_t const mask = static_cast<uint32_t>(
			_mm_movemask_epi8(_mm_cmpgt_epi8(v_end, m_ctrl)));
		return mask == 0 ? size : static_cast<size_t>(std::countr_one(mask));
	}

	static void convert_special_to_empty_and_full_to_tomb(_swiss_table_ctrl* group);

private:
	vsm_no_sanitize_address
	[[nodiscard]] static vsm_always_inline __m128i load(
		_swiss_table_ctrl const* const ctrl) noexcept
	{
		return _mm_loadu_si128(reinterpret_cast<__m128i const*>(ctrl));
	}

	vsm_no_sanitize_address
	[[nodiscard]] static void vsm_always_inline store(
		_swiss_table_ctrl* const ctrl,
		__m128i const v_ctrl) noexcept
	{
		_mm_storeu_si128(reinterpret_cast<__m128i*>(ctrl), v_ctrl);
	}
};
#endif

#if vsm_arch_x86
using _swiss_table_group = _swiss_table_group_x86;
#else
using _swiss_table_group = _swiss_table_group_generic<size_t>;
#endif

inline constexpr size_t _swiss_table_group_size = _swiss_table_group::size;
extern std::array<_swiss_table_ctrl, _swiss_table_group_size> const _swiss_table_empty_group;

struct _swiss_table_probe
{
	size_t mask;
	size_t offset;
	size_t index; // NOLINT(modernize-use-default-member-init)

	explicit _swiss_table_probe(size_t const hash, size_t const mask)
		: mask(mask)
		, offset(hash & mask)
		, index(0)
	{
	}

	[[nodiscard]] size_t get_offset(size_t const additional_offset) const
	{
		return (offset + additional_offset) & mask;
	}

	void next()
	{
		index += _swiss_table_group_size;
		offset += index;
		offset &= mask;
	}
};


inline vsm_always_inline size_t _swiss_table_hash_1(size_t const hash)
{
	return hash >> 7;
}

inline vsm_always_inline _swiss_table_ctrl _swiss_table_hash_2(size_t const hash)
{
	return static_cast<_swiss_table_ctrl>(hash & 0x7F);
}


template<
	typename T,
	typename K,
	typename P,
	typename A>
class _swiss_table_base_impl;

template<typename SwissTableBase, size_t Capacity>
class _swiss_table_impl;


template<size_t Size>
struct _swiss_table_padding
{
	unsigned char m_padding[Size];
};

template<>
struct _swiss_table_padding<0>
{
};

template<typename T, size_t Alignment>
using _swiss_table_padding_before = _swiss_table_padding<Alignment - sizeof(T) % Alignment>;

template<
	typename T1,
	typename T2,
	size_t Alignment = std::max(alignof(T1), alignof(T2))>
using _swiss_table_padding_between = _swiss_table_padding<Alignment - (sizeof(T1) + sizeof(T2)) % Alignment>;

struct _swiss_table_size_and_free
{
	size_t m_size;
	size_t m_free;
};

struct _swiss_table_capacity
{
	size_t m_capacity : sizeof(size_t) * CHAR_BIT - 1;
	size_t m_is_local : 1;
};

struct _swiss_table
	: _swiss_table_size_and_free
	, _swiss_table_capacity
{
	void _set(size_t const capacity, bool const is_local)
	{
		vsm_gcc_diagnostic(push)
		vsm_gcc_diagnostic(ignored "-Wconversion")

		m_capacity = capacity;
		m_is_local = is_local;

		vsm_gcc_diagnostic(pop)
	}

	[[nodiscard]] unsigned char* _get_ptr() const
	{
		return m_is_local ? _get_storage() : _get_storage_ptr();
	}

	[[nodiscard]] unsigned char* _get_storage() const
	{
		return reinterpret_cast<unsigned char*>(const_cast<_swiss_table*>(this) + 1);
	}

	vsm_no_sanitize_address
	[[nodiscard]] unsigned char* _get_storage_ptr() const
	{
		return *reinterpret_cast<unsigned char* const*>(this + 1);
	}

	vsm_no_sanitize_address
	void _set_storage_ptr(unsigned char* const ptr)
	{
		*reinterpret_cast<unsigned char**>(this + 1) = ptr;
	}
};

template<typename P>
struct _swiss_table_policies
{
	vsm_no_unique_address P m_policies;

	_swiss_table_policies() = default;

	explicit _swiss_table_policies(any_cvref_of<P> auto&& policies)
		: m_policies(vsm_forward(policies))
	{
	}
};

template<typename A>
struct _swiss_table_allocator
{
	vsm_no_unique_address A m_allocator;

	_swiss_table_allocator() = default;

	explicit _swiss_table_allocator(any_cvref_of<A> auto&& allocator)
		: m_allocator(vsm_forward(allocator))
	{
	}
};

inline constexpr size_t _swiss_table_storage_size(size_t const capacity, size_t const sizeof_t)
{
	return capacity * sizeof_t + capacity + _swiss_table_group_size + 1;
}

vsm_msvc_warning(push)
vsm_msvc_warning(disable: 4584) // 'T' is already a base-class of 'U'

vsm_gnu_diagnostic(push)
vsm_gnu_diagnostic(ignored "-Winaccessible-base")

template<typename P>
struct vsm_empty_bases _swiss_table_with_policies
	: _swiss_table_policies<P>
	, _swiss_table_padding_between<P, _swiss_table>
	, _swiss_table
{
	using _swiss_table_policies<P>::_swiss_table_policies;
};

template<typename P, typename A>
struct vsm_empty_bases _swiss_table_with_allocator
	: _swiss_table_allocator<A>
	, _swiss_table_padding_between<A, _swiss_table_with_policies<P>>
	, _swiss_table_with_policies<P>
{
	_swiss_table_with_allocator() = default;

	explicit _swiss_table_with_allocator(auto&& policies, _swiss_table_placeholder_t)
		: _swiss_table_with_policies<P>(vsm_forward(policies))
	{
		static_assert(std::is_default_constructible_v<A>);
	}

	explicit _swiss_table_with_allocator(_swiss_table_placeholder_t, auto&& allocator)
		: _swiss_table_allocator<A>(vsm_forward(allocator))
	{
		static_assert(std::is_default_constructible_v<P>);
	}

	explicit _swiss_table_with_allocator(auto&& policies, auto&& allocator)
		: _swiss_table_allocator<A>(vsm_forward(allocator))
		, _swiss_table_with_policies<P>(vsm_forward(policies))
	{
	}
};

template<typename SwissTable, typename T, typename P, typename A, size_t C>
class vsm_empty_bases _swiss_table_storage
	: _swiss_table_padding_before<SwissTable, std::max(alignof(T), alignof(unsigned char*))>
	, public SwissTable
{
	static_assert(std::has_single_bit(C + 1));

	using SwissTable::SwissTable;

	union
	{
		unsigned char* m_storage_ptr;
		alignas(T) unsigned char m_storage[_swiss_table_storage_size(C, sizeof(T))];
	};

	template<typename BaseFacade, size_t Capacity>
	friend class _swiss_table_impl;
};

template<typename SwissTable, typename T, typename P, typename A>
class vsm_empty_bases _swiss_table_storage<SwissTable, T, P, A, 0>
	: _swiss_table_padding_before<SwissTable, alignof(unsigned char*)>
	, public SwissTable
{
	using SwissTable::SwissTable;

	unsigned char* m_storage_ptr;

	template<typename BaseFacade, size_t Capacity>
	friend class _swiss_table_impl;
};

vsm_msvc_warning(pop)
vsm_gnu_diagnostic(pop)

inline constexpr size_t _swiss_table_max_size(size_t const capacity)
{
	if constexpr (_swiss_table_group_size <= 8)
	{
		if (capacity <= 8)
		{
			// TODO: Document why this is neeeded.
			return capacity == 0 ? 0 : capacity - 2;
		}
	}

	// Max load factor is 7/8.
	return capacity - capacity / 8;
}

inline constexpr size_t _swiss_table_min_capacity_for(size_t const min_max_size)
{
	size_t capacity = std::bit_ceil(min_max_size + 1) - 1;

	if (capacity < _swiss_table_group_size)
	{
		capacity = _swiss_table_group_size - 1;
	}

	while (_swiss_table_max_size(capacity) < min_max_size)
	{
		capacity = capacity * 2 + 1;
	}

	return capacity;
}

inline constexpr size_t _swiss_table_ctrl_size(size_t const capacity)
{
	return capacity + _swiss_table_group_size + 1;
}

inline _swiss_table_ctrl* _swiss_table_ctrl_ptr(
	unsigned char* const data,
	size_t const capacity,
	size_t const sizeof_t)
{
	return reinterpret_cast<_swiss_table_ctrl*>(data + capacity * sizeof_t);
}

vsm_no_sanitize_address
inline void _swiss_table_ctrl_init(_swiss_table_ctrl* const ctrl, size_t const capacity)
{
	size_t const ctrl_size = capacity + 1 + _swiss_table_group_size;

	vsm_assume(ctrl_size % _swiss_table_group_size == 0);
	vsm_memset_no_sanitize_address(
		ctrl,
		static_cast<int8_t>(_swiss_table_ctrl::empty),
		ctrl_size);

	ctrl[capacity] = _swiss_table_ctrl::end;
}

vsm_no_sanitize_address
inline vsm_always_inline _swiss_table_ctrl _swiss_table_ctrl_get(
	_swiss_table_ctrl const* const ctrl,
	size_t const index)
{
	return ctrl[index];
}

vsm_no_sanitize_address
inline vsm_always_inline void _swiss_table_ctrl_set_1(
	_swiss_table_ctrl* const ctrl,
	size_t const index,
	_swiss_table_ctrl const value)
{
	ctrl[index] = value;
}

vsm_no_sanitize_address
inline void _swiss_table_ctrl_set_2(
	_swiss_table_ctrl* const ctrl,
	size_t const capacity,
	size_t const slot_index,
	_swiss_table_ctrl const value)
{
	size_t const copy_index =
		((slot_index - _swiss_table_group_size) & capacity) + _swiss_table_group_size;

	ctrl[slot_index] = value;
	ctrl[copy_index] = value;
}

template<size_t SizeofT, typename TK, typename UK, typename P>
std::pair<void*, size_t> _swiss_table_find_1(
	_swiss_table_with_policies<P> const& table,
	size_t const hash,
	input_t<UK> key)
{
	size_t const capacity = table.m_capacity;

	unsigned char* const data = table._get_ptr();
	_swiss_table_ctrl* const ctrl = _swiss_table_ctrl_ptr(data, capacity, SizeofT);

	_swiss_table_ctrl const hash_2 = _swiss_table_hash_2(hash);
	_swiss_table_probe probe(_swiss_table_hash_1(hash), capacity);

	while (true)
	{
		_swiss_table_group const group(ctrl + probe.offset);

		for (size_t const mask_index : group.match(hash_2))
		{
			size_t const slot_index = probe.get_offset(mask_index);
			void* const slot = data + slot_index * SizeofT;

			bool const is_equal = static_cast<P const&>(table.m_policies).comparator(
				key,
				vsm::normalize_key(
					static_cast<P const&>(table.m_policies)
						.key_selector(*reinterpret_cast<TK const*>(slot))));

			if (vsm_likely(is_equal))
			{
				return { slot, slot_index };
			}
		}

		if (vsm_likely(group.match_empty()))
		{
			return { nullptr, static_cast<size_t>(-1) };
		}

		probe.next();
	}
}

template<size_t SizeofT, typename TK, typename UK, typename P>
void* _swiss_table_find(_swiss_table_with_policies<P> const& table, size_t const hash, input_t<UK> key)
{
	return _swiss_table_find_1<SizeofT, TK, UK>(table, hash, key).first;
}

size_t _swiss_table_find_free(_swiss_table_ctrl const* ctrl, size_t capacity, size_t hash);

void _swiss_table_refresh_1(_swiss_table_ctrl* ctrl, size_t capacity);

inline size_t _swiss_table_probe_index(
	size_t const capacity,
	size_t const slot_index,
	size_t const hash)
{
	return ((slot_index - _swiss_table_hash_1(hash)) & capacity) / _swiss_table_group_size;
}

template<typename T, typename K, typename P>
void _swiss_table_refresh(_swiss_table_with_policies<P>& table)
{
	size_t const capacity = table.m_capacity;

	unsigned char* const data = table._get_ptr();
	_swiss_table_ctrl* const ctrl = _swiss_table_ctrl_ptr(data, capacity, sizeof(T));

	_swiss_table_refresh_1(ctrl, capacity);

	for (size_t old_index = 0; old_index < capacity; ++old_index)
	{
		if (_swiss_table_ctrl_get(ctrl, old_index) == _swiss_table_ctrl::tomb)
		{
			continue;
		}

		void* const old_slot = data + old_index * sizeof(T);

		size_t const hash = static_cast<P const&>(table.m_policies)
			.hasher(vsm::normalize_key(
				static_cast<P const&>(table.m_policies)
					.key_selector(*reinterpret_cast<K const*>(old_slot))));

		size_t const new_index = _swiss_table_find_free(ctrl, capacity, hash);

		size_t const old_probe_index = _swiss_table_probe_index(capacity, old_index, hash);
		size_t const new_probe_index = _swiss_table_probe_index(capacity, new_index, hash);

		if (vsm_likely(new_probe_index == old_probe_index))
		{
			_swiss_table_ctrl_set_2(ctrl, capacity, old_index, _swiss_table_hash_2(hash));
			continue;
		}

		void* const new_slot = data + new_index * sizeof(T);

		if (_swiss_table_ctrl_get(ctrl, new_index) == _swiss_table_ctrl::empty)
		{
#if vsm_has_address_sanitizer
			__asan_unpoison_memory_region(new_slot, sizeof(T));
#endif

			_swiss_table_ctrl_set_2(ctrl, capacity, new_index, _swiss_table_hash_2(hash));
			vsm::relocate_at(static_cast<T*>(old_slot), static_cast<T*>(new_slot));
			_swiss_table_ctrl_set_2(ctrl, capacity, old_index, _swiss_table_ctrl::empty);

#if vsm_has_address_sanitizer
			__asan_poison_memory_region(old_slot, sizeof(T));
#endif
		}
		else
		{
			vsm_assert(_swiss_table_ctrl_get(ctrl, new_index) == _swiss_table_ctrl::tomb);
			_swiss_table_ctrl_set_2(ctrl, capacity, new_index, _swiss_table_hash_2(hash));

			// TODO: Use relocating swap
			{
				using std::swap;
				swap(*static_cast<T*>(new_slot), *static_cast<T*>(old_slot));
			}

			--old_index;
		}
	}

	table.m_free = _swiss_table_max_size(capacity) - table.m_size;
}

template<typename T, typename K, typename P>
void _swiss_table_rehash(
	_swiss_table_with_policies<P>& old_table,
	_swiss_table& new_table,
	unsigned char* const new_data)
{
	size_t const old_capacity = old_table.m_capacity;
	size_t const new_capacity = new_table.m_capacity;

	unsigned char* const old_data = old_table._get_ptr();

	_swiss_table_ctrl* const old_ctrl = _swiss_table_ctrl_ptr(old_data, old_capacity, sizeof(T));
	_swiss_table_ctrl* const new_ctrl = _swiss_table_ctrl_ptr(new_data, new_capacity, sizeof(T));

	for (size_t old_slot_index = 0; old_slot_index < old_capacity; ++old_slot_index)
	{
		if (_swiss_table_ctrl_get(old_ctrl, old_slot_index) < static_cast<_swiss_table_ctrl>(0))
		{
			continue;
		}

		unsigned char* const old_slot = old_data + old_slot_index * sizeof(T);

		size_t const hash = static_cast<P const&>(old_table.m_policies)
			.hasher(vsm::normalize_key(
				static_cast<P const&>(old_table.m_policies)
					.key_selector(*reinterpret_cast<K const*>(old_slot))));

		size_t const new_slot_index = _swiss_table_find_free(new_ctrl, new_capacity, hash);
		_swiss_table_ctrl_set_2(new_ctrl, new_capacity, new_slot_index, _swiss_table_hash_2(hash));
		unsigned char* const new_slot = new_data + new_slot_index * sizeof(T);

#if vsm_has_address_sanitizer
		__asan_unpoison_memory_region(new_slot, sizeof(T));
#endif

		vsm::relocate_at(reinterpret_cast<T*>(old_slot), reinterpret_cast<T*>(new_slot));

#if vsm_has_address_sanitizer
		__asan_poison_memory_region(old_slot, sizeof(T));
#endif
	}
}

template<typename A>
unsigned char* _swiss_table_allocate(A const& allocator, size_t const sizeof_t, size_t& capacity)
{
	// TODO: Specify max allocation size.
	auto const new_allocation = vsm::allocate_or_throw(
		allocator,
		_swiss_table_storage_size(capacity, sizeof_t));

	while (true)
	{
		size_t const new_capacity = capacity * 2 + 1;

		if (new_capacity > std::numeric_limits<size_t>::max() / 2)
		{
			break;
		}

		if (new_allocation.size < _swiss_table_storage_size(new_capacity, sizeof_t))
		{
			break;
		}

		capacity = new_capacity;
	}

#if vsm_has_address_sanitizer
	__asan_poison_memory_region(
		new_allocation.storage,
		_swiss_table_storage_size(capacity, sizeof_t));
#endif

	return static_cast<unsigned char*>(new_allocation.storage);
}

template<typename P, typename A>
void _swiss_table_deallocate(_swiss_table_with_allocator<P, A>& table, size_t const sizeof_t)
{
	if (!table.m_is_local)
	{
		unsigned char* const data = table._get_storage_ptr();

		if (data != reinterpret_cast<unsigned char const*>(_swiss_table_empty_group.data()))
		{
			vsm_assert(data != nullptr);

#if vsm_has_address_sanitizer
			__asan_unpoison_memory_region(
				data,
				_swiss_table_storage_size(table.m_capacity, sizeof_t));
#endif

			table.m_allocator.deallocate(vsm::allocation(
				data,
				_swiss_table_storage_size(table.m_capacity, sizeof_t)));
		}
	}
}

template<typename T, typename K, typename P, typename A>
void _swiss_table_resize_1(_swiss_table_with_allocator<P, A>& table, size_t new_capacity)
{
	vsm_assert(std::has_single_bit(new_capacity + 1));
	vsm_assert(_swiss_table_max_size(new_capacity) >= table.m_size);

	if (new_capacity > std::numeric_limits<size_t>::max() / 2)
	{
		vsm_except_throw_or_terminate(std::length_error("swiss table size out of range"));
	}

	unsigned char* const new_data = _swiss_table_allocate(
		table.m_allocator,
		sizeof(T),
		new_capacity);

	_swiss_table_ctrl_init(_swiss_table_ctrl_ptr(new_data, new_capacity, sizeof(T)), new_capacity);

	size_t const size = table.m_size;

	_swiss_table new_table;
	new_table.m_size = size;
	new_table.m_free = _swiss_table_max_size(new_capacity) - size;
	new_table._set(new_capacity, /* is_local: */ false);

	if (size != 0)
	{
		_swiss_table_rehash<T, K, P>(table, new_table, new_data);
	}

	_swiss_table_deallocate(table, sizeof(T));

	static_cast<_swiss_table&>(table) = new_table;
	table._set_storage_ptr(new_data);
}

template<typename T, typename K, typename P, typename A>
void _swiss_table_resize_2(_swiss_table_with_allocator<P, A>& table)
{
	size_t const capacity = table.m_capacity;

	// NOLINTBEGIN(readability-misleading-indentation)

	/**/ if (capacity == 0)
	{
		_swiss_table_resize_1<T, K>(table, _swiss_table_group_size - 1);
	}
	else if (table.m_size <= _swiss_table_max_size(capacity) / 2)
	{
		_swiss_table_refresh<T, K>(table);
	}
	else
	{
		_swiss_table_resize_1<T, K>(table, capacity * 2 + 1);
	}

	// NOLINTEND(readability-misleading-indentation)
}

template<typename T, typename K, typename P, typename A>
void _swiss_table_resize_3(_swiss_table& table)
{
	_swiss_table_resize_2<T, K>(static_cast<_swiss_table_with_allocator<P, A>&>(table));
}

template<typename T, typename K, typename P, typename A>
void _swiss_table_reserve(_swiss_table_with_allocator<P, A>& table, size_t const min_capacity)
{
	vsm_assert(min_capacity > _swiss_table_max_size(table.m_capacity));
	_swiss_table_resize_1<T, K>(table, _swiss_table_min_capacity_for(min_capacity));
}

using _swiss_table_resize_t = void(_swiss_table& table);

void* _swiss_table_insert_1(
	_swiss_table& table,
	size_t sizeof_t,
	size_t hash,
	_swiss_table_resize_t* resize);

template<size_t SizeofT, typename TK, typename UK, typename P>
insert_result<void*> _swiss_table_insert(
	_swiss_table_with_policies<P>& table,
	size_t const hash,
	input_t<UK> key,
	_swiss_table_resize_t* const resize)
{
	auto const [slot, slot_index] = _swiss_table_find_1<SizeofT, TK, UK>(table, hash, key);

	if (slot != nullptr)
	{
		return { slot, false };
	}

	return { _swiss_table_insert_1(table, SizeofT, hash, resize), true };
}

void _swiss_table_erase_1(_swiss_table& table, size_t sizeof_t, size_t slot_index);

template<size_t SizeofT, typename TK, typename UK, typename P>
void* _swiss_table_erase(_swiss_table_with_policies<P>& table, size_t const hash, input_t<UK> key)
{
	auto const [slot, slot_index] = _swiss_table_find_1<SizeofT, TK, UK>(table, hash, key);

	if (slot != nullptr)
	{
		_swiss_table_erase_1(table, SizeofT, slot_index);
	}

	return slot;
}

template<typename T>
void _swiss_table_destroy_elements(_swiss_table& table) noexcept
{
	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		size_t const capacity = table.m_capacity;

		unsigned char* data = table._get_ptr();
		_swiss_table_ctrl* ctrl = _swiss_table_ctrl_ptr(data, capacity, sizeof(T));

		for (size_t i = 0; i < capacity; ++i)
		{
			if (_swiss_table_ctrl_get(ctrl, i) >= static_cast<_swiss_table_ctrl>(0))
			{
				unsigned char* const slot = data + i * sizeof(T);
				std::destroy_at(reinterpret_cast<T*>(slot));

#if vsm_has_address_sanitizer
				__asan_poison_memory_region(slot, sizeof(T));
#endif
			}
		}
	}
}

template<typename T>
void _swiss_table_clear(_swiss_table& table) noexcept
{
	vsm_assert(table.m_size != 0);

	_swiss_table_destroy_elements<T>(table);

	size_t const capacity = table.m_capacity;
	size_t const max_size = _swiss_table_max_size(capacity);

	if (table.m_free != max_size)
	{
		_swiss_table_ctrl* const ctrl = _swiss_table_ctrl_ptr(
			table._get_ptr(),
			capacity,
			sizeof(T));

		_swiss_table_ctrl_init(ctrl, capacity);
	}

	table.m_size = 0;
	table.m_free = max_size;
}

template<size_t SizeofT>
void _swiss_table_initialize(_swiss_table& table, size_t const capacity) noexcept
{
	table.m_size = 0;
	table.m_free = _swiss_table_max_size(capacity);
	table._set(capacity, /* is_local: */ capacity != 0);

	if (capacity != 0)
	{
		_swiss_table_ctrl_init(
			_swiss_table_ctrl_ptr(table._get_storage(), capacity, SizeofT),
			capacity);

#if vsm_has_address_sanitizer
		__asan_poison_memory_region(
			table._get_storage(),
			_swiss_table_storage_size(capacity, SizeofT));
#endif
	}
	else
	{
		table._set_storage_ptr(reinterpret_cast<unsigned char*>(
			const_cast<_swiss_table_ctrl*>(_swiss_table_empty_group.data())));
	}
}

template<typename T, typename P, typename A>
void _swiss_table_destroy(_swiss_table_with_allocator<P, A>& table) noexcept
{
#if vsm_has_address_sanitizer
	if (table.m_is_local)
	{
		__asan_unpoison_memory_region(
			table._get_storage(),
			_swiss_table_storage_size(table.m_capacity, sizeof(T)));
	}
#endif

	if (table.m_size != 0)
	{
		_swiss_table_destroy_elements<T>(table);
	}

	_swiss_table_deallocate(table, sizeof(T));
}

template<typename T, typename P, typename A>
void _swiss_table_move(
	_swiss_table_with_allocator<P, A>& dst,
	_swiss_table_with_allocator<P, A>& src) noexcept
{
	if (src.m_is_local)
	{
		size_t const capacity = src.m_capacity;

		unsigned char* const dst_data = dst._get_storage();
		unsigned char* const src_data = src._get_storage();

		_swiss_table_ctrl* const dst_ctrl =
			_swiss_table_ctrl_ptr(dst_data, capacity, sizeof(T));

		_swiss_table_ctrl* const src_ctrl =
			_swiss_table_ctrl_ptr(src_data, capacity, sizeof(T));

#if vsm_has_address_sanitizer
		__asan_poison_memory_region(
			dst_data,
			_swiss_table_storage_size(capacity, sizeof(T)));
#endif

		if (src.m_size != 0)
		{
#if !vsm_has_address_sanitizer
			if constexpr (is_bitwise_relocatable_v<T>)
			{
				std::memcpy(dst_data, src_data, capacity * sizeof(T));
			}
			else
#endif
			{
				for (size_t i = 0; i < capacity; ++i)
				{
					if (_swiss_table_ctrl_get(src_ctrl, i) >= static_cast<_swiss_table_ctrl>(0))
					{
						T* const src_slot = reinterpret_cast<T*>(src_data + i * sizeof(T));
						T* const dst_slot = reinterpret_cast<T*>(dst_data + i * sizeof(T));

#if vsm_has_address_sanitizer
						__asan_unpoison_memory_region(dst_slot, sizeof(T));
#endif

						vsm::relocate_at(src_slot, dst_slot);
					}
				}
			}

			vsm_memcpy_no_sanitize_address(
				dst_ctrl,
				src_ctrl,
				_swiss_table_ctrl_size(capacity));
		}
		else
		{
			_swiss_table_ctrl_init(dst_ctrl, capacity);
		}

#if vsm_has_address_sanitizer
		__asan_unpoison_memory_region(
			src_data,
			_swiss_table_storage_size(capacity, sizeof(T)));
#endif
	}
	else
	{
		dst._set_storage_ptr(src._get_storage_ptr());
	}

	static_cast<_swiss_table&>(dst) = static_cast<_swiss_table&>(src);
}

template<typename T, typename P, typename A>
void _swiss_table_move_assign(
	_swiss_table_with_allocator<P, A>& dst,
	_swiss_table_with_allocator<P, A>& src) noexcept
{
	_swiss_table_destroy<T>(dst);
	dst.m_policies = static_cast<P const&>(src.m_policies);
	dst.m_allocator = static_cast<A const&>(src.m_allocator);
	_swiss_table_move<T>(dst, src);
}

template<typename T, typename P, typename A>
void _swiss_table_swap(
	_swiss_table_with_allocator<P, A>& lhs,
	_swiss_table_with_allocator<P, A>& rhs) noexcept;


struct _swiss_table_sentinel
{
	explicit _swiss_table_sentinel() = default;
};

template<typename T>
class _swiss_table_iterator_1;

template<typename T>
class _swiss_table_iterator_n;

template<typename T, typename Base>
class _swiss_table_iterator_base : Base
{
public:
	using value_type = T;
	using difference_type = ptrdiff_t;

	_swiss_table_iterator_base() = default;

	_swiss_table_iterator_base(_swiss_table_sentinel) noexcept
	{
		Base::m_data = nullptr;
	}

	explicit _swiss_table_iterator_base(Base const& base) noexcept
		: Base(base)
	{
	}

	[[nodiscard]] T& operator*() const
	{
		return *reinterpret_cast<T*>(Base::m_data);
	}

	[[nodiscard]] T* operator->() const
	{
		return reinterpret_cast<T*>(Base::m_data);
	}

	[[nodiscard]] friend bool operator==(
		_swiss_table_iterator_base const& lhs,
		_swiss_table_sentinel) noexcept
	{
		return lhs.Base::m_data == nullptr;
	}
	
	[[nodiscard]] friend bool operator==(
		_swiss_table_iterator_base const& lhs,
		_swiss_table_iterator_base const& rhs) noexcept
	{
		return lhs.Base::m_data == rhs.Base::m_data;
	}

private:
	template<typename>
	friend class _swiss_table_iterator_1;

	template<typename>
	friend class _swiss_table_iterator_n;
};

struct _swiss_table_iterator_n_base
{
	unsigned char* m_data;
	_swiss_table_ctrl* m_ctrl;

	template<size_t SizeofT>
	void skip_free_slots() noexcept
	{
		while (*m_ctrl < _swiss_table_ctrl::end)
		{
			auto const shift = _swiss_table_group(m_ctrl).count_leading_free_or_end();

			m_data += shift * SizeofT;
			m_ctrl += shift;
		}

		if (*m_ctrl == _swiss_table_ctrl::end)
		{
			m_data = nullptr;
		}
	}

	template<size_t SizeofT>
	void advance() noexcept
	{
		m_data += SizeofT;
		m_ctrl += 1;

		skip_free_slots<SizeofT>();
	}

	template<size_t SizeofT>
	static _swiss_table_iterator_n_base begin(
		unsigned char* const data,
		size_t const capacity) noexcept
	{
		_swiss_table_iterator_n_base iterator =
		{
			.m_data = data,
			.m_ctrl = _swiss_table_ctrl_ptr(data, capacity, SizeofT),
		};

		iterator.skip_free_slots<SizeofT>();
		return iterator;
	}
};

template<typename T>
class _swiss_table_iterator_n : public _swiss_table_iterator_base<T, _swiss_table_iterator_n_base>
{
	using base = _swiss_table_iterator_base<T, _swiss_table_iterator_n_base>;

public:
	using base::base;

	template<cv_convertible_to<T> U>
	_swiss_table_iterator_n(_swiss_table_iterator_n<U> const& iterator) noexcept
		: base(static_cast<_swiss_table_iterator_n_base>(iterator))
	{
	}

	_swiss_table_iterator_n& operator++() &
	{
		_swiss_table_iterator_n_base::advance<sizeof(T)>();
		return *this;
	}

	[[nodiscard]] _swiss_table_iterator_n& operator++(int) &
	{
		auto it = *this;
		_swiss_table_iterator_n_base::advance<sizeof(T)>();
		return it;
	}
};

struct _swiss_table_iterator_1_base
{
	void* m_data;
};

template<typename T>
class _swiss_table_iterator_1 : public _swiss_table_iterator_base<T, _swiss_table_iterator_1_base>
{
	using base = _swiss_table_iterator_base<T, _swiss_table_iterator_1_base>;

public:
	using base::base;

	template<cv_convertible_to<T> U>
	_swiss_table_iterator_1(_swiss_table_iterator_1<U> const& iterator) noexcept
		: base(static_cast<_swiss_table_iterator_1_base const&>(iterator))
	{
	}

	template<cv_convertible_to<T> U>
	_swiss_table_iterator_1(_swiss_table_iterator_n<U> const& iterator) noexcept
		: base(_swiss_table_iterator_1_base(iterator.m_data))
	{
	}

	explicit _swiss_table_iterator_1(T* const data) noexcept
		: base(_swiss_table_iterator_1_base(const_cast<void*>(static_cast<void const*>(data))))
	{
	}

	_swiss_table_iterator_1 operator++() &
	{
		_swiss_table_iterator_1_base::m_data = nullptr;
		return *this;
	}

	[[nodiscard]] _swiss_table_iterator_1 operator++(int) &
	{
		auto it = *this;
		_swiss_table_iterator_1_base::m_data = nullptr;
		return *this;
	}
};

template<typename T, typename K, typename P, typename A>
class _swiss_table_base_impl : _swiss_table_with_allocator<P, A>
{
public:
	using value_type                    = T;
	using key_type                      = K;
	using policies_type                 = P;
	using allocator_type                = A;
	using size_type                     = size_t;
	using difference_type               = ptrdiff_t;
	using reference                     = T&;
	using const_reference               = T const&;
	using pointer                       = T*;
	using const_pointer                 = T const*;
	using iterator                      = _swiss_table_iterator_n<T>;
	using const_iterator                = _swiss_table_iterator_n<T const>;
	using single_iterator               = _swiss_table_iterator_1<T>;
	using const_single_iterator         = _swiss_table_iterator_1<T const>;
	using insert_result                 = vsm::insert_result<single_iterator>;


	[[nodiscard]] P const& policies() const noexcept
	{
		return this->m_policies;
	}

	[[nodiscard]] A const& allocator() const noexcept
	{
		return this->m_allocator;
	}


	[[nodiscard]] bool empty() const noexcept
	{
		return this->m_size == 0;
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return this->m_size;
	}

	[[nodiscard]] size_t max_size() const noexcept
	{
		return _swiss_table_max_size(std::numeric_limits<size_t>::max() / 2);
	}

	[[nodiscard]] size_t capacity() const noexcept
	{
		return _swiss_table_max_size(this->m_capacity);
	}

	void reserve(size_t const min_capacity)
	{
		static_assert(is_nothrow_relocatable_v<T>);

		if (min_capacity > _swiss_table_max_size(this->m_capacity))
		{
			_swiss_table_reserve<T, K>(*this, min_capacity);
		}
	}

	void clear()
	{
		if (this->m_size != 0)
		{
			_swiss_table_clear<T>(*this);
		}
	}


	template<hash_table_key<_swiss_table_base_impl> Key>
	[[nodiscard]] single_iterator find(Key const& key)
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		void* const storage = _swiss_table_find<sizeof(T), K, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical);

		return single_iterator(static_cast<T*>(storage));
	}

	template<hash_table_key<_swiss_table_base_impl> Key>
	[[nodiscard]] const_single_iterator find(Key const& key) const
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		void* const storage = _swiss_table_find<sizeof(T), K, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical);

		return const_single_iterator(static_cast<T const*>(storage));
	}


	template<hash_table_key<_swiss_table_base_impl> Key>
	size_t erase(Key const& key)
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		void* const storage = _swiss_table_erase<sizeof(T), Key, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical);

		if (storage == nullptr)
		{
			return 0;
		}

		std::destroy_at(static_cast<T*>(storage));
		return 1;
	}

	void erase(const_single_iterator const iterator)
	{
		std::destroy_at(std::to_address(iterator));
		_swiss_table_erase<T>(*this, std::to_address(iterator));
	}


	[[nodiscard]] iterator begin()
	{
		return iterator(_swiss_table_iterator_n_base::begin<sizeof(T)>(
			this->_get_ptr(),
			this->m_capacity));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(_swiss_table_iterator_n_base::begin<sizeof(T)>(
			this->_get_ptr(),
			this->m_capacity));
	}

	[[nodiscard]] _swiss_table_sentinel end() const
	{
		return _swiss_table_sentinel();
	}

protected:
	using _swiss_table_with_allocator<P, A>::_swiss_table_with_allocator;

	template<typename Key>
	[[nodiscard]] insert_result insert_uninitialized(Key const& key)
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		auto const [storage, inserted] = _swiss_table_insert<sizeof(T), Key, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical,
			_swiss_table_resize_3<T, K, P, A>);

		return { single_iterator(static_cast<T*>(storage)), inserted };
	}

	template<typename BaseFacade, size_t Capacity>
	friend class _swiss_table_impl;
};


template<typename SwissTable, typename T, typename P, typename A, size_t Capacity>
using _swiss_table_base = _swiss_table_storage<
	SwissTable,
	T,
	P,
	A,
	Capacity == 0
		? 0
		: _swiss_table_min_capacity_for(Capacity)>;

template<
	template<typename...> typename BaseFacade,
	typename T,
	typename K,
	typename P,
	typename A,
	size_t Capacity>
class _swiss_table_impl<BaseFacade<_swiss_table_base_impl<T, K, P, A>>, Capacity>
	: public _swiss_table_base<BaseFacade<_swiss_table_base_impl<T, K, P, A>>, T, P, A, Capacity>
{
	using base = _swiss_table_base<BaseFacade<_swiss_table_base_impl<T, K, P, A>>, T, P, A, Capacity>;

	static_assert(offsetof(_swiss_table, m_size) == 0);
	static_assert(offsetof(base, m_size) + sizeof(_swiss_table) == offsetof(base, m_storage_ptr));

public:
	_swiss_table_impl()
	{
		_initialize();
	}

	explicit _swiss_table_impl(any_cvref_of<P> auto&& policies)
		: base(vsm_forward(policies), _swiss_table_placeholder_t())
	{
		_initialize();
	}

	explicit _swiss_table_impl(any_cvref_of<A> auto&& allocator)
		: base(_swiss_table_placeholder_t(), vsm_forward(allocator))
	{
		_initialize();
	}

	explicit _swiss_table_impl(any_cvref_of<P> auto&& policies, any_cvref_of<A> auto&& allocator)
		: base(vsm_forward(policies), vsm_forward(allocator))
	{
		_initialize();
	}

	_swiss_table_impl(_swiss_table_impl&& other) noexcept
		requires allocators::is_propagatable_v<A>
		: base(other.m_policies, other.m_allocator)
	{
		static_assert(is_nothrow_relocatable_v<T>);
		static_assert(std::is_nothrow_copy_constructible_v<P>);
		static_assert(std::is_nothrow_copy_constructible_v<A>);

		_swiss_table_move<T>(*this, other);
		other._initialize();
	}

	_swiss_table_impl& operator=(_swiss_table_impl&& other) & noexcept
		requires allocators::is_propagatable_v<A>
	{
		static_assert(is_nothrow_relocatable_v<T>);
		static_assert(std::is_nothrow_copy_constructible_v<P>);
		static_assert(std::is_nothrow_copy_constructible_v<A>);

		if (vsm_likely(this != &other))
		{
			_swiss_table_move_assign<T>(*this, other);
			other._initialize();
		}

		return *this;
	}

	~_swiss_table_impl()
	{
		_swiss_table_destroy<T>(*this);
	}


	void shrink_to_fit();

	void swap(_swiss_table_impl& other) noexcept
		requires allocators::is_propagatable_v<A>
	{
		static_assert(std::is_nothrow_swappable_v<P>);
		static_assert(std::is_nothrow_swappable_v<A>);

		_swiss_table_swap<T, Capacity != 0>(*this, other);
	}

private:
	void vsm_always_inline _initialize() noexcept
	{
		static constexpr size_t small_capacity = Capacity == 0
			? 0
			: _swiss_table_min_capacity_for(Capacity);

		_swiss_table_initialize<sizeof(T)>(*this, small_capacity);
	}
};

} // namespace vsm::detail
