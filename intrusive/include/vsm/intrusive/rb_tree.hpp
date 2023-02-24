#pragma once

#include <vsm/intrusive/link.hpp>
#include <vsm/intrusive/list.hpp>

#include <vsm/insert_result.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/utility.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {
namespace detail {

#define vsm_rb_hook_from_children(children) \
	static_cast<rb_tree_::hook*>(reinterpret_cast<rb_tree_::hook_data*>(children))

struct _rb
{
	template<typename T>
	using ptr = incomplete_tag_ptr<T, bool, 1>;

	struct hook
	{
		hook* children[2];
	
		// Pointer to hook::children[0] of a parent hook or to _rb::m_root,
		// with the tag bit indicating node colour.
		ptr<hook*> parent;
	};

	template<typename T, typename Tag>
	class iterator
	{
		hook** m_children;

	public:
		using difference_type = ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;


		iterator() = default;

		iterator(hook** const children)
			: m_children(children)
		{
		}


		[[nodiscard]] T& operator*() const
		{
			return *links::get_elem<T, Tag>(reinterpret_cast<hook*>(m_children));
		}

		[[nodiscard]] T* operator->() const
		{
			return links::get_elem<T, Tag>(reinterpret_cast<hook*>(m_children));
		}


		iterator& operator++() &
		{
			m_children = iterator_advance(m_children, 0);
			return *this;
		}

		[[nodiscard]] iterator operator++(int) &
		{
			iterator result = *this;
			m_children = iterator_advance(m_children, 0);
			return result;
		}

		iterator& operator--() &
		{
			m_children = iterator_advance(m_children, 1);
			return *this;
		}

		[[nodiscard]] iterator operator--(int) &
		{
			iterator result = *this;
			m_children = iterator_advance(m_children, 1);
			return result;
		}


		[[nodiscard]] bool operator==(iterator const&) const = default;
	};


	hook* m_root = nullptr;
	size_t m_size = 0;


	_rb() = default;

	_rb(_rb&& other)
		: m_root(other.m_root)
		, m_size(other.m_size)
	{
		other.m_root = nullptr;
		other.m_size = 0;
	}

	_rb& operator=(_rb&& other) & noexcept
	{
		if (m_root != nullptr)
		{
			clear();
		}
		m_root = other.m_root;
		m_size = other.m_size;
		other.m_root = nullptr;
		other.m_size = 0;
		return *this;
	}

	~_rb()
	{
		if (m_root != nullptr)
		{
			clear();
		}
	}


	struct find_result
	{
		hook* node;
		ptr<hook* const> parent;
	};

	void insert(hook* node, ptr<hook*> parent_and_side);
	void remove(hook* node);
	void clear();
	_list::hook* flatten();

	friend void swap(_rb& lhs, _rb& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_root, rhs.m_root);
		swap(lhs.m_size, rhs.m_size);
	}


	static hook** iterator_begin(hook** root);
	static hook** iterator_advance(hook** children, bool l);
};

} // namespace detail

template<typename Tag>
using basic_rb_tree_link = basic_link<3, Tag>;

using rb_tree_link = basic_rb_tree_link<void>;

template<
	typename T,
	key_selector<T> KeySelector = identity_key_selector,
	typename Comparator = std::compare_three_way>
class rb_tree : detail::_rb
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;

	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

	using       iterator = _rb::iterator<      element_type, tag_type>;
	using const_iterator = _rb::iterator<const element_type, tag_type>;

	using insert_result = vsm::insert_result<element_type>;

private:
	vsm_no_unique_address KeySelector m_key_selector;
	vsm_no_unique_address Comparator m_comparator;

