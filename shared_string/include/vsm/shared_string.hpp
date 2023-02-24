#pragma once

#include <vsm/array.hpp>
#include <vsm/atomic.hpp>
#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/default_allocator.hpp>
#include <vsm/standard.hpp>
#include <vsm/type_traits.hpp>
#include <vsm/utility.hpp>
#include <vsm/vector.hpp>

#include <atomic>
#include <memory>
#include <string_view>

namespace vsm {

template<typename Char, typename Traits = std::char_traits<Char>, typename Allocator = default_allocator>
class basic_unique_string;

template<typename Char, typename Traits = std::char_traits<Char>, typename Allocator = default_allocator>
class basic_shared_string;

namespace detail::shared_string_ {

struct borrow_tag {};

template<typename Char, typename Allocator>
struct shared_string_base
{
	struct large_storage
	{
		atomic<size_t> refcount;
		size_t capacity;
		Char data[];

		explicit large_storage(size_t const capacity)
			: refcount(1)
			, capacity(capacity)
		{
		}
		
		void acquire()
		{
			vsm_verify(refcount.fetch_add(1, std::memory_order_relaxed) != 0);
		}
		
		bool release()
		{
			size_t const r = refcount.fetch_sub(1, std::memory_order_acq_rel);
			vsm_assert(r != 0);
			return r == 1;
		}
	
		bool is_unique() const
		{
			size_t const r = refcount.load(std::memory_order_acquire);
			vsm_assert(r != 0);
			return r == 1;
		}
	
		static large_storage* from_data(Char* const data)
		{
			return reinterpret_cast<large_storage*>(reinterpret_cast<std::byte*>(data) - offsetof(large_storage, data));
		}
	};

	static constexpr size_t large_storage_offset = offsetof(large_storage, data);
	static constexpr size_t large_storage_size = large_storage_offset + sizeof(Char);

	static constexpr size_t storage_size = 3 * sizeof(uintptr_t);
	static constexpr size_t max_small_size = storage_size / sizeof(Char) - 1;

	using ctrl_type = unsigned_integer_of_size<sizeof(Char)>;

	static constexpr ctrl_type large_flag                   = 0x80;
	static constexpr ctrl_type dynamic_flag                 = 0x40;

	union storage_union
	{
		struct
		{
			Char* large_data;
			size_t large_size;
			array<char, alignof(uintptr_t) - sizeof(Char)> large_padding;
			ctrl_type large_ctrl;
		};

		struct
		{
			Char small_data[max_small_size];
			ctrl_type small_ctrl;
		};
	};

	static_assert(sizeof(storage_union) == storage_size);
	static_assert(offsetof(storage_union, large_ctrl) == storage_size - sizeof(ctrl_type));
	static_assert(offsetof(storage_union, small_ctrl) == storage_size - sizeof(ctrl_type));


	class vector_allocator
	{
		static constexpr size_t data_offset = offsetof(large_storage, data);
		
		[[no_unique_address]] Allocator m_allocator;
	
	public:
		vector_allocator(any_cvref_of<Allocator> auto&& allocator)
			: m_allocator(vsm_forward(allocator))
		{
		}
	
		Allocator const& underlying() const
		{
			return m_allocator;
		}

		allocation allocate(size_t const size)
		{
			allocation allocation = m_allocator.allocate(data_offset + size * sizeof(Char));
			if (allocation.buffer != nullptr)
			{
				size_t const capacity = (allocation.size - data_offset) / sizeof(Char);
				allocation.buffer = (new (allocation.buffer) large_storage(capacity))->data;
				allocation.size = capacity;
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
				.buffer = large_storage::from_data(reinterpret_cast<Char*>(allocation.buffer)),
				.size = allocation.size + data_offset,
			};
		}
	};

	using vector_type = small_vector<Char, max_small_size + 1, vector_allocator>;


	class large_storage_deleter
	{
		Allocator m_allocator;
	
	public:
		explicit(false) large_storage_deleter(Allocator const& allocator)
			: m_allocator(allocator)
		{
		}
	
		void operator()(large_storage* const large)
		{
			vsm_assert(large->is_unique());
			m_allocator.deallocate(allocation{ large, large_storage_size + large->capacity * sizeof(Char) });
		}
	};
	
