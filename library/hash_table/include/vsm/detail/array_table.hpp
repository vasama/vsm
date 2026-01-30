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

#include <bit>

namespace vsm::detail {

struct _array_table_placeholder_t
{
	explicit _array_table_placeholder_t() = default;
};


template<typename I>
using _array_table_hash_t = select_t<(sizeof(I) >= sizeof(size_t)), size_t, I>;

template<typename I>
inline vsm_always_inline _array_table_hash_t<I> _array_table_hash(size_t const hash) noexcept
{
	// TODO: Consider mixing?
	return static_cast<_array_table_hash_t<I>>(hash);
}


template<
	typename T,
	typename K,
	typename I,
	typename P,
	typename A>
class _array_table_base_impl;

template<typename ArrayTableBase, size_t Capacity>
class _array_table_impl;


template<typename I>
struct _array_table_bucket
{
	I index;
	_array_table_hash_t<I> hash;

	vsm_no_sanitize_address
	vsm_always_inline _array_table_bucket load() const
	{
		return *this;
	}

	vsm_no_sanitize_address
	vsm_always_inline void store(_array_table_bucket const& other)
	{
		*this = other;
	}

	vsm_no_sanitize_address
	vsm_always_inline I load_index() const
	{
		return index;
	}

	vsm_no_sanitize_address
	vsm_always_inline void store_index(I const new_index)
	{
		index = new_index;
	}

	vsm_no_sanitize_address
	vsm_always_inline _array_table_hash_t<I> load_hash() const
	{
		return hash;
	}

	vsm_no_sanitize_address
	vsm_always_inline void store_hash(_array_table_hash_t<I> const new_hash)
	{
		hash = new_hash;
	}

	vsm_no_sanitize_address
	vsm_always_inline void swap(_array_table_bucket& other)
	{
		{
			auto const temporary = this->index;
			this->index = other.index;
			other.index = temporary;
		}

		{
			auto const temporary = this->hash;
			this->hash = other.hash;
			other.hash = temporary;
		}
	}
};

template<typename I>
extern _array_table_bucket<I> const _array_table_empty_bucket;

extern template _array_table_bucket<uint_least16_t> const _array_table_empty_bucket<uint_least16_t>;
extern template _array_table_bucket<uint_least32_t> const _array_table_empty_bucket<uint_least32_t>;
extern template _array_table_bucket<size_t> const _array_table_empty_bucket<size_t>;


template<size_t Size>
struct _array_table_padding
{
	unsigned char m_padding[Size];
};

template<>
struct _array_table_padding<0>
{
};

template<typename T, size_t Alignment>
using _array_table_padding_before = _array_table_padding<Alignment - sizeof(T) % Alignment>;

template<
	typename T1,
	typename T2,
	size_t Alignment = std::max(alignof(T1), alignof(T2))>
using _array_table_padding_between = _array_table_padding<Alignment - (sizeof(T1) + sizeof(T2)) % Alignment>;

template<typename I>
struct _array_table_size
{
	I m_size;
};

template<typename I>
struct _array_table_hash_mask
{
	I m_hash_mask : sizeof(I) * CHAR_BIT - 1;
	I m_is_local : 1;
};

template<typename I>
struct _array_table
	: _array_table_size<I>
	, _array_table_hash_mask<I>
{
	void _set(size_t const hash_mask, bool const is_local)
	{
		vsm_assert(hash_mask <= std::numeric_limits<I>::max());

		vsm_gcc_diagnostic(push)
		vsm_gcc_diagnostic(ignored "-Wconversion")

		this->m_hash_mask = hash_mask;
		this->m_is_local = is_local;

		vsm_gcc_diagnostic(pop)
	}

	[[nodiscard]] _array_table_bucket<I>* _get_ptr() const
	{
		return this->m_is_local ? _get_storage() : _get_storage_ptr();
	}

	[[nodiscard]] _array_table_bucket<I>* _get_storage() const
	{
		return reinterpret_cast<_array_table_bucket<I>*>(const_cast<_array_table*>(this) + 1);
	}

	vsm_no_sanitize_address
		[[nodiscard]] _array_table_bucket<I>* _get_storage_ptr() const
	{
		return *reinterpret_cast<_array_table_bucket<I>*const*>(this + 1);
	}

	vsm_no_sanitize_address
		void _set_storage_ptr(_array_table_bucket<I>* const ptr)
	{
		*reinterpret_cast<_array_table_bucket<I>**>(this + 1) = ptr;
	}
};

template<typename P>
struct _array_table_policies
{
	vsm_no_unique_address P m_policies;