public:
	rb_tree() = default;

	explicit constexpr rb_tree(KeySelector key_selector)
		: m_key_selector(vsm_move(key_selector))
	{
	}

	explicit constexpr rb_tree(Comparator comparator)
		: m_comparator(vsm_move(comparator))
	{
	}

	explicit constexpr rb_tree(KeySelector key_selector, Comparator comparator)
		: m_key_selector(vsm_move(key_selector))
		, m_comparator(vsm_move(comparator))
	{
	}

	rb_tree& operator=(rb_tree&&) & = default;


	/// @return Size of the set.
	[[nodiscard]] size_t size() const
	{
		return m_size;
	}

	/// @return True if the set is empty.
	[[nodiscard]] bool empty() const
	{
		return m_size == 0;
	}


	/// @brief Find element by homogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element, or null if not found.
	[[nodiscard]] element_type* find(key_type const& key)
		noexcept(noexcept(find_internal(key)))
	{
		hook* const node = find_internal(key).node;

		return node == nullptr
			? nullptr
			: get_elem(node);
	}

	/// @brief Find element by homogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element, or null if not found.
	[[nodiscard]] element_type const* find(const key_type& key) const
		noexcept(noexcept(find_internal(key)))
	{
		hook const* const node = find_internal(key).node;

		return node == nullptr
			? nullptr
			: get_elem(node);
	}

	/// @brief Find element by heterogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element or null.
	template<typename Key>
	[[nodiscard]] element_type* find_equivalent(Key const& key)
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		hook* const node = find_internal(key).node;

		return node == nullptr
			? nullptr
			: get_elem(node);
	}

	/// @brief Find element by heterogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element or null.
	template<typename Key>
	[[nodiscard]] element_type const* find_equivalent(Key const& key) const
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		hook const* const node = find_internal(key).node;

		return node == nullptr
			? nullptr
			: get_elem(node);
	}


	/// @brief Insert new element into the tree.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	insert_result insert(element_type* const element)
	{
		auto const r = find_internal(m_key_selector(*element));
		if (r.node != nullptr)
		{
			return { get_elem(r.node), false };
		}
		_rb::insert(
			detail::links::construct<hook, tag_type>(element),
			const_pointer_cast<ptr<hook*>>(r.parent));
		return { element, true };
	}

	/// @brief Remove an element from the tree.
	/// @param element Element to be removed.
	/// @pre @p element is part of this tree.
	void remove(element_type* const element)
	{
		_rb::remove(get_hook(element));
	}

	/// @brief Remove all elements from the tree.
	using _rb::clear;

	/// @brief Flatten the tree into a linked list using an in-order traversal.
	[[nodiscard]] list<T> flatten()
	{
		size_t const size = m_size;
		return list<T>(_rb::flatten(), size);
	}


	/// @brief Create an iterator referring to an element.
	/// @pram element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] iterator make_iterator(element_type* const element)
	{
		return iterator(get_hook(element));
	}

	/// @brief Create an iterator referring to an element.
	/// @pram element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] const_iterator make_iterator(element_type const* const element) const
	{
		return const_iterator(get_hook(element));
	}


	[[nodiscard]] iterator begin()
	{
		return iterator(iterator_begin(&m_root));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(iterator_begin(const_cast<hook**>(&m_root)));
	}

	[[nodiscard]] iterator end()
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator end() const
	{
		return const_iterator(const_cast<hook**>(&m_root));
	}


	friend void swap(rb_tree& lhs, rb_tree& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_key_selector, rhs.m_key_selector);
		swap(lhs.m_comparator, rhs.m_comparator);
	}

private:
	[[nodiscard]] static auto* get_hook(auto* const element)
	{
		return detail::links::get_hook<hook, tag_type>(element);
	}

	[[nodiscard]] static auto* get_elem(auto* const hook)
	{
		return detail::links::get_elem<element_type, tag_type>(hook);
	}

	template<typename Key>
	find_result find_internal(Key const& key) const
		noexcept(noexcept(m_comparator(key, std::declval<key_type const&>())))
	{
		hook* const* parent = &m_root;
		bool l = 0;

		while (parent[l] != nullptr)
		{
			hook* const child = parent[l];

			auto const ordering = m_comparator(
				key,
				m_key_selector(*get_elem(child)));

			if (ordering == 0)
			{
				return { child, { parent, l } };
			}

			parent = child->children;
			l = ordering > 0;
		}

		return find_result{ nullptr, { parent, l } };
	}
};

} // namespace vsm::intrusive