	using unique_large_storage = std::unique_ptr<large_storage, large_storage_deleter>;
};

} // namespace detail::shared_string_

#define vsm_detail detail::shared_string_

template<typename Char, typename Traits, typename Allocator>
class basic_unique_string : vsm_detail::shared_string_base<Char, Allocator>::vector_type
{
	static_assert(std::is_copy_constructible_v<Allocator>);

	using shared_string_type = basic_shared_string<Char, Traits, Allocator>;
	using string_view_type = std::basic_string_view<Char, Traits>;

	using t = vsm_detail::shared_string_base<Char, Allocator>;
	using b = typename t::vector_type;

public:
	basic_unique_string() = default;

	basic_unique_string(shared_string_type&& shared)
		: b(shared.m_allocator)
	{
		if (shared.m.large_ctrl & t::large_flag)
		{
			if (!try_adopt(vsm_move(shared)))
			{
				b::_assign(shared.m.large_data, shared.m.large_size + 1);
			}
		}
		else
		{
			b::_assign(shared.m.small_data, (t::max_small_size + 1) - shared.m.large_ctrl);
		}
	}

	explicit basic_unique_string(shared_string_type const& shared)
		: b(shared.m_allocator)
	{
		if (shared.m.large_ctrl & t::large_flag)
		{
			b::_assign(shared.m.large_data, shared.m.large_size + 1);
		}
		else
		{
			b::_assign(shared.m.small_data, (t::max_small_size + 1) - shared.m.large_ctrl);
		}
	}

	basic_unique_string(basic_unique_string&& other) = default;

	basic_unique_string& operator=(shared_string_type&& shared) &
	{
		if (shared.m.large_ctrl & t::large_flag)
		{
			if (!try_adopt(vsm_move(shared)))
			{
				b::_assign(shared.m.large_data, shared.m.large_size + 1);
			}
		}
		else
		{
			b::_assign(shared.m.small_data, (t::max_small_size + 1) - shared.m.large_ctrl);
		}
	}

	basic_unique_string& operator=(basic_unique_string&& other) & = default;


	using b::operator[];
	using b::front;

	[[nodiscard]] Char& back()
	{
		return b::operator[](size() - 1);
	}

	[[nodiscard]] Char const& back() const
	{
		return b::operator[](size() - 1);
	}

	using b::data;


	using b::begin;
	using b::cbegin;

	[[nodiscard]] typename b::iterator end()
	{
		return b::end() - 1;
	}

	[[nodiscard]] typename b::const_iterator end() const
	{
		return b::end() - 1;
	}

	[[nodiscard]] typename b::const_iterator cend() const
	{
		return b::end() - 1;
	}


	[[nodiscard]] bool empty() const
	{
		return b::size() == 1;
	}

	[[nodiscard]] size_t size() const
	{
		return b::size() - 1;
	}

	void reserve(size_t const min_capacity)
	{
		b::reserve(min_capacity + 1);
	}

	[[nodiscard]] size_t capacity() const
	{
		return b::capacity() - 1;
	}

	using b::shrink_to_fit;


	void clear()
	{
		resize(0);
	}

	void resize(size_t const size)
	{
		b::resize(size + 1);
		b::operator[](size) = Char(0);
	}

	void append(string_view_type const string)
	{
		b::insert(b::end() - 1, string.begin(), string.end());
	}


	[[nodiscard]] operator string_view_type() const
	{
		return string_view_type(data(), size());
	}

	template<convertible_from<string_view_type> String>
	[[nodiscard]] explicit operator String() const
	{
		return string_view_type(data(), size());
	}


	[[nodiscard]] friend bool operator==(basic_unique_string const& lhs, basic_unique_string const& rhs)
	{
		return string_view_type(lhs) == string_view_type(rhs);
	}