	_array_table_policies() = default;

	explicit _array_table_policies(auto&& policies)
		: m_policies(vsm_forward(policies))
	{
	}
};

template<typename A>
struct _array_table_allocator
{
	vsm_no_unique_address A m_allocator;

	_array_table_allocator() = default;

	explicit _array_table_allocator(auto&& allocator)
		: m_allocator(vsm_forward(allocator))
	{
	}
};

vsm_msvc_warning(push)
vsm_msvc_warning(disable: 4584) // 'T' is already a base-class of 'U'

template<typename I, typename P>
struct vsm_empty_bases _array_table_with_policies
	: _array_table_policies<P>
	, _array_table_padding_between<P, _array_table<I>>
	, _array_table<I>
{
	using _array_table_policies<P>::_array_table_policies;
};

template<typename I, typename P, typename A>
struct vsm_empty_bases _array_table_with_allocator
	: _array_table_allocator<A>
	, _array_table_padding_between<A, _array_table_with_policies<I, P>>
	, _array_table_with_policies<I, P>
{
	_array_table_with_allocator() = default;

	explicit _array_table_with_allocator(auto&& policies, _array_table_placeholder_t)
		: _array_table_with_policies<I, P>(vsm_forward(policies))
	{
		static_assert(std::is_default_constructible_v<A>);
	}

	explicit _array_table_with_allocator(_array_table_placeholder_t, auto&& allocator)
		: _array_table_allocator<A>(vsm_forward(allocator))
	{
		static_assert(std::is_default_constructible_v<P>);
	}

	explicit _array_table_with_allocator(auto&& policies, auto&& allocator)
		: _array_table_allocator<A>(vsm_forward(allocator))
		, _array_table_with_policies<I, P>(vsm_forward(policies))
	{
	}
};

inline constexpr size_t _array_table_max_size(size_t const hash_mask)
{
	return (hash_mask + 1) * 3 / 4;
}

inline constexpr size_t _array_table_min_capacity_for(size_t const min_max_size)
{
	size_t hash_mask = std::bit_ceil(min_max_size + 1) - 1;

	while (_array_table_max_size(hash_mask) < min_max_size)
	{
		hash_mask = hash_mask * 2 + 1;
	}

	return hash_mask;
}

template<typename I>
constexpr size_t _array_table_storage_size(size_t const hash_mask, size_t const sizeof_t)
{
	return hash_mask * sizeof(_array_table_bucket<I>) + _array_table_max_size(hash_mask) * sizeof_t;
}

template<typename ArrayTable, typename T, typename I, typename P, typename A, size_t C>
class vsm_empty_bases _array_table_storage
	: _array_table_padding_before<ArrayTable, std::max(alignof(T), alignof(void*))>
	, public ArrayTable
{
	static_assert(std::has_single_bit(C + 1));

	using ArrayTable::ArrayTable;

	union
	{
		unsigned char* m_storage_ptr;
		alignas(T) unsigned char m_storage[_array_table_storage_size<I>(C, sizeof(T))];
	};

	template<typename BaseFacade, size_t Capacity>
	friend class _array_table_impl;
};

template<typename ArrayTable, typename T, typename I, typename P, typename A>
class vsm_empty_bases _array_table_storage<ArrayTable, T, I, P, A, 0>
	: _array_table_padding_before<ArrayTable, alignof(void*)>
	, public ArrayTable
{
	using ArrayTable::ArrayTable;

	unsigned char* m_storage_ptr;

	template<typename BaseFacade, size_t Capacity>
	friend class _array_table_impl;
};

vsm_msvc_warning(pop)

template<typename I>
unsigned char* _array_table_data(_array_table_bucket<I>* const buckets, size_t const hash_mask)
{
	return reinterpret_cast<unsigned char*>(buckets + (hash_mask + 1));
}

#if vsm_has_address_sanitizer
template<size_t SizeofT, typename I>
void _array_table_update_annotation(
	_array_table<I> const& table,
	size_t const old_size,
	size_t const new_size)
{
	auto const buckets = table._get_ptr();
	auto const data = _array_table_data(buckets, table.m_hash_mask);
	auto const max_size = _array_table_max_size(table.m_hash_mask);

	__sanitizer_annotate_contiguous_container(
		data,
		data + max_size * SizeofT,
		data + old_size * SizeofT,
		data + new_size * SizeofT);
}

template<size_t SizeofT, typename I>
void _array_table_create_annotation(_array_table<I> const& table)
{
	_array_table_update_annotation<SizeofT>(table, 0, 0);
}

template<size_t SizeofT, typename I>
void _array_table_remove_annotation(_array_table<I> const& table)
{
	_array_table_update_annotation<SizeofT>(table, table.m_size, 0);
}
#endif

template<typename I>
size_t _array_table_bucket_distance(
	_array_table_bucket<I> const& bucket,
	size_t const index,
	size_t const hash_mask)
{
	size_t const ideal_index = bucket.load_hash() & hash_mask;
	return (index - ideal_index) & hash_mask;
}

template<typename I>
void _array_table_shift_indices(
	_array_table_bucket<I>* const buckets,
	size_t const hash_mask,
	size_t const min_index,
	ptrdiff_t const index_shift)
{
	for (_array_table_bucket<I>& bucket : std::span(buckets, hash_mask + 1))
	{
		size_t const old_index = bucket.load_index();
		size_t const new_index = old_index < min_index
			? old_index
			: old_index + static_cast<size_t>(index_shift);

		bucket.store_index(static_cast<I>(new_index));
	}
}

template<typename I>
void _array_table_shift_buckets(
	_array_table_bucket<I>* const buckets,
	size_t const hash_mask,
	size_t empty_bucket_index)
{
	vsm_assert(buckets[empty_bucket_index].index == std::numeric_limits<I>::max());

	for (size_t prev_bucket_index = empty_bucket_index;;)
	{
		_array_table_bucket<I>& prev_bucket = buckets[prev_bucket_index];
		vsm_assert(prev_bucket.load_index() == std::numeric_limits<I>::max());

		size_t const next_bucket_index = (prev_bucket_index + 1) & hash_mask;
		_array_table_bucket<I>& next_bucket = buckets[next_bucket_index];

		if (next_bucket.load_index() == std::numeric_limits<I>::max())
		{
			break;
		}

		if (_array_table_bucket_distance(next_bucket, next_bucket_index, hash_mask) == 0)
		{
			break;
		}

		prev_bucket.swap(next_bucket);
		prev_bucket_index = next_bucket_index;
	}
}

struct _array_table_find_t
{
	void* slot;
	size_t bucket_index;
	size_t bucket_distance;
};

template<size_t SizeofT, typename TK, typename UK, typename I, typename P>
_array_table_find_t _array_table_find_1(
	_array_table_with_policies<I, P> const& table,
	size_t const hash,
	input_t<UK> key)
{
	size_t const hash_mask = table.m_hash_mask;

	_array_table_bucket<I>* const buckets = table._get_ptr();
	unsigned char* const data = _array_table_data(buckets, hash_mask);

	size_t bucket_index = hash & hash_mask;
	size_t bucket_distance = 0;

	while (true)
	{
		_array_table_bucket<I> const& bucket = buckets[bucket_index];

		if (bucket.load_index() == std::numeric_limits<I>::max() ||
			bucket_distance > _array_table_bucket_distance(bucket, bucket_index, hash_mask))
		{
			return { nullptr, bucket_index, bucket_distance };
		}

		if (bucket.load_hash() == _array_table_hash<I>(hash))
		{
			unsigned char* const slot = data + bucket.load_index() * SizeofT;

			bool const is_equal = static_cast<P const&>(table.m_policies).comparator(
				key,
				vsm::normalize_key(
					static_cast<P const&>(table.m_policies)
						.key_selector(*reinterpret_cast<TK const*>(slot))));

			if (is_equal)
			{
				return { slot, bucket_index, bucket_distance };
			}
		}

		bucket_index = (bucket_index + 1) & hash_mask;
		bucket_distance += 1;
	}
}

template<size_t SizeofT, typename TK, typename UK, typename I, typename P>
void* _array_table_find(
	_array_table_with_policies<I, P> const& table,
	size_t const hash,
	input_t<UK> key)
{
	return _array_table_find_1<SizeofT, TK, UK>(table, hash, key).slot;
}

template<typename I, typename A>
_array_table_bucket<I>* _array_table_allocate(
	A const& allocator,
	size_t const sizeof_t,
	size_t& hash_mask)
{
	// TODO: Specify max allocation size.
	auto const new_allocation = vsm::allocate_or_throw(
		allocator,
		_array_table_storage_size<I>(hash_mask, sizeof_t));

	while (true)
	{
		size_t const new_hash_mask = hash_mask * 2 + 1;

		if (new_hash_mask > std::numeric_limits<I>::max() / 2)
		{
			break;
		}

		if (new_allocation.size < _array_table_storage_size<I>(new_hash_mask, sizeof_t))
		{
			break;
		}

		hash_mask = new_hash_mask;
	}

	auto const buckets = static_cast<_array_table_bucket<I>*>(new_allocation.storage);

#if vsm_has_address_sanitizer
	__asan_poison_memory_region(
		buckets,
		(hash_mask + 1) * sizeof(_array_table_bucket<I>));
#endif

	return buckets;
}

template<typename I, typename P, typename A>
void _array_table_deallocate(_array_table_with_allocator<I, P, A>& table, size_t const sizeof_t)
{
	if (!table.m_is_local)
	{
		_array_table_bucket<I>* const buckets = table._get_storage_ptr();

		if (buckets != &_array_table_empty_bucket<I>)
		{
			vsm_assert(buckets != nullptr);

#if vsm_has_address_sanitizer
			__asan_unpoison_memory_region(
				buckets,
				(table.m_hash_mask + 1) * sizeof(_array_table_bucket<I>));
#endif

			table.m_allocator.deallocate(vsm::allocation(
				buckets,
				_array_table_storage_size<I>(table.m_hash_mask, sizeof_t)));
		}
	}
}

template<typename T, typename K, typename I, typename P, typename A>
void _array_table_resize_1(_array_table_with_allocator<I, P, A>& table, size_t new_hash_mask)
{
	vsm_assert(std::has_single_bit(new_hash_mask + 1));
	vsm_assert(_array_table_max_size(new_hash_mask) >= table.m_size);

	if (new_hash_mask > std::numeric_limits<I>::max() / 2)
	{
		vsm_except_throw_or_terminate(std::length_error("deterministic table size out of range"));
	}

	using bucket_type = _array_table_bucket<I>;

	bucket_type* const new_buckets = _array_table_allocate<I>(
		table.m_allocator,
		sizeof(T),
		new_hash_mask);

	bucket_type* const old_buckets = table._get_ptr();

	size_t const old_hash_mask = table.m_hash_mask;

	size_t const new_bucket_count = new_hash_mask + 1;
	size_t const old_bucket_count = old_hash_mask + 1;

	vsm_memset_no_sanitize_address(
		new_buckets,
		0xFF,
		new_bucket_count * sizeof(bucket_type));

	for (size_t i = 0; i < old_bucket_count; ++i)
	{
		bucket_type const& old_bucket = old_buckets[i];

		if (old_bucket.load_index() != std::numeric_limits<I>::max())
		{
			_array_table_insert_1(new_buckets, new_hash_mask, old_bucket);
		}
	}

	unsigned char* const new_data = _array_table_data(new_buckets, new_hash_mask);
	unsigned char* const old_data = _array_table_data(old_buckets, old_hash_mask);

	if (size_t const size = table.m_size)
	{
		vsm::uninitialized_relocate_n(
			reinterpret_cast<T*>(old_data),
			size,
			reinterpret_cast<T*>(new_data));
	}

	_array_table_deallocate(table, sizeof(T));

	table._set_storage_ptr(new_buckets);
	table._set(new_hash_mask, /* is_local: */ false);
}

template<typename T, typename K, typename I, typename P, typename A>
void _array_table_resize_2(_array_table_with_allocator<I, P, A>& table)
{
	_array_table_resize_1<T, K>(table, table.m_hash_mask * 2 + 1);
}

template<typename T, typename K, typename I, typename P, typename A>
void _array_table_resize_3(_array_table<I>& table)
{
	_array_table_resize_2<T, K>(static_cast<_array_table_with_allocator<I, P, A>&>(table));
}

template<typename T, typename K, typename I, typename P, typename A>
void _array_table_reserve(_array_table_with_allocator<I, P, A>& table, size_t const min_capacity)
{
	vsm_assert(min_capacity > _array_table_max_size(table.m_hash_mask));
	_array_table_resize_1<T, K>(table, _array_table_min_capacity_for(min_capacity));
}

template<typename I>
void _array_table_insert_2(
	_array_table_bucket<I>* const buckets,
	size_t const hash_mask,
	_array_table_bucket<I> new_bucket,
	size_t bucket_index,
	size_t bucket_distance)
{
	while (true)
	{
		_array_table_bucket<I>& bucket = buckets[bucket_index];

		if (bucket.load_index() == std::numeric_limits<I>::max())
		{
			bucket.store(new_bucket);
			return;
		}

		size_t const distance = _array_table_bucket_distance(bucket, bucket_index, hash_mask);

		if (bucket_distance > distance)
		{
			auto const t = bucket.load();
			bucket.store(new_bucket);
			new_bucket = t;

			bucket_distance = distance;
		}

		bucket_index = (bucket_index + 1) & hash_mask;
		bucket_distance += 1;
	}
}

template<typename I>
void _array_table_insert_1(
	_array_table_bucket<I>* const buckets,
	size_t const hash_mask,
	_array_table_bucket<I> new_bucket)
{
	_array_table_insert_2(buckets, hash_mask, new_bucket, new_bucket.hash & hash_mask, 0);
}

template<typename I>
using _array_table_resize_t = void(_array_table<I>& table);

template<size_t SizeofT, typename TK, typename UK, typename I, typename P>
insert_result<void*> _array_table_insert(
	_array_table_with_policies<I, P>& table,
	size_t const hash,
	input_t<UK> key,
	_array_table_resize_t<I>* const resize)
{
	auto [slot, bucket_index, bucket_distance] =
		_array_table_find_1<SizeofT, TK, UK>(table, hash, key);

	if (slot != nullptr)
	{
		return { slot, false };
	}

	size_t hash_mask = table.m_hash_mask;

	if (table.m_size == _array_table_max_size(hash_mask))
	{
		resize(table);

		// It's very important to update the local copy here.
		hash_mask = table.m_hash_mask;

		bucket_index = hash & hash_mask;
		bucket_distance = 0;
	}

	_array_table_bucket<I>* const buckets = table._get_ptr();
	unsigned char* const data = _array_table_data(buckets, hash_mask);

	_array_table_insert_2(
		buckets,
		hash_mask,
		_array_table_bucket<I>(table.m_size, _array_table_hash<I>(hash)),
		bucket_index,
		bucket_distance);

#if vsm_has_address_sanitizer
	_array_table_update_annotation<SizeofT>(table, table.m_size, table.m_size + 1);
#endif

	return { data + table.m_size++ * SizeofT, true };
}

template<typename T, typename I>
void _array_table_erase(_array_table<I>& table, size_t const bucket_index)
{
	size_t const hash_mask = table.m_hash_mask;

	_array_table_bucket<I>* const buckets = table._get_ptr();
	unsigned char* const data = _array_table_data(buckets, hash_mask);

	_array_table_bucket<I>& bucket = buckets[bucket_index];

	size_t const data_index = bucket.load_index();
	size_t const table_size = table.m_size--;

	if (data_index != table_size - 1)
	{
		vsm::uninitialized_relocate(
			reinterpret_cast<T*>(data) + data_index + 1,
			reinterpret_cast<T*>(data) + table_size,
			reinterpret_cast<T*>(data) + data_index);

		_array_table_shift_indices(buckets, hash_mask, data_index + 1, -1);
	}

	bucket.store_index(std::numeric_limits<I>::max());
	_array_table_shift_buckets(buckets, hash_mask, bucket_index);
}

template<typename T, typename I>
void _array_table_destroy_elements(_array_table<I>& table) noexcept
{
	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		size_t const hash_mask = table.m_hash_mask;

		unsigned char* const data = _array_table_data(table._get_ptr(), hash_mask);

		std::destroy(
			reinterpret_cast<T*>(data),
			reinterpret_cast<T*>(data) + table.m_size);
	}
}

