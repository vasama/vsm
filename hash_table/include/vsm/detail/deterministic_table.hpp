#pragma once

#include <vsm/detail/hash_table.hpp>
#include <vsm/allocator.hpp>
#include <vsm/assert.h>
#include <vsm/insert_result.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/standard/stdexcept.hpp>
#include <vsm/type_traits.hpp>

#include <bit>

namespace vsm::detail::deterministic_table {

template<typename I>
using truncated_hash = select_t<(sizeof(I) >= sizeof(size_t)), size_t, I>;

template<typename I>
inline truncated_hash<I> truncate_hash(size_t const hash)
{
	//TODO: Consider mixing?
	return static_cast<truncated_hash<I>>(hash);
}



template<typename I>
struct bucket
{
	I index;
	truncated_hash<I> hash;

	static bucket const empty_bucket;
};

extern template bucket<uint_least16_t> const bucket<uint_least16_t>::empty_bucket;
extern template bucket<uint_least32_t> const bucket<uint_least32_t>::empty_bucket;
extern template bucket<size_t> const bucket<size_t>::empty_bucket;

template<typename I>
struct _table
{
	bucket<I>* buckets;
	I hash_mask;
	I size;
};

template<typename I, typename P>
using _table_policies = hash_table_policies_layout<_table<I>, P>;

template<typename I, typename P, typename A>
using _table_allocator = hash_table_allocator_layout<_table_policies<I, P>, A>;

template<typename I, typename P, typename A, size_t C>
using _table_storage = _table_allocator<I, P, A>;


template<typename I>
void* get_element_end(bucket<I>* const buckets, size_t const element_size, size_t const index)
{
	return reinterpret_cast<std::byte*>(buckets) - index * element_size;
}

template<typename I>
void* get_element(bucket<I>* const buckets, size_t const element_size, size_t const index)
{
	return get_element_end(buckets, element_size, index + 1);
}

inline size_t get_max_elements(size_t const hash_mask)
{
	return (hash_mask + 1) * 3 / 4;
}

template<typename I>
size_t get_bucket_distance(bucket<I> const& bucket, size_t const index, size_t const hash_mask)
{
	size_t const ideal_index = bucket.hash & hash_mask;
	return index - ideal_index & hash_mask;
}

template<typename I>
bucket<I>* get_empty_bucket()
{
	return const_cast<bucket<I>*>(&bucket<I>::empty_bucket);
}

template<typename I>
void construct(_table<I>& table)
{
	table.buckets = get_empty_bucket<I>();
	table.hash_mask = 0;
	table.size = 0;
}

template<typename T, typename I, typename P, typename A>
void destroy(_table_allocator<I, P, A>& table, size_t const element_size)
{
	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		if (table.size != 0)
		{
			auto const elements = get_element_end(table.buckets, element_size, table.size);

			std::destroy(
				reinterpret_cast<T*>(elements),
				reinterpret_cast<T*>(table.buckets));
		}
	}

	if (table.buckets != get_empty_bucket<I>())
	{
		size_t const buckets_capacity = table.hash_mask + 1;
		size_t const elements_capacity = get_max_elements(table.hash_mask);
		size_t const buckets_size = buckets_capacity * sizeof(bucket<I>);
		size_t const elements_size = elements_capacity * element_size;
		size_t const allocation_size = buckets_size + elements_size;

		void* const base = reinterpret_cast<std::byte*>(table.buckets) - elements_size;
		table.allocator.deallocate(allocation(base, allocation_size));
	}
}

template<typename I>
struct find_result
{
	void* slot;
	size_t bucket_index;
	size_t bucket_distance;
};

template<typename K, typename UserK, typename I, typename P>
find_result<I> find_for_insert(
	_table_policies<I, P> const& table,
	size_t const element_size,
	size_t const hash,
	input_t<UserK> key)
{
	P const& p = table.policies;

	bucket<I>* const buckets = table.buckets;
	size_t const hash_mask = table.hash_mask;

	size_t bucket_index = hash & hash_mask;
	size_t bucket_distance = 0;

	while (true)
	{
		bucket<I>& bucket = buckets[bucket_index];

		if (bucket.index == std::numeric_limits<I>::max() ||
			bucket_distance > get_bucket_distance(bucket, bucket_index, hash_mask))
		{
			return { nullptr, bucket_index, bucket_distance };
		}

		if (bucket.hash == truncate_hash<I>(hash))
		{
			void* const element = get_element(buckets, element_size, bucket.index);
			auto const& element_key = *std::launder(reinterpret_cast<K const*>(element));

			if (p.comparator(key, p.key_selector(element_key)))
			{
				return { element, bucket_index, bucket_distance };
			}
		}

		bucket_index = bucket_index + 1 & hash_mask;
		++bucket_distance;
	}
}