	template<std::convertible_to<string_view_type> String>
	[[nodiscard]] friend bool operator==(basic_unique_string const& lhs, String const& rhs)
	{
		return string_view_type(lhs) == string_view_type(rhs);
	}

private:
	bool try_adopt(shared_string_type&& shared)
	{
		if (shared.m.large_ctrl & t::dynamic_flag)
		{
			size_t const size = shared.m.large_size;
		
			if (auto large = shared.release_large_storage())
			{
				b::_acquire_storage(large->data, size + 1, large->capacity + 1);
				(void)large.release();
				return true;
			}
		}
		
		return false;
	}

	typename t::large_storage* release_large_storage()
	{
		vsm_assert(b::size() > t::max_small_size);
		return t::large_storage::from_data(b::_release_storage());
	}

	friend class basic_shared_string<Char, Traits, Allocator>;
};

template<typename Char, typename Traits, typename Allocator>
class basic_shared_string
{
	using unique_string_type = basic_unique_string<Char>;
	using string_view_type = std::basic_string_view<Char, Traits>;

	using t = vsm_detail::shared_string_base<Char, Allocator>;

	typename t::storage_union m;
	[[no_unique_address]] Allocator m_allocator;

public:
	constexpr basic_shared_string()
	{
		set_empty();
	}

	template<size_t Size>
	basic_shared_string(Char const(&string)[Size])
	{
		size_t const size = Traits::length(string);

		if constexpr (Size <= t::max_small_size)
		{
			set_small(string, size);
		}
		else
		{
			if (size <= t::max_small_size)
			{
				set_small(string, size);
			}
			else
			{
				set_large(string, size);
			}
		}
	}

	basic_shared_string(string_view_type const string)
	{
		size_t const size = string.size();

		if (size <= t::max_small_size)
		{
			set_small(string.data(), size);
		}
		else
		{
			set_large(string.data(), size);
		}
	}

	basic_shared_string(unique_string_type&& unique)
		: m_allocator(unique.get_allocator().underlying())
	{
		if (size_t const size = unique.size(); size > t::max_small_size)
		{
			set_large(unique.release_large_storage(), size);
		}
		else
		{
			set_small(unique.data(), size);
		}
	}

	explicit basic_shared_string(unique_string_type const& unique)
		: m_allocator(unique.get_allocator().underlying())
	{
		if (size_t const size = unique.size(); size > t::max_small_size)
		{
			set_large(unique.data(), size);
		}
		else
		{
			set_small(unique.data(), size);
		}
	}

	basic_shared_string(basic_shared_string&& other)
		: m(other.m)
	{
		other.set_empty();
	}

	basic_shared_string(basic_shared_string const& other)
		: m(other.m)
	{
		acquire();
	}

	basic_shared_string& operator=(unique_string_type&& unique) &
	{
		release();
		if (size_t const size = unique.size(); size > t::max_small_size)
		{
			set_large(unique.release_large_storage(), size);
		}
		else
		{
			set_small(unique.data(), size);
		}
		return *this;
	}

	basic_shared_string& operator=(basic_shared_string&& other) &
	{
		release();
		m = other.m;
		other.set_empty();
		return *this;
	}

	basic_shared_string& operator=(basic_shared_string const& other) &
	{
		release();
		m = other.m;
		acquire();
		return *this;
	}

	~basic_shared_string()
	{
		release();
	}


	[[nodiscard]] size_t size() const
	{
		return m.large_ctrl & t::large_flag
			? m.large_size
			: t::max_small_size - m.large_ctrl;
	}

	[[nodiscard]] bool empty() const
	{
		return m.large_ctrl & t::large_flag
			? m.large_size == 0
			: m.large_ctrl == t::max_small_size;
	}

	[[nodiscard]] Char const* data() const
	{
		return m.large_ctrl & t::large_flag
			? m.large_data
			: m.small_data;
	}

	[[nodiscard]] Char const& operator[](size_t const index) const
	{
		vsm_assert(index < size());
		return data()[index];
	}


	[[nodiscard]] Char const* begin() const
	{
		return m.large_ctrl & t::large_flag
			? m.large_data
			: m.small_data;
	}

	[[nodiscard]] Char const* end() const
	{
		return m.large_ctrl & t::large_flag
			? m.large_data + m.large_size
			: m.small_data + (t::max_small_size - m.large_ctrl);
	}