template<typename I>
void _array_table_clear_buckets(
	_array_table_bucket<I>* const buckets,
	size_t const hash_mask) noexcept
{
	vsm_memset_no_sanitize_address(
		buckets,
		0xFF,
		(hash_mask + 1) * sizeof(_array_table_bucket<I>));
}

template<typename T, typename I>
void _array_table_clear(_array_table<I>& table) noexcept
{
	vsm_assert(table.m_size != 0);

	_array_table_destroy_elements<T>(table);

#if vsm_has_address_sanitizer
	_array_table_update_annotation<sizeof(T)>(table, table.m_size, 0);
#endif

	_array_table_clear_buckets(table._get_ptr(), table.m_hash_mask);

	table.m_size = 0;
}

template<size_t SizeofT, typename I>
void _array_table_initialize(_array_table<I>& table, size_t const hash_mask) noexcept
{
	table.m_size = 0;
	table._set(hash_mask, /* is_local: */ hash_mask != 0);

	if (hash_mask != 0)
	{
#if vsm_has_address_sanitizer
		__asan_poison_memory_region(
			table._get_storage(),
			(hash_mask + 1) * sizeof(_array_table_bucket<I>));

		_array_table_create_annotation<SizeofT>(table);
#endif

		_array_table_clear_buckets(table._get_storage(), hash_mask);
	}
	else
	{
		table._set_storage_ptr(const_cast<_array_table_bucket<I>*>(&_array_table_empty_bucket<I>));
	}
}

