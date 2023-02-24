#pragma once

#include <vsm/intrusive/link.hpp>
#include <vsm/intrusive/list.hpp>

#include <vsm/insert_result.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/linear.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/utility.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {

using rb_tree_link = link<3>;

namespace detail::rb_tree_ {

#define vsm_rb_hook(element, ...) \
	(reinterpret_cast<rb_tree_::hook __VA_ARGS__*>(static_cast<rb_tree_link __VA_ARGS__*>(element)))

#define vsm_rb_elem(hook, ...) \
	(static_cast<T __VA_ARGS__*>(reinterpret_cast<rb_tree_link __VA_ARGS__*>(hook)))

#define vsm_rb_hook_from_children(children) \
	static_cast<rb_tree_::hook*>(reinterpret_cast<rb_tree_::hook_data*>(children))


template<typename T>
using ptr = incomplete_tag_ptr<T, bool, 1>;

struct hook;

struct hook_data
{
	hook* children[2];

	// Pointer to hook::children[0] of a parent hook or to base::m_root,
	// with the tag bit indicating node color.
	ptr<hook*> parent;
};

struct hook : link_base, hook_data {};

struct base : link_container
{
	linear<hook*> m_root;
	linear<size_t> m_size;


	base(base&&) = default;

	base& operator=(base&& src) & noexcept
	{
		if (m_root.value != nullptr)
		{
			clear();
		}
		m_root = vsm_move(src.m_root);
		m_size = vsm_move(src.m_size);
		return *this;
	}

	~base()
	{
		if (m_root.value != nullptr)
		{
			clear();
		}
	}


	struct find_result
	{
		hook* node;
		ptr<const ptr<hook>> parent;
	};

	void insert(hook* node, ptr<hook*> parent_and_side) noexcept;
	void remove(hook* node) noexcept;
	void clear() noexcept;
	list_::hook* flatten() noexcept;

	friend void swap(base& lhs, base& rhs) noexcept
	{
		using std::swap;
		swap(
			static_cast<link_container&>(lhs),
			static_cast<link_container&>(rhs));
		swap(lhs.m_root, rhs.m_root);
		swap(lhs.m_size, rhs.m_size);
	}
};


template<typename T>
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


	[[nodiscard]] T& operator*() const noexcept
	{
		return *vsm_rb_elem(vsm_rb_hook_from_children(m_children));
	}

	[[nodiscard]] T* operator->() const noexcept
	{
		return vsm_rb_elem(vsm_rb_hook_from_children(m_children));
	}


	iterator& operator++() & noexcept
	{
		m_children = iterator_advance(m_children, 0);
		return *this;
	}

	[[nodiscard]] iterator operator++(int) & noexcept
	{
		iterator result = *this;
		m_children = iterator_advance(m_children, 0);
		return result;
	}

	iterator& operator--() & noexcept
	{
		m_children = iterator_advance(m_children, 1);
		return *this;
	}

	[[nodiscard]] iterator operator--(int) & noexcept
	{
		iterator result = *this;
		m_children = iterator_advance(m_children, 1);
		return result;
	}


	[[nodiscard]] bool operator==(iterator const&) const = default;
};


template<std::derived_from<rb_tree_link> T,
	key_selector<T> KeySelector = identity_key_selector,
	typename Comparator = std::three_way_compare>
class rb_tree : base
{
	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

	[[no_unique_address]] KeySelector m_key_selector;
	[[no_unique_address]] Comparator m_comparator;

public:
	using element_type = T;

	using       iterator = rb_tree_::iterator<      T>;
	using const_iterator = rb_tree_::iterator<const T>;

	using insert_result = vsm::insert_result<T>;


	rb_tree() = default;

	explicit constexpr rb_tree(KeySelector key_selector) noexcept
		: m_key_selector(vsm_move(key_selector))
	{
	}

	explicit constexpr rb_tree(Comparator comparator) noexcept
		: m_comparator(vsm_move(comparator))
	{
	}

