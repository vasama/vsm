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

using avl_tree_link = link<3>;

namespace detail::avl_tree_ {

#define vsm_detail_avl_hook(element, ...) \
	(reinterpret_cast<hook __VA_ARGS__*>(static_cast<avl_tree_link __VA_ARGS__*>(element)))

#define vsm_detail_avl_elem(hook, ...) \
	(static_cast<T __VA_ARGS__*>(reinterpret_cast<avl_tree_link __VA_ARGS__*>(hook)))

#define vsm_detail_avl_hook_from_children(children) \
	static_cast<hook*>(reinterpret_cast<hook_data*>(children))


template<typename T>
using ptr = incomplete_tag_ptr<T, bool, 1>;

struct hook;

struct hook_data
{
	// Child pointers with tag==1 indicating +1 subtree height on that side.
	ptr<hook> children[2];

	// Pointer to hook::children[0] of a parent hook or to base::m_root.
	ptr<hook>* parent;
};

struct hook : link_base, hook_data {};

struct base : link_container
{
	ptr<hook> m_root = {};
	size_t m_size = {};


	base() = default;

	base(base&& other) noexcept
		: link_container(static_cast<link_container&&>(other))
		, m_root(other.m_root)
		, m_size(other.m_size)
	{
		other.m_root = {};
		other.m_size = {};
	}

	base& operator=(base&& other) & noexcept
	{
		if (!m_root.is_zero())
		{
			clear();
		}

		static_cast<link_container&>(*this) = static_cast<link_container&&>(other);

		m_root = other.m_root;
		m_size = other.m_size;
		other.m_root = {};
		other.m_size = {};

		return *this;
	}

	~base()
	{
		if (!m_root.is_zero())
		{
			clear();
		}
	}


	struct find_result
	{
		hook* node;
		ptr<const ptr<hook>> parent;
	};

	void insert(hook* node, ptr<ptr<hook>> parent_and_side);
	void remove(hook* node);
	void clear();
	list_::hook* flatten();

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

ptr<hook>* iterator_begin(ptr<hook>* root);
ptr<hook>* iterator_advance(ptr<hook>* children, bool l);


template<typename T>
class iterator
{
	ptr<hook>* m_children;

public:
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = T*;
	using reference = T&;


	iterator() = default;

	iterator(ptr<hook>* const children)
		: m_children(children)
	{
	}


	[[nodiscard]] T& operator*() const
	{
		return *vsm_detail_avl_elem(vsm_detail_avl_hook_from_children(m_children));
	}

	[[nodiscard]] T* operator->() const
	{
		return vsm_detail_avl_elem(vsm_detail_avl_hook_from_children(m_children));
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


template<std::derived_from<avl_tree_link> T,
	key_selector<T> KeySelector = identity_key_selector,
	typename Comparator = std::compare_three_way>
class avl_tree : base
{
	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

	vsm_no_unique_address KeySelector m_key_selector;
	vsm_no_unique_address Comparator m_comparator;

public:
	using element_type = T;

	using       iterator = avl_tree_::iterator<      T>;
	using const_iterator = avl_tree_::iterator<const T>;

	using insert_result = vsm::insert_result<T>;


	avl_tree() = default;

	explicit constexpr avl_tree(KeySelector key_selector)
		: m_key_selector(vsm_move(key_selector))
	{
	}

	explicit constexpr avl_tree(Comparator comparator)
		: m_comparator(vsm_move(comparator))
	{
	}

	explicit constexpr avl_tree(KeySelector key_selector, Comparator comparator)
		: m_key_selector(vsm_move(key_selector))
		, m_comparator(vsm_move(comparator))
	{
	}

	avl_tree& operator=(avl_tree&&) & = default;


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
	[[nodiscard]] T* find(key_type const& key)
		noexcept(noexcept(find_internal(key)))
	{
		return vsm_detail_avl_elem(find_internal(key).node);
	}

	/// @brief Find element by homogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element, or null if not found.
	[[nodiscard]] T const* find(const key_type& key) const
		noexcept(noexcept(find_internal(key)))
	{
		return vsm_detail_avl_elem(find_internal(key).node);
	}

	/// @brief Find element by heterogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element or null.
	template<typename Key>
	[[nodiscard]] T* find_equivalent(Key const& key)
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		return vsm_detail_avl_elem(find_internal(key).node);
	}

	/// @brief Find element by heterogeneous key.
	/// @param key Lookup key.
	/// @return Pointer to element or null.
	template<typename Key>
	[[nodiscard]] T const* find_equivalent(Key const& key) const
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		return vsm_detail_avl_elem(find_internal(key).node);
	}


	/// @brief Insert new element into the tree.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	insert_result insert(T* const element)
	{
		auto const r = find_internal(m_key_selector(*element));
		if (r.node != nullptr)
		{
			return { vsm_detail_avl_elem(r.node), false };
		}
		base::insert(vsm_detail_avl_hook(element), const_pointer_cast<ptr<ptr<hook>>>(r.parent));
		return { element, true };
	}

	/// @brief Remove an element from the tree.
	/// @param element Element to be removed.
	/// @pre @p element is part of this tree.
	void remove(T* const element)
	{
		base::remove(vsm_detail_avl_hook(element));
	}

	/// @brief Remove all elements from the tree.
	using base::clear;

	/// @brief Flatten the tree into a linked list using an in-order traversal.
	[[nodiscard]] list<T> flatten()
	{
		size_t const size = this->size();
		return list<T>(static_cast<link_container&&>(*this), base::flatten(), size);
	}


	/// @brief Create an iterator referring to an element.
	/// @pram element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] iterator make_iterator(T* const element)
	{
		vsm_intrusive_link_check(*this, *element);
		return iterator(vsm_detail_avl_hook(element));
	}

	/// @brief Create an iterator referring to an element.
	/// @pram element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] const_iterator make_iterator(T const* const element) const
	{
		vsm_intrusive_link_check(*this, *element);
		return const_iterator(vsm_detail_avl_hook(element, const));
	}


	[[nodiscard]] iterator begin()
	{
		return iterator(iterator_begin(&m_root));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(iterator_begin(const_cast<ptr<hook>*>(&m_root)));
	}

	[[nodiscard]] iterator end()
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator end() const
	{
		return const_iterator(const_cast<ptr<hook>*>(&m_root));
	}


	friend void swap(avl_tree& lhs, avl_tree& rhs) noexcept
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
		ptr<hook> const* parent = &m_root;
		bool l = 0;

		while (!parent[l].is_zero())
		{
			hook* const child = parent[l].ptr();

			auto const ordering = m_comparator(key, m_key_selector(*vsm_detail_avl_elem(child)));
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

} // namespace detail::avl_tree_

using detail::avl_tree_::avl_tree;

} // namespace vsm::intrusive