template<typename T, typename I, typename P, typename A>
void _array_table_destroy(_array_table_with_allocator<I, P, A>& table) noexcept
{
#if vsm_has_address_sanitizer
	_array_table_remove_annotation<sizeof(T)>(table);

	if (table.m_is_local)
	{
		__asan_unpoison_memory_region(
			table._get_storage(),
			(table.m_hash_mask + 1) * sizeof(_array_table_bucket<I>));
	}
#endif

	if (table.m_size != 0)
	{
		_array_table_destroy_elements<T>(table);
	}

	_array_table_deallocate(table, sizeof(T));
}

template<typename T, typename I, typename P, typename A>
void _array_table_move(
	_array_table_with_allocator<I, P, A>& dst,
	_array_table_with_allocator<I, P, A>& src) noexcept
{
	if (src.m_is_local)
	{
		size_t const hash_mask = src.m_hash_mask;

		_array_table_bucket<I>* const src_buckets = src._get_storage();
		_array_table_bucket<I>* const dst_buckets = dst._get_storage();

		unsigned char* const src_data = _array_table_data<I>(src_buckets, hash_mask);
		unsigned char* const dst_data = _array_table_data<I>(dst_buckets, hash_mask);

#if vsm_has_address_sanitizer
		__asan_poison_memory_region(
			dst_buckets,
			(hash_mask + 1) * sizeof(_array_table_bucket<I>));
#endif

		if (size_t const size = src.m_size)
		{
			vsm_memcpy_no_sanitize_address(
				dst_buckets,
				src_buckets,
				(hash_mask + 1) * sizeof(_array_table_bucket<I>));

			vsm::uninitialized_relocate_n(
				reinterpret_cast<T*>(src_data),
				size,
				reinterpret_cast<T*>(dst_data));
		}
		else
		{
			_array_table_clear_buckets(dst_buckets, hash_mask);
		}

#if vsm_has_address_sanitizer
		__asan_unpoison_memory_region(
			src_buckets,
			(hash_mask + 1) * sizeof(_array_table_bucket<I>));
#endif
	}
	else
	{
		dst._set_storage_ptr(src._get_storage_ptr());
	}

	static_cast<_array_table<I>&>(dst) = static_cast<_array_table<I>&>(src);

#if vsm_has_address_sanitizer
	_array_table_create_annotation<sizeof(T)>(dst);
	_array_table_remove_annotation<sizeof(T)>(src);
#endif
}

