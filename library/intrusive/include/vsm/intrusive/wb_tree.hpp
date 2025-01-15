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
namespace detail {

#define vsm_detail_wb_hook_from_children(children) \
	reinterpret_cast<hook*>(children)

struct _wb
{
	template<typename T>
	using ptr = incomplete_tag_ptr<T, bool, 1>;

	struct hook
	{
		hook* children[2];

		// Pointer to children[0] of a parent hook or _wb::m_root;
		hook** parent;

		// Weight of the subtree rooted at this node, including this node.
		size_t weight;
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

		explicit iterator(hook** const children)
			: m_children(children)
		{
		}

		template<cv_convertible_to<T> U>
		iterator(iterator<U, Tag> const& other)
			: m_children(other.m_children)
		{
		}


		[[nodiscard]] T& operator*() const
		{
			return *linker::get_elem<T, Tag>(vsm_detail_wb_hook_from_children(m_children));
		}

		[[nodiscard]] T* operator->() const
		{
			return linker::get_elem<T, Tag>(vsm_detail_wb_hook_from_children(m_children));
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

	private:
		friend _wb;

		template<typename, typename>
		friend class iterator;
	};


	hook* m_root = nullptr;


	_wb() = default;

	_wb(_wb&& other) noexcept
		: m_root(other.m_root)
	{
		other.m_root = nullptr;
	}

	_wb& operator=(_wb&& other) & noexcept
	{
		if (m_root != nullptr)
		{
			clear();
		}

		m_root = other.m_root;
		other.m_root = nullptr;

		return *this;
	}

	~_wb()
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

	hook* select(size_t rank) const;
	size_t rank(hook const* node) const;

	void insert(hook* node, ptr<hook*> parent_and_side);
	void erase(hook* node);
	void clear();
	_list::hook* flatten();

	friend void swap(_wb& lhs, _wb& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_root, rhs.m_root);
	};


	template<typename T, typename Tag>
	static hook** get_iterator_ptr(iterator<T, Tag> const& it)
	{
		return it.m_children;
	}

	static hook** iterator_begin(hook** root);
	static hook** iterator_advance(hook** children, bool l);
};

} // namespace detail

template<typename Tag>
using basic_wb_tree_link = basic_link<4, Tag>;

using wb_tree_link = basic_wb_tree_link<void>;

template<typename T>
using wb_tree_children = std::array<T*, 2>;

template<
	typename T,
	key_selector<T> KeySelector = identity_key_selector,
	typename Comparator = std::compare_three_way>
class wb_tree : detail::_wb
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;

	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<element_type const&>()));

	using       iterator = _wb::iterator<      element_type, tag_type>;
	using const_iterator = _wb::iterator<const element_type, tag_type>;

	using insert_result = vsm::insert_result<iterator>;

private:
	vsm_no_unique_address KeySelector m_key_selector;
	vsm_no_unique_address Comparator m_comparator;

public:
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
		return m_root == nullptr
			? 0
			: m_root->weight;
	}

	/// @return True if the set is empty.
	[[nodiscard]] bool empty() const
	{
		return m_root == nullptr;
	}


	[[nodiscard]] element_type& root()
	{
		vsm_assert(m_root != nullptr);
		return *get_elem(m_root);
	}

	[[nodiscard]] element_type const& root() const
	{
		vsm_assert(m_root != nullptr);
		return *get_elem(m_root);
	}


	[[nodiscard]] size_t weight(element_type const& element) const
	{
		vsm_intrusive_link_check(*this, element);
		return get_hook(std::addressof(element))->weight;
	}

	[[nodiscard]] wb_tree_children<element_type> children(element_type const& element)
	{
		vsm_intrusive_link_check(*this, element);
		hook const* const node = get_hook(std::addressof(element));
		return { get_elem(node->children[0]), get_elem(node->children[1]) };
	}

	[[nodiscard]] wb_tree_children<element_type const> children(element_type const& element) const
	{
		vsm_intrusive_link_check(*this, element);
		hook const* const node = get_hook(std::addressof(element));
		return { get_elem(node->children[0]), get_elem(node->children[1]) };
	}


	[[nodiscard]] element_type& select(size_t const rank)
	{
		return *get_elem(_wb::select(rank));
	}

	[[nodiscard]] element_type const& select(size_t const rank) const
	{
		return *get_elem(_wb::select(rank));
	}

	[[nodiscard]] size_t rank(element_type const& element) const
	{
		return _wb::rank(get_hook(std::addressof(element)));
	}


	/// @brief Find element by key.
	/// @param key Lookup key.
	/// @return Iterator to the element or end.
	template<typename Key = key_type>
	[[nodiscard]] iterator find(Key const& key)
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook* const node = _find(key).node;

		return node == nullptr
			? end()
			: iterator(node->children);
	}

	/// @brief Find element by key.
	/// @param key Lookup key.
	/// @return Iterator to the element or end.
	template<typename Key = key_type>
	[[nodiscard]] const_iterator find(Key const& key) const
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook const* const node = _find(key).node;

		return node == nullptr
			? end()
			: const_iterator(node->children);
	}

	/// @brief Access element by key.
	/// @param key Lookup key.
	/// @return Pointer to the element or null.
	template<typename Key = key_type>
	[[nodiscard]] element_type* at_ptr(Key const& key)
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook* const node = _find(key).node;

		return node == nullptr
			? nullptr
			: get_elem(node);
	}

	/// @brief Access element by key.
	/// @param key Lookup key.
	/// @return Pointer to the element or null.
	template<typename Key = key_type>
	[[nodiscard]] element_type const* at_ptr(Key const& key) const
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook const* const node = _find(key).node;

		return node == nullptr
			? nullptr
			: get_elem(node);
	}


	insert_result insert(element_type& element)
	{
		auto const r = _find(m_key_selector(element));

		if (r.node != nullptr)
		{
			return { iterator(r.node->children), false };
		}

		_wb::insert(
			detail::linker::construct<hook, tag_type>(std::addressof(element)),
			const_pointer_cast<ptr<hook*>>(r.parent));

		return { iterator(get_hook(std::addressof(element))->children), true };
	}

	void erase(element_type& element)
	{
		_wb::erase(get_hook(std::addressof(element)));
	}

	void erase(const_iterator const position)
	{
		_wb::erase(reinterpret_cast<hook*>(get_iterator_ptr(position)));
	}

	using _wb::clear;

	[[nodiscard]] list<T> flatten()
	{
		size_t const size = this->size();
		return list<T>(_wb::flatten(), size);
	}



	[[nodiscard]] iterator make_iterator(element_type& element)
	{
		return iterator(get_hook(std::addressof(element)));
	}

	[[nodiscard]] const_iterator make_iterator(element_type const& element) const
	{
		return const_iterator(get_hook(std::addressof(element)));
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
		swap(lhs.m_key_selector, rhs.m_key_selector);
		swap(lhs.m_comparator, rhs.m_comparator);
	}

private:
	[[nodiscard]] static auto* get_hook(auto* const element)
	{
		return detail::linker::get_hook<hook, tag_type>(element);
	}

	[[nodiscard]] static auto* get_elem(auto* const hook)
	{
		return detail::linker::get_elem<element_type, tag_type>(hook);
	}

	template<typename Key>
	find_result _find(Key const& key) const
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
