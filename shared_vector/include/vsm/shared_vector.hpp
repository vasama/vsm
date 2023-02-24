#pragma once

#include <vsm/vector2.hpp>

#include <span>

namespace vsm {

template<typename T, typename Allocator = default_allocator>
class unique_vector;

template<typename T, typename Allocator = default_allocator>
class shared_vector;

namespace detail::shared_vector_ {

template<typename T, typename Allocator>
struct storage
{
	using value_type = T;
	using allocator_type = Allocator;

	atomic<size_t> refcount;
	[[no_unique_address]] Allocator allocator;
	
	size_t size;
	size_t capacity;
	
	T data[];
	
	
	explicit storage(any_cvref_of<Allocator> auto&& allocator)
		: refcount(1)
		, allocator(vsm_forward(allocator))
	{
	}
	

	static storage* from_data(T* const data)
	{
		return reinterpret_cast<storage*>(
			reinterpret_cast<std::byte*>(data) - offsetof(storage, data));
	}
};

template<typename T, typename Allocator>
class storage_allocator
{
	using storage_type = storage<T, Allocator>;
	static constexpr size_t data_offset = offsetof(storage_type, data);
	
	[[no_unique_address]] Allocator m_allocator;

public:
	storage_allocator(any_cvref_of<Allocator> auto&& allocator)
		: m_allocator(vsm_forward(allocator))
	{
	}

	allocation allocate(size_t const size)
	{
		allocation allocation = m_allocator.allocate(data_offset + size * sizeof(T));
		if (allocation.buffer != nullptr)
		{
			allocation.buffer = new (allocation.buffer) storage_type(
				static_cast<Allocator const&>(m_allocator)
			)->data;
			allocation.size = (allocation.size - data_offset) / sizeof(T);
		}
		return allocation;
	}
	
	void deallocate(allocation const allocation)
	{
		m_allocator.deallocate(get_block(allocation));
	}
	
	size_t resize(allocation const allocation, size_t const min_size)
	{
		size_t new_size = m_allocator.resize(get_block(allocation), data_offset + min_size);
		if (new_size != 0)
		{
			new_size -= data_offset;
		}
		return new_size;
	}
	
private:
	static allocation get_block(allocation const allocation)
	{
		return vsm::allocation
		{
			.buffer = storage_type::from_data(reinterpret_cast<T*>(allocation.buffer)),
			.size = allocation.size + data_offset,
		};
	}

	friend class unique_vector<T, Allocator>;
};

} // namespace detail::shared_vector_

template<typename T, allocator Allocator>
class unique_vector : vector<T, storage_allocator<T, Allocator>>
{
	using base = vector<T, storage_allocator<T, Allocator>>;

	using storage_type = storage<T, Allocator>;

	using shared_vector_type = shared_vector<T, Allocator>;

public:
	unique_vector(shared_vector_type&& shared)
		: unique_vector(vsm_move(shared), Allocator())
	{
	}

	explicit unique_vector(any_cvref_of<shared_vector_type> auto&& shared, any_cvref_of<Allocator> auto&& allocator = Allocator())
		: unique_vector(vsm_forward(shared).get_unique_ref(vsm_forward(allocator)), vsm_forward(allocator))
	{
	}
	
	unique_vector& operator(unique_vector&&) & = default;
	unique_vector& operator(unique_vector const&) & = default;
	
	
	using base::at;
	using base::operator[];
	using base::front;
	using base::back;
	using base::data;

	using base::begin;
	using base::cbegin;
	using base::end;
	using base::cend;
	using base::rbegin;
	using base::crbegin;
	using base::rend;
	using base::crend;
	
	using base::empty;
	using base::size;
	using base::reserve;
	using base::capacity;
	using base::shrink_to_fit;
	
	using base::clear;
	using base::insert;
	using base::emplace;
	using base::erase;
	using base::_erase_unstable;
	using base::push_back;
	using base::_push_back_range;
	using base::_push_back_n;
	using base::_push_back;
	using base::_push_back_default;
	using base::_push_back_uninitialized;
	using base::emplace_back;
	using base::pop_back;
	using base::_pop_back_n;
	using base::_pop_back_uninitialized;
	using base::_pop_back_value;
	using base::resize;
	using base::_resize_default;
	using base::_resize_uninitialized;
	using base::swap;

private:
	explicit unique_vector(storage_type* const storage, any_cvref_of<Allocator> auto&& allocator)
		: base(storage != nullptr ? vsm_move(storage->allocator) : Allocator(vsm_forward(allocator)))
	{
		if (storage != nullptr)
		{
			base::_acquire_storage({ storage->data, storage->size, storage->capacity });
		}
	}

	storage_type* get_unique_ref()
	{
		if (capacity() == 0)
		{
			return nullptr;
		}

		auto const buffer = base::_release_storage();
		auto const storage = storage_type::from_data(buffer.data);
		storage->size = buffer.size;
		storage->capacity = buffer.capacity;

		return storage;
	}

