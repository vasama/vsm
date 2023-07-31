#pragma once

#include <vsm/intrusive/link.hpp>
#include <vsm/intrusive/list.hpp>

#include <vsm/insert_result.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/utility.hpp>

#include <array>
#include <concepts>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {

using wb_tree_link = link<4>;

namespace detail::wb_tree_ {

#define vsm_detail_wb_hook(elem, ...) \
	(reinterpret_cast<hook __VA_ARGS__*>(static_cast<wb_tree_link __VA_ARGS__*>(elem)))

#define vsm_detail_wb_elem(hook, ...) \
	(static_cast<T __VA_ARGS__*>(reinterpret_cast<wb_tree_link __VA_ARGS__*>(hook)))

#define vsm_detail_wb_hook_from_children(children) \
	static_cast<hook*>(reinterpret_cast<hook_data*>(children))


template<typename T>
using ptr = incomplete_tag_ptr<T, bool, 1>;

struct hook;

struct hook_data
{
	hook* children[2];

	// Pointer to children[0] of a parent hook or base::m_root;
	hook** parent;

	// Weight of the subtree rooted at this node, including this node.
	uintptr_t weight;
};

struct hook : link_base, hook_data {};

struct base : link_container
{
	hook* m_root = {};


	base() = default;

	base(base&& other) noexcept
		: link_container(static_cast<link_container&&>(other))
		, m_root(other.m_root)
	{
		other.m_root = {};
	}

	base& operator=(base&& other) & noexcept
	{
		if (m_root != nullptr)
		{
			clear();
		}

		static_cast<link_container&>(*this) = static_cast<link_container&&>(other);

		m_root = other.m_root;
		other.m_root = {};

		return *this;
	}

	~base()
	{
		if (m_root != nullptr)
		{
			clear();
		}
	}


	struct find_result
	{
		hook* node;
		ptr<hook*> parent;
	};

	hook* select(size_t rank) const;
	size_t rank(hook const* node) const;

	void insert(hook* node, ptr<hook*> parent_and_side);
	void remove(hook* node);
	void clear();
	list_::hook* flatten();

	friend void swap(base& lhs, base& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_root, rhs.m_root);
	};
};