	[[nodiscard]] operator string_view_type() const
	{
		return m.large_ctrl & t::large_flag
			? string_view_type(m.large_data, m.large_size)
			: string_view_type(m.small_data, t::max_small_size - m.large_ctrl);
	}

	template<convertible_from<string_view_type> String>
	[[nodiscard]] explicit operator String() const
	{
		return m.large_ctrl & t::large_flag
			? string_view_type(m.large_data, m.large_size)
			: string_view_type(m.small_data, t::max_small_size - m.large_ctrl);
	}


	[[nodiscard]] friend bool operator==(basic_shared_string const& lhs, basic_shared_string const& rhs)
	{
		return string_view_type(lhs) == string_view_type(rhs);
	}

	template<std::convertible_to<string_view_type> String>
	[[nodiscard]] friend bool operator==(basic_shared_string const& lhs, String const& rhs)
	{
		return string_view_type(lhs) == string_view_type(rhs);
	}


	[[nodiscard]] static constexpr basic_shared_string borrow(string_view_type const string)
	{
		return basic_shared_string(string.data(), string.size(), vsm_detail::borrow_tag());
	}

private:
	explicit constexpr basic_shared_string(Char const* const data, size_t const size, vsm_detail::borrow_tag)
	{
		if (size <= t::max_small_size)
		{
			set_small(data, size);
		}
		else
		{
			set_borrow(data, size);
		}
	}

	constexpr void set_empty()
	{
		m.small_data[0] = Char(0);
		vsm_if_consteval
		{
			for (size_t i = 1; i < t::max_small_size; ++i)
			{
				m.small_data[i] = Char(0);
			}
		}
		m.small_ctrl = t::max_small_size;
	}

	constexpr void set_small(Char const* const string, size_t const size)
	{
		vsm_assume(size <= t::max_small_size);
		vsm_if_consteval
		{
			for (size_t i = 0; i < size; ++i)
			{
				m.small_data[i] = string[i];
			}
			for (size_t i = size; i < t::max_small_size; ++i)
			{
				m.small_data[i] = Char(0);
			}
		}
		else
		{
			memcpy(m.small_data, string, size * sizeof(Char));
			m.small_data[size] = Char(0);
		}
		m.small_ctrl = t::max_small_size - size;
	}

	constexpr void set_borrow(Char const* const string, size_t const size)
	{
		m.large_data = const_cast<Char*>(string);
		m.large_size = size;
		vsm_if_consteval
		{
			m.large_padding = {};
		}
		m.large_ctrl = t::large_flag;
	}

	void set_large(Char const* const string, size_t const size)
	{
		allocation const a = m_allocator.allocate(t::large_storage_size + size * sizeof(Char));

		size_t const capacity = (a.size - t::large_storage_size) / sizeof(Char);
		auto const large = new (a.buffer) typename t::large_storage(capacity);

		memcpy(large->data, string, size * sizeof(Char));
		large->data[size] = Char(0);

		set_large(large, size);
	}

	void set_large(typename t::large_storage* const large, size_t const size)
	{
		m.large_data = large->data;
		m.large_size = size;
		m.large_ctrl = t::large_flag | t::dynamic_flag;
	}

	void acquire()
	{
		if (m.large_ctrl & t::dynamic_flag)
		{
			t::large_storage::from_data(m.large_data)->acquire();
		}
	}

	void release()
	{
		if (m.large_ctrl & t::dynamic_flag)
		{
			if (auto const large = t::large_storage::from_data(m.large_data); large->release())
			{
				m_allocator.deallocate({ large, t::large_storage_size + large->capacity * sizeof(Char) });
			}
		}
	}

	typename t::unique_large_storage release_large_storage()
	{
		vsm_assert(m.large_ctrl & t::dynamic_flag);

		typename t::large_storage* large = t::large_storage::from_data(m.large_data);

		if (large->is_unique())
		{
			set_empty();
		}
		else
		{
			large = nullptr;
		}

		return typename t::unique_large_storage(large, m_allocator);
	}

	friend class basic_unique_string<Char, Traits, Allocator>;
};

#undef vsm_detail

using unique_string = basic_unique_string<char>;
using shared_string = basic_shared_string<char>;

} // namespace vsm