template<typename T, typename I, typename P, typename A>
void _array_table_move_assign(
	_array_table_with_allocator<I, P, A>& dst,
	_array_table_with_allocator<I, P, A>& src) noexcept
{
	_array_table_destroy<T>(dst);
	dst.m_policies = static_cast<P const&>(src.m_policies);
	dst.m_allocator = static_cast<A const&>(src.m_allocator);
	_array_table_move<T>(dst, src);
}

template<typename T, typename I, typename P, typename A>
void _array_table_swap(
	_array_table_with_allocator<I, P, A>& lhs,
	_array_table_with_allocator<I, P, A>& rhs) noexcept;


template<typename T, typename K, typename I, typename P, typename A>
class _array_table_base_impl : _array_table_with_allocator<I, P, A>
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
	using iterator                      = T*;
	using const_iterator                = T const*;
	using single_iterator               = iterator;
	using const_single_iterator         = const_iterator;
	using insert_result                 = vsm::insert_result<iterator>;


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
		return _array_table_max_size(std::numeric_limits<I>::max() / 2);
	}

	[[nodiscard]] size_t capacity() const noexcept
	{
		return _array_table_max_size(this->m_hash_mask);
	}

	void reserve(size_t const min_capacity)
	{
		static_assert(is_nothrow_relocatable_v<T>);

		if (min_capacity > _array_table_max_size(this->m_hash_mask))
		{
			_array_table_reserve<T, K>(*this, min_capacity);
		}
	}

	void clear()
	{
		if (this->m_size != 0)
		{
			_array_table_clear<T>(*this);
		}
	}


	template<hash_table_key<_array_table_base_impl> Key>
	[[nodiscard]] iterator find(Key const& key)
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		void* const storage = _array_table_find<sizeof(T), K, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical);

		return iterator(static_cast<T*>(storage));
	}

	template<hash_table_key<_array_table_base_impl> Key>
	[[nodiscard]] const_single_iterator find(Key const& key) const
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		void* const storage = _array_table_find<sizeof(T), K, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical);

		return const_single_iterator(static_cast<T const*>(storage));
	}


	template<hash_table_key<_array_table_base_impl> Key>
	size_t erase(Key const& key)
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		auto const result = _array_table_find_1<sizeof(T), K, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical);

		if (result.slot == nullptr)
		{
			return 0;
		}

		std::destroy_at(static_cast<T*>(result.slot));
		_array_table_erase<T>(*this, result.bucket_index);

		return 1;
	}

	// TODO: Erase by key.
	void erase(const_single_iterator const iterator);


	[[nodiscard]] iterator begin()
	{
		return reinterpret_cast<T*>(_array_table_data(this->_get_ptr(), this->m_hash_mask));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return reinterpret_cast<T*>(_array_table_data(this->_get_ptr(), this->m_hash_mask));
	}

	[[nodiscard]] iterator end()
	{
		size_t const hash_mask = this->m_hash_mask;
		unsigned char* const data = _array_table_data(this->_get_ptr(), hash_mask);
		return reinterpret_cast<T*>(data + _array_table_max_size(hash_mask) * sizeof(T));
	}

	[[nodiscard]] const_iterator end() const
	{
		size_t const hash_mask = this->m_hash_mask;
		unsigned char* const data = _array_table_data(this->_get_ptr(), hash_mask);
		return reinterpret_cast<T*>(data + _array_table_max_size(hash_mask) * sizeof(T));
	}