	explicit constexpr rb_tree(KeySelector key_selector, Comparator comparator) noexcept
		: m_key_selector(vsm_move(key_selector))
		, m_comparator(vsm_move(comparator))
	{
	}

	rb_tree& operator=(rb_tree&&) & = default;


	/// @return Size of the set.
	[[nodiscard]] size_t size() const noexcept
	{
		return m_size.value;
	}

	/// @return True if the set is empty.
	[[nodiscard]] bool empty() const noexcept
	{
		return m_size.value == 0;
	}


	/// @brief Find element by homogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element, or null if not found.
	[[nodiscard]] T* find(key_type const& key)
		noexcept(noexcept(find_internal(key)))
	{
		return vsm_rb_elem(find_internal(key).node);
	}

	/// @brief Find element by homogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element, or null if not found.
	[[nodiscard]] T const* find(const KeyType& key) const
		noexcept(noexcept(find_internal(key)))
	{
		return vsm_rb_elem(find_internal(key).node);
	}

	/// @brief Find element by heterogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element or null.
	template<typename Key>
	[[nodiscard]] T* find_equivalent(Key const& key)
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		return vsm_rb_elem(find_internal(key).node);
	}

	/// @brief Find element by heterogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element or null.
	template<typename Key>
	[[nodiscard]] T const* find_equivalent(Key const& key) const
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		return vsm_rb_elem(find_internal(key).node);
	}


	/// @brief Insert new element into the tree.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	insert_result insert(T* const element)
	{
		auto const r = find_internal(m_key_selector(*element));
		if (r.node != nullptr)
		{
			return { vsm_rb_elem(r.node), false };
		}
		base::insert(vsm_rb_hook(element), r.parent);
		return { element, true };
	}

	/// @brief Remove an element from the tree.
	/// @param element Element to be removed.
	/// @pre @p element is part of this tree.
	void remove(T* const element) noexcept
	{
		base::remove(vsm_rb_hook(element));
	}

	/// @brief Remove all elements from the tree.
	using base::clear;

	/// @brief Flatten the tree into a linked list using an in-order traversal.
	[[nodiscard]] list<T> flatten() noexcept
	{
		size_t const size = this->size();
		return list<T>(static_cast<link_container&&>(*this), base::flatten(), size);
	}


	/// @brief Create an iterator referring to an element.
	/// @pram element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] iterator make_iterator(T* const element) noexcept
	{
		vsm_intrusive_link_check(*this, *element);
		return iterator(vsm_rb_hook(element));
	}

	/// @brief Create an iterator referring to an element.
	/// @pram element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] const_iterator make_iterator(T const* const element) const noexcept
	{
		vsm_intrusive_link_check(*this, *element);
		return const_iterator(vsm_rb_hook(element, const));
	}


	[[nodiscard]] iterator begin() noexcept
	{
		return iterator(iterator_begin(&m_root.value));
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator(iterator_begin(const_cast<hook**>(&m_root.value)));
	}

	[[nodiscard]] iterator end() noexcept
	{
		return iterator(&m_root.value);
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator(const_cast<hook**>(&m_root.value));
	}


	friend void swap(wb_tree& lhs, wb_tree& rhs) noexcept
	{
		using std::swap;
		swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
		swap(lhs.m_key_selector, rhs.m_key_selector);
		swap(lhs.m_comparator, rhs.m_comparator);
	}

private:
	template<typename Key>
	find_result find_internal(Key const& key) const
		noexcept(noexcept(m_comparator(key, std::declval<key_type const&>())))
	{
		hook** parent = const_cast<hook**>(&m_root.value);
		bool l = 0;

		while (parent[l] != nullptr)
		{
			hook* const child = parent[l];

			auto const ordering = m_comparator(key, m_key_selector(*vsm_rb_elem(child)));
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

#undef vsm_rb_hook
#undef vsm_rb_elem

} // namespace detail::rb_tree_

using detail::rb_tree_::rb_tree;

} // namespace vsm::intrusive