hook** iterator_begin(hook** root);
hook** iterator_advance(hook** children, bool l);


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


	[[nodiscard]] T& operator*() const
	{
		return *vsm_detail_wb_elem(vsm_detail_wb_hook_from_children(m_children));
	}

	[[nodiscard]] T* operator->() const
	{
		return vsm_detail_wb_elem(vsm_detail_wb_hook_from_children(m_children));
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


template<std::derived_from<wb_tree_link> T>
using wb_tree_children = std::array<T, 2>;

template<std::derived_from<wb_tree_link> T,
	key_selector<T> KeySelector = identity_key_selector,
	typename Comparator = std::compare_three_way>
class wb_tree : base
{
	vsm_no_unique_address KeySelector m_key_selector;
	vsm_no_unique_address Comparator m_comparator;

public:
	using element_type = T;
	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

	using       iterator = wb_tree_::iterator<      T>;
	using const_iterator = wb_tree_::iterator<const T>;

	using insert_result = vsm::insert_result<T>;


	wb_tree() = default;

	explicit constexpr wb_tree(KeySelector key_selector)
		: m_key_selector(vsm_move(key_selector))
	{
	}

	explicit constexpr wb_tree(Comparator comparator)
		: m_comparator(vsm_move(comparator))
	{
	}

	explicit constexpr wb_tree(KeySelector key_selector, Comparator comparator)
		: m_key_selector(vsm_move(key_selector))
		, m_comparator(vsm_move(comparator))
	{
	}

	wb_tree& operator=(wb_tree&&) & = default;


	/// @return Size of the set.
	[[nodiscard]] size_t size() const
	{
		return m_root != nullptr ? m_root->weight : 0;
	}

	/// @return True if the set is empty.
	[[nodiscard]] bool empty() const
	{
		return m_root == nullptr;
	}


	[[nodiscard]] T* root()
	{
		vsm_assert(m_root != nullptr);
		return vsm_detail_wb_elem(m_root);
	}

	[[nodiscard]] T const* root() const
	{
		vsm_assert(m_root != nullptr);
		return vsm_detail_wb_elem(m_root);
	}


	[[nodiscard]] size_t weight(T const* const element) const
	{
		vsm_intrusive_link_check(*this, *element);
		return vsm_detail_wb_hook(element, const)->weight;
	}

	[[nodiscard]] wb_tree_children<T> children(T const* const element)
	{
		vsm_intrusive_link_check(*this, *element);
		hook const* const node = vsm_detail_wb_hook(element, const);
		return { vsm_detail_wb_elem(node->children[0]), vsm_detail_wb_elem(node->children[1]) };
	}

	[[nodiscard]] wb_tree_children<T const> children(T const* const element) const
	{
		vsm_intrusive_link_check(*this, *element);
		hook const* const node = vsm_detail_wb_hook(element, const);
		return { vsm_detail_wb_elem(node->children[0]), vsm_detail_wb_elem(node->children[1]) };
	}


	[[nodiscard]] T* select(size_t const rank)
	{
		return vsm_detail_wb_elem(base::select(rank));
	}

	[[nodiscard]] const T* select(size_t const rank) const
	{
		return vsm_detail_wb_elem(base::select(rank));
	}

	[[nodiscard]] size_t rank(T const* const element) const
	{
		return base::rank(vsm_detail_wb_hook(element));
	}


	[[nodiscard]] T* find(key_type const& key)
		noexcept(noexcept(find_internal(key)))
	{
		return vsm_detail_wb_elem(find_internal(key).node);
	}

	[[nodiscard]] T const* find(key_type const& key) const
		noexcept(noexcept(find_internal(key)))
	{
		return vsm_detail_wb_elem(find_internal(key).node);
	}

	template<typename Key>
	[[nodiscard]] T* find_equivalent(Key const& key)
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		return vsm_detail_wb_elem(find_internal(key).node);
	}

	template<typename Key>
	[[nodiscard]] T const* find_equivalent(Key const& key) const
		noexcept(noexcept(find_internal(key)))
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		return vsm_detail_wb_elem(find_internal(key).node);
	}


	insert_result insert(T* const element)
		noexcept(noexcept(find_internal(std::declval<key_type>())))
	{
		auto const r = find_internal(m_key_selector(*element));
		if (r.node != nullptr)
		{
			return { vsm_detail_wb_elem(r.node), false };
		}
		base::insert(vsm_detail_wb_hook(element), r.parent);
		return { element, true };
	}

	void remove(T* const element)
	{
		base::remove(vsm_detail_wb_hook(element));
	}

	using base::clear;

	[[nodiscard]] list<T> flatten()
	{
		size_t const size = this->size();
		return list<T>(static_cast<link_container&&>(*this), base::flatten(), size);
	}



	[[nodiscard]] iterator make_iterator(T* const element)
	{
		vsm_intrusive_link_check(*this, *element);
		return iterator(vsm_detail_wb_hook(element));
	}

	[[nodiscard]] const_iterator make_iterator(T const* const element) const
	{
		vsm_intrusive_link_check(*this, *element);
		return const_iterator(vsm_detail_wb_hook(element, const));
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
		hook** parent = const_cast<hook**>(&m_root);
		bool l = 0;

		while (parent[l] != nullptr)
		{
			hook* const child = parent[l];

			auto const ordering = m_comparator(key, m_key_selector(*vsm_detail_wb_elem(child)));
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

} // namespace detail::wb_tree_

using detail::wb_tree_::wb_tree_children;
using detail::wb_tree_::wb_tree;

} // namespace vsm::intrusive