template<typename K, typename UserK, size_t ElementSize, typename I, typename P>
void* find(
	_table_policies<I, P> const& table,
	size_t const hash,
	input_t<UserK> key)
{
	return find_for_insert<K, UserK>(table, ElementSize, hash, key).slot;
}

template<typename I>
void insert_index(
	bucket<I>* const buckets,
	size_t const hash_mask,
	bucket<I> new_bucket,
	size_t bucket_index,
	size_t bucket_distance)
{
	while (true)
	{
		bucket<I>& bucket = buckets[bucket_index];

		if (bucket.index == std::numeric_limits<I>::max())
		{
			bucket = new_bucket;
			break;
		}

		size_t const distance = get_bucket_distance(bucket, bucket_index, hash_mask);

		if (bucket_distance > distance)
		{
			std::swap(bucket, new_bucket);
			bucket_distance = distance;
		}

		bucket_index = bucket_index + 1 & hash_mask;
		++bucket_distance;
	}
}

template<typename I>
void insert_index(
	bucket<I>* const buckets,
	size_t const hash_mask,
	bucket<I> const new_bucket)
{
	return insert_index(buckets, hash_mask, new_bucket, new_bucket.hash & hash_mask, 0);
}

//TODO: Doesn't use P.
template<typename I, typename P, typename A>
void resize(_table_allocator<I, P, A>& table, size_t const element_size, size_t const min_capacity)
{
	size_t const new_bucket_capacity = std::bit_ceil(min_capacity * 4 / 3);
	size_t const new_hash_mask = new_bucket_capacity - 1;
	vsm_assert(new_hash_mask <= std::numeric_limits<I>::max());
	size_t const new_element_capacity = get_max_elements(new_hash_mask);

	size_t const new_elements_size = new_element_capacity * element_size;
	size_t const new_buckets_size = new_bucket_capacity * sizeof(bucket<I>);
	size_t const new_allocation_size = new_elements_size + new_buckets_size;

	auto const allocation = table.allocator.allocate(new_allocation_size);
	if (allocation.buffer == nullptr)
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}

	auto const new_elements = reinterpret_cast<std::byte*>(allocation.buffer);
	auto const new_buckets = reinterpret_cast<bucket<I>*>(new_elements + new_elements_size);
	std::memset(new_buckets, 0xff, new_buckets_size);

	auto const old_buckets = table.buckets;
	size_t const old_hash_mask = table.hash_mask;
	size_t const old_bucket_capacity = old_hash_mask + 1;
	size_t const old_element_capacity = get_max_elements(old_hash_mask);

	for (size_t i = 0; i < old_bucket_capacity; ++i)
	{
		bucket<I> const& old_bucket = old_buckets[i];
		if (old_bucket.index != std::numeric_limits<I>::max())
		{
			insert_index(new_buckets, new_hash_mask, old_bucket);
		}
	}

	//TODO: Do a proper type-aware relocate instead.
	if (table.size != 0)
	{
		std::memcpy(
			new_elements + (new_element_capacity - table.size) * element_size,
			get_element_end(old_buckets, element_size, table.size),
			table.size * element_size);
	}

	size_t const old_elements_size = old_element_capacity * element_size;
	size_t const old_buckets_size = old_bucket_capacity * sizeof(bucket<I>);
	size_t const old_allocation_size = old_elements_size + old_buckets_size;

	if (old_buckets != get_empty_bucket<I>())
	{
		table.allocator.deallocate(vsm::allocation
		{
			get_element_end(old_buckets, element_size, old_element_capacity),
			old_allocation_size,
		});
	}

	table.buckets = new_buckets;
	table.hash_mask = static_cast<I>(new_hash_mask);
}

template<typename K, typename UserK, typename I, typename P, typename A>
insert_result2<void*> insert(
	_table_allocator<I, P, A>& table,
	size_t const element_size,
	size_t const hash,
	input_t<UserK> key)
{
	auto r = find_for_insert<K, UserK>(table, element_size, hash, key);

	if (r.slot != nullptr)
	{
		return { r.slot, false };
	}

	if (table.size == std::numeric_limits<I>::max())
	{
		vsm_except_throw_or_terminate(
			std::length_error("The max size of the table has been reached."));
	}

	if (table.size == get_max_elements(table.hash_mask))
	{
		resize(table, element_size, table.hash_mask + 2);
		r.bucket_index = hash & table.hash_mask;
		r.bucket_distance = 0;
	}

	I const element_index = table.size++;

	insert_index(
		table.buckets,
		table.hash_mask,
		{ element_index, truncate_hash<I>(hash) },
		r.bucket_index,
		r.bucket_distance);

	return { get_element(table.buckets, element_size, element_index), true };
}