protected:
	using _array_table_with_allocator<I, P, A>::_array_table_with_allocator;

	template<typename Key>
	[[nodiscard]] insert_result insert_uninitialized(Key const& key)
	{
		decltype(auto) k = detail::get_lookup_key<K>(this->m_policies.key_selector, key);
		decltype(auto) k_canonical = vsm::normalize_key(k);
		using k_type = remove_ref_t<decltype(k_canonical)>;

		auto const [storage, inserted] = _array_table_insert<sizeof(T), Key, k_type>(
			*this,
			static_cast<P const&>(this->m_policies)
				.hasher(static_cast<k_type const&>(k_canonical)),
			k_canonical,
			_array_table_resize_3<T, K, I, P, A>);

		return { iterator(static_cast<T*>(storage)), inserted };
	}

	template<typename BaseFacade, size_t Capacity>
	friend class _array_table_impl;
};


template<typename ArrayTable, typename T, typename I, typename P, typename A, size_t Capacity>
using _array_table_base = _array_table_storage<
	ArrayTable,
	T,
	I,
	P,
	A,
	Capacity == 0
		? 0
		: _array_table_min_capacity_for(Capacity)>;

template<
	template<typename...> typename BaseFacade,
	typename T,
	typename K,
	typename I,
	typename P,
	typename A,
	size_t Capacity>
