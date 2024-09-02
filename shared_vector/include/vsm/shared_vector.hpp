#pragma once

#include <vsm/atomic.hpp>
#include <vsm/default_allocator.hpp>
#include <vsm/utility.hpp>
#include <vsm/vector.hpp>

#include <span>

namespace vsm {

template<typename T, allocator Allocator = default_allocator>
class unique_vector;

template<typename T, allocator Allocator = default_allocator>
class shared_vector;

namespace detail {

template<typename T, typename Allocator>
struct _shared_vector
{
	struct storage
	{
		using value_type = T;
		using allocator_type = Allocator;

		atomic<size_t> refcount;
		vsm_no_unique_address Allocator allocator;

		size_t size;
		size_t capacity;

		T data[];


		explicit storage(any_cvref_of<Allocator> auto&& allocator)
			: refcount(1)
			, allocator(vsm_forward(allocator))
			, data{}
		{
		}

		storage(storage const&) = delete;
		storage& operator=(storage const&) = delete;

		~storage() = default;
		~storage() requires (!std::is_trivially_destructible_v<T>)
		{
		}


		static storage* from_data(T* const data)
		{
			return reinterpret_cast<storage*>(
				reinterpret_cast<std::byte*>(data) - offsetof(storage, data));
		}
	};

	class storage_allocator
	{
		static constexpr size_t data_offset = offsetof(storage, data);

		vsm_no_unique_address Allocator m_allocator;

	public:
		storage_allocator() = default;

		storage_allocator(any_cvref_of<Allocator> auto&& allocator)
			: m_allocator(vsm_forward(allocator))
		{
		}

		allocation allocate(size_t const size) const
		{
			allocation allocation = m_allocator.allocate(data_offset + size);
			if (allocation.buffer != nullptr)
			{
				allocation.buffer = (new (allocation.buffer) storage(m_allocator))->data;
				allocation.size = allocation.size - data_offset;
			}
			return allocation;
		}

		void deallocate(allocation const allocation) const
		{
			m_allocator.deallocate(get_block(allocation));
		}

		size_t resize(allocation const allocation, size_t const min_size) const
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
				.buffer = storage::from_data(reinterpret_cast<T*>(allocation.buffer)),
				.size = allocation.size + data_offset,
			};
		}

		friend class unique_vector<T, Allocator>;
	};

	using vector_type = vector<T, storage_allocator>;
};

} // namespace detail

template<typename T, allocator Allocator>
class unique_vector : detail::_shared_vector<T, Allocator>::vector_type
{
	using base = detail::_shared_vector<T, Allocator>::vector_type;

	using storage_type = typename detail::_shared_vector<T, Allocator>::storage;

	using shared_vector_type = shared_vector<T, Allocator>;

public:
	using base::base;

	unique_vector(shared_vector_type&& shared)
		: unique_vector(vsm_move(shared), Allocator())
	{
	}

	explicit unique_vector(any_cvref_of<shared_vector_type> auto&& shared, any_cvref_of<Allocator> auto&& allocator = Allocator())
		: unique_vector(vsm_forward(shared).get_unique_ref(vsm_forward(allocator)), vsm_forward(allocator))
	{
	}

	unique_vector& operator=(unique_vector&&) & = default;
	unique_vector& operator=(unique_vector const&) & = default;


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
	using base::_push_back_default;
	using base::_push_back_uninitialized;
	using base::emplace_back;
	using base::pop_back;
	using base::_pop_back_n;
	using base::_pop_back_uninitialized;
	using base::_pop_back_value;
	using base::resize;
	using base::_resize_default;
	using base::swap;

private:
	explicit unique_vector(storage_type* const storage, any_cvref_of<Allocator> auto&& allocator)
		: base(storage != nullptr ? vsm_move(storage->allocator) : Allocator(vsm_forward(allocator)))
	{
		if (storage != nullptr)
		{
			base::_acquire_storage(
				storage->data,
				storage->size,
				storage->capacity);
		}
	}

	storage_type* get_unique_ref()
	{
		if (capacity() == 0)
		{
			return nullptr;
		}

		size_t const size = base::size();
		size_t const capacity = base::capacity();

		T* const data = base::_release_storage();
		storage_type* const storage = storage_type::from_data(data);

		storage->size = size;
		storage->capacity = capacity;

		return storage;
	}

	friend shared_vector<T, Allocator>;
};

template<typename T, allocator Allocator>
class shared_vector : std::span<T const>
{
	using base = std::span<T const>;

	using storage_type = typename detail::_shared_vector<T, Allocator>::storage;
	static constexpr size_t data_offset = offsetof(storage_type, data);

	using unique_vector_type = unique_vector<T, Allocator>;

	storage_type* m_storage = nullptr;

public:
	shared_vector() = default;

	template<std::ranges::input_range R>
	explicit shared_vector(std::from_range_t, R&& range, Allocator const& allocator = Allocator())
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
		: shared_vector(unique_vector_type(std::from_range, range, allocator))
	{
	}

	template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
	explicit shared_vector(Iterator const begin, Sentinel const end, Allocator const& allocator = Allocator())
		requires std::convertible_to<std::iter_reference_t<Iterator>, T>
		: shared_vector(unique_vector_type(begin, end, allocator))
	{
	}

	shared_vector(unique_vector_type&& unique) noexcept
		: shared_vector(unique.get_unique_ref())
	{
	}

	shared_vector(shared_vector&& other) noexcept
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

	shared_vector& operator=(shared_vector&& other) & noexcept
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

	[[nodiscard]] T& at(size_t const index)
	{
		if (index < base::size())
		{
			vsm_except_throw_or_terminate(std::out_of_range("shared_vector index out of range"));
		}
		else
		{
			return base::operator[](index);
		}
	}

	[[nodiscard]] T const& at(size_t const index) const
	{
		if (index < base::size())
		{
			vsm_except_throw_or_terminate(std::out_of_range("shared_vector index out of range"));
		}
		else
		{
			return base::operator[](index);
		}
	}

	using base::operator[];
	using base::front;
	using base::back;
	using base::data;

	using base::empty;
	using base::size;

	[[nodiscard]] size_t capacity() const
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
		return empty()
			? nullptr
			: create_storage(vsm_forward(allocator));
	}

	storage_type* create_storage(any_cvref_of<Allocator> auto&& allocator) const
	{
		size_t const size = base::size();
		allocation const allocation = allocator.allocate(data_offset + size * sizeof(T));
		size_t const capacity = (allocation.size - data_offset) / sizeof(T);

		storage_type* const storage = new (allocation.buffer) storage_type(vsm_forward(allocator));

		storage->size = size;
		storage->capacity = capacity;

		std::uninitialized_copy_n(
			base::data(),
			size,
			storage->data);

		return storage;
	}

	static bool is_unique_ref(size_t const refcount)
	{
		vsm_assert(refcount > 0);
		return refcount == 1;
	}

	static void acquire_ref(storage_type* const storage)
	{
		(void)storage->refcount.fetch_add(1, std::memory_order_relaxed);
	}

	static void release_ref(storage_type* const storage)
	{
		if (is_unique_ref(storage->refcount.fetch_sub(1, std::memory_order_acq_rel)))
		{
			Allocator allocator = vsm_move(storage->allocator);

			std::destroy_n(storage->data, storage->size);
			storage->~storage_type();

			allocator.deallocate(allocation
			{
				storage,
				data_offset + storage->capacity * sizeof(T),
			});
		}
	}

	friend unique_vector<T, Allocator>;
};

} // namespace vsm