template<typename I>
void move_construct(_table<I>& dst, _table<I>& src)
{
	dst = src;
	construct(src);
}

template<typename DestroyT, size_t ElementSize, typename I, typename P, typename A>
void move_assign(
	_table_allocator<I, P, A>& dst,
	_table_allocator<I, P, A>& src,
	size_t const element_size)
{
	if (&dst == &src)
	{
		return;
	}

	destroy<DestroyT>(dst, element_size);
	dst = src;
	construct(src);
}

template<typename T>
class iterator
{
	T* m_element_end;

public:
	using value_type = T;
	using difference_type = ptrdiff_t;

	iterator() = default;

	explicit iterator(T* const element_end)
		: m_element_end(element_end)
	{
	}

	[[nodiscard]] T& operator*() const
	{
		return *std::launder(m_element_end - 1);
	}

	[[nodiscard]] T* operator->() const
	{
		return std::launder(m_element_end - 1);
	}

	iterator& operator++() &
	{
		--m_element_end;
		return *this;
	}

	[[nodiscard]] iterator operator++(int) &
	{
		auto it = *this;
		--m_element_end;
		return it;
	}

	friend auto operator<=>(iterator const&, iterator const&) = default;
};

template<
	typename T,
	typename Key,
	typename I,
	typename Policies,
	typename Allocator,
	size_t Capacity>
class table
{
#	pragma push_macro("destroy_type")
#	define destroy_type select_t<std::is_trivially_destructible_v<T>, std::byte, T>

	_table_storage<I, Policies, Allocator, Capacity> m;

public:
	static constexpr bool is_ordered = true;

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
	using iterator                      = deterministic_table::iterator<T>;
	using const_iterator                = deterministic_table::iterator<T const>;
	using single_iterator               = deterministic_table::iterator<T>;
	using const_single_iterator         = deterministic_table::iterator<T const>;
	using insert_result                 = vsm::insert_result2<iterator>;


	table()
	{
		construct(m);
	}

	explicit table(any_cvref_of<Allocator> auto&& allocator)
		: m(vsm_forward(allocator))
	{
		construct(m);
	}

	table(table&& other) noexcept
		: m(vsm_as_const(other.m.allocator), vsm_as_const(other.m.policies))
	{
		move_construct(m, other.m);
	}

	table& operator=(table&& other) & noexcept
	{
		move_assign<destroy_type>(m, other.m, sizeof(T));
		return *this;
	}

	~table()
	{
		destroy<destroy_type>(m, sizeof(T));
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
		return get_max_elements(m.hash_mask);
	}

	void reserve(size_t const min_capacity);
	void shrink_to_fit();


	template<typename K>
	[[nodiscard]] iterator find(K const& key)
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		auto const element = deterministic_table::find<Key, user_key_type, sizeof(T)>(
			m,
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k);

		return iterator(static_cast<T*>(element) + 1);
	}

	template<typename K>
	[[nodiscard]] const_iterator find(K const& key) const
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		auto const element = deterministic_table::find<Key, user_key_type, sizeof(T)>(
			m,
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k);

		return const_iterator(static_cast<T const*>(element) + 1);
	}

	template<typename K>
	[[nodiscard]] insert_result insert(K const& key)
	{
		decltype(auto) k = get_lookup_key<Key>(m.policies.key_selector, key);
		decltype(auto) n_k = normalize_key(k);
		using user_key_type = remove_ref_t<decltype(n_k)>;

		auto const r = deterministic_table::insert<Key, user_key_type>(
			m,
			sizeof(T),
			static_cast<Policies const&>(m.policies).hasher(n_k),
			n_k);

		return { iterator(static_cast<T*>(r.iterator) + 1), r.inserted };
	}


	template<typename K>
	[[nodiscard]] size_t erase(K const& key);


	[[nodiscard]] iterator begin()
	{
		return iterator(reinterpret_cast<T*>(m.buckets));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(reinterpret_cast<T*>(m.buckets));
	}

	[[nodiscard]] iterator end()
	{
		return iterator(static_cast<T*>(get_element_end(m.buckets, sizeof(T), m.size)));
	}

	[[nodiscard]] const_iterator end() const
	{
		return const_iterator(static_cast<T const*>(get_element_end(m.buckets, sizeof(T), m.size)));
	}

#	pragma pop_macro("destroy_type")
};

} // namespace vsm::detail::deterministic_table