class _array_table_impl<BaseFacade<_array_table_base_impl<T, K, I, P, A>>, Capacity>
	: public _array_table_base<BaseFacade<_array_table_base_impl<T, K, I, P, A>>, T, I, P, A, Capacity>
{
	using base = _array_table_base<BaseFacade<_array_table_base_impl<T, K, I, P, A>>, T, I, P, A, Capacity>;

	static_assert(offsetof(_array_table<I>, m_size) == 0);
	static_assert(offsetof(base, m_size) + sizeof(_array_table<I>) == offsetof(base, m_storage_ptr));

public:
	_array_table_impl()
	{
		_initialize();
	}

	explicit _array_table_impl(any_cvref_of<P> auto&& policies)
		: base(vsm_forward(policies), _array_table_placeholder_t())
	{
		_initialize();
	}

	explicit _array_table_impl(any_cvref_of<A> auto&& allocator)
		: base(_array_table_placeholder_t(), vsm_forward(allocator))
	{
		_initialize();
	}

	explicit _array_table_impl(any_cvref_of<P> auto&& policies, any_cvref_of<A> auto&& allocator)
		: base(vsm_forward(policies), vsm_forward(allocator))
	{
		_initialize();
	}

	_array_table_impl(_array_table_impl&& other) noexcept
		requires allocators::is_propagatable_v<A>
		: base(other.m_policies, other.m_allocator)
	{
		static_assert(is_nothrow_relocatable_v<T>);
		static_assert(std::is_nothrow_copy_constructible_v<P>);
		static_assert(std::is_nothrow_copy_constructible_v<A>);

		_array_table_move<T>(*this, other);
		other._initialize();
	}

	_array_table_impl& operator=(_array_table_impl&& other) & noexcept
		requires allocators::is_propagatable_v<A>
	{
		static_assert(is_nothrow_relocatable_v<T>);
		static_assert(std::is_nothrow_copy_constructible_v<P>);
		static_assert(std::is_nothrow_copy_constructible_v<A>);

		if (vsm_likely(this != &other))
		{
			_array_table_move_assign<T>(*this, other);
			other._initialize();
		}

		return *this;
	}

	~_array_table_impl()
	{
		_array_table_destroy<T>(*this);
	}


	void shrink_to_fit();

	void swap(_array_table_impl& other) noexcept
		requires allocators::is_propagatable_v<A>
	{
		static_assert(std::is_nothrow_swappable_v<P>);
		static_assert(std::is_nothrow_swappable_v<A>);

		_array_table_swap<T, Capacity != 0>(*this, other);
	}

private:
	void vsm_always_inline _initialize() noexcept
	{
		// This is required because the elements are stored after the buckets within the same
		// allocation. This could be relaxed if there is a good way to insert the necessary padding.
		static_assert(alignof(T) <= sizeof(_array_table_bucket<I>));

		static constexpr size_t small_capacity = Capacity == 0
			? 0
			: _array_table_min_capacity_for(Capacity);

		_array_table_initialize<sizeof(T)>(*this, small_capacity);
	}
};

} // namespace vsm::detail