	friend class shared_vector<T, Allocator>;
};

template<typename T, allocator Allocator>
class shared_vector : std::span<T const>
{
	using base = std::span<T const>;
	
	using storage_type = storage<T, Allocator>;
	static constexpr size_t data_offset = offsetof(storage_type, data);

	using unique_vector_type = unique_vector<T, Allocator>;

	storage_type* m_storage;

public:
	shared_vector(unique_vector_type&& unique)
		: shared_vector(unique.get_unique_ref())
	{
	}

	shared_vector(shared_vector&& other)
		: base(std::exchange(static_cast<base&>(other), base()))
		, m_storage(std::exchange(other.m_storage, nullptr))
	{
	}
	
	shared_vector(shared_vector const& other)
		: base(static_cast<base const&>(other))
		, m_storage(other.m_storage)
	{
		if (m_storage != nullptr)
		{
			acquire_ref(m_storage);
		}
	}
	
	shared_vector& operator=(shared_vector&& other) &
	{
		if (m_storage != nullptr)
		{
			release_ref(m_storage);
		}
		
		static_cast<base&>(*this) = std::exchange(static_cast<base&>(other), base());
		m_storage = std::exchange(other.m_storage, nullptr);
		
		return *this;
	}
		
	shared_vector& operator=(shared_vector const& other) &
	{
		if (m_storage != nullptr)
		{
			release_ref(m_storage);
		}
		
		static_cast<base>(*this) = static_cast<base const&>(other);
		m_storage = other.m_storage;
		
		if (m_storage != nullptr)
		{
			acquire_ref(m_storage);
		}
		
		return *this;
	}
		
	~shared_vector()
	{
		if (m_storage != nullptr)
		{
			release_ref(m_storage);
		}
	}


	using base::begin;
	using base::end;
	using base::rbegin;
	using base::rend;

	using base::at;
	using base::operator[];
	using base::front;
	using base::back;
	using base::data;

	using base::empty;
	using base::size;
	
	size_t capacity() const
	{
		return m_storage != nullptr ? m_storage->capacity : size();
	}


	void realize(any_cvref_of<Allocator> auto&& allocator = {})
	{
		if (m_storage == nullptr && !empty())
		{
			m_storage = create_storage(vsm_forward(allocator));
			static_cast<base&>(*this) = base(m_storage->data, m_storage->size);
		}
	}

	[[nodiscard]] static shared_vector borrow(std::span<T const> const data)
	{
		return shared_vector(nullptr, data);
	}

private:
	explicit shared_vector(storage_type* const storage)
		: base(storage != nullptr ? base(storage->data, storage->size) : base())
		, m_storage(storage)
	{
	}
	
	explicit shared_vector(storage_type* const storage, base const data)
		: base(data)
		, m_storage(storage)
	{
	}

	storage_type* get_unique_ref(any_cvref_of<Allocator> auto&& allocator)
	{
		if (m_storage != nullptr)
		{
			if (is_unique_ref(m_storage->refcount.load(std::memory_order_acquire)))
			{
				static_cast<base&>(*this) = base();
				return std::exchange(m_storage, nullptr);
			}
		}
		
		return static_cast<shared_vector const&>(*this).get_unique_ref(vsm_forward(allocator));
	}
	
	storage_type* get_unique_ref(any_cvref_of<Allocator> auto&& allocator) const
	{
		return empty() ? nullptr : create_storage(vsm_forward(allocator));
	}
	
	storage_type* create_storage(any_cvref_of<Allocator> auto&& allocator) const
	{
		allocation const allocation = allocator.allocate(data_offset + size());
		size_t const capacity = (allocation.size - data_offset) / sizeof(T);
		
		storage_type* const storage = new storage_type(allocation.buffer)
		{
			.refcount = 1,
			.allocator = vsm_forward(allocator),
			.size = size(),
			.capacity = capacity,
		};
		
		std::uninitialized_copy_construct_n(data(), size(), storage->data);
		
		return storage;
	}
	
	static bool is_unique_ref(size_t const refcount)
	{
		vsm_assert(refcount > 0);
		return refcount == 1;
	}

	static void acquire_ref(storage_type* const storage)
	{
		storage->refcount.fetch_add(1, std::memory_order_relaxed);
	}
	
	static void release_ref(storage_type* const storage)
	{
		if (is_unique_ref(storage->refcount.fetch_sub(1, std::memory_order_acq_rel)))
		{
			Allocator allocator = vsm_move(storage->allocator);
		
			std::destroy_n(data(), size());
			storage->~storage_type();
			
			allocator.deallocate(allocation{ storage, data_offset + size_bytes() });
		}
	}
	
	friend class unique_string<T, Allocator>;
};

} // namespace vsm
