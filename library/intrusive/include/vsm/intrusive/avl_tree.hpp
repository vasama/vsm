#pragma once

#include <vsm/intrusive/link.hpp>
#include <vsm/intrusive/list.hpp>

#include <vsm/concepts.hpp>
#include <vsm/insert_result.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/utility.hpp>

#include <type_traits>
#include <utility>

namespace vsm::intrusive {
namespace detail {

#define vsm_detail_avl_hook_from_children(children) \
	reinterpret_cast<hook*>(children)

struct _avl
{
	template<typename T>
	using ptr = incomplete_tag_ptr<T, bool, 1>;

	struct hook
	{
		// Child pointers with tag==1 indicating +1 subtree height on that side.
		ptr<hook> children[2];

		// Pointer to hook::children[0] of a parent hook or to _avl::m_root.
		ptr<hook>* parent;
	};

	template<typename T, typename Tag>
	class iterator
	{
		ptr<hook>* m_children;

	public:
		using difference_type = ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;


		iterator() = default;

		explicit iterator(ptr<hook>* const children)
			: m_children(children)
		{
		}

		explicit iterator(T* const element)
			: m_children(detail::linker::get_hook<hook, Tag>(element)->children)
		{
		}

		template<cv_convertible_to<T> U>
		iterator(iterator<U, Tag> const& other)
			: m_children(other.m_children)
		{
		}


		[[nodiscard]] T& operator*() const
		{
			static_assert(check<T, Tag, hook>);
			return *linker::get_elem<T, Tag>(vsm_detail_avl_hook_from_children(m_children));
		}

		[[nodiscard]] T* operator->() const
		{
			static_assert(check<T, Tag, hook>);
			return linker::get_elem<T, Tag>(vsm_detail_avl_hook_from_children(m_children));
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
		friend _avl;

		template<typename, typename>
		friend class iterator;
	};


	ptr<hook> m_root = nullptr;
	size_t m_size = 0;

	_avl() = default;

	_avl(_avl&& other) noexcept
		: m_root(other.m_root)
		, m_size(other.m_size)
	{
		other.m_root = nullptr;
		other.m_size = 0;
	}

	_avl& operator=(_avl&& other) & noexcept
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

	~_avl()
	{
		if (!m_root.is_zero())
		{
			clear();
		}
	}


	struct find_result
	{
		hook* node;
		ptr<ptr<hook> const> parent;
	};

	ptr<hook> const* lower_bound(ptr<ptr<hook> const> parent_and_side) const;
	void insert(hook* node, ptr<ptr<hook>> parent_and_side);
	void erase(hook* node);
	void replace(hook* existing_node, hook* new_node);
	void clear();
	_list::hook* flatten();

	friend void swap(_avl& lhs, _avl& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_root, rhs.m_root);
		swap(lhs.m_size, rhs.m_size);
	}


	template<typename T, typename Tag>
	static ptr<hook>* get_iterator_ptr(iterator<T, Tag> const& it)
	{
		return it.m_children;
	}

	static ptr<hook>* iterator_begin(ptr<hook>* root);
	static ptr<hook>* iterator_advance(ptr<hook>* children, bool l);
};

} // namespace detail

template<typename Tag>
using basic_avl_tree_link = basic_link<3, Tag>;

using avl_tree_link = basic_avl_tree_link<void>;

template<
	typename T,
	key_selector<detail::element_t<T>> KeySelector = identity_key_selector,
	typename Comparator = std::compare_three_way>
class avl_tree : detail::_avl
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;

	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<element_type const&>()));

private:
	vsm_no_unique_address KeySelector m_key_selector;
	vsm_no_unique_address Comparator m_comparator;

public:

	using       iterator = _avl::iterator<      element_type, tag_type>;
	using const_iterator = _avl::iterator<const element_type, tag_type>;

	using insert_result = vsm::insert_result<iterator>;


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


	/// @brief Find element by key.
	/// @param key Lookup key.
	/// @return Iterator to the element or end.
	template<typename Key = key_type>
	[[nodiscard]] iterator find(Key const& key)
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook* const hook = _find(key).node;

		return hook == nullptr
			? end()
			: iterator(hook->children);
	}

	/// @brief Find element by key.
	/// @param key Lookup key.
	/// @return Iterator to the element or end.
	template<typename Key = key_type>
	[[nodiscard]] const_iterator find(Key const& key) const
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook const* const hook = _find(key).node;

		return hook == nullptr
			? end()
			: const_iterator(hook->children);
	}

	/// @brief Access element by key.
	/// @param key Lookup key.
	/// @return Pointer to the element or null.
	template<typename Key = key_type>
	[[nodiscard]] element_type* at_ptr(Key const& key)
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook* const hook = _find(key).node;

		return hook == nullptr
			? nullptr
			: get_elem(hook);
	}

	/// @brief Access element by key.
	/// @param key Lookup key.
	/// @return Pointer to the element or null.
	template<typename Key = key_type>
	[[nodiscard]] element_type const* at_ptr(Key const& key) const
		requires (requires (key_type const& tree_key) { m_comparator(key, tree_key); })
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		hook const* const hook = _find(key).node;

		return hook == nullptr
			? nullptr
			: get_elem(hook);
	}


	[[nodiscard]] iterator lower_bound(key_type const& key)
	{
		return iterator(const_cast<ptr<hook>*>(_lower_bound(key)));
	}

	[[nodiscard]] const_iterator lower_bound(key_type const& key) const
	{
		return const_iterator(_lower_bound(key));
	}

	template<typename Key = key_type>
	[[nodiscard]] iterator equivalent_lower_bound(Key const& key)
	{
		return iterator(const_cast<ptr<hook>*>(_lower_bound(key)));
	}

	template<typename Key = key_type>
	[[nodiscard]] const_iterator equivalent_lower_bound(Key const& key) const
	{
		return const_iterator(_lower_bound(key));
	}


	/// @brief Insert new element into the tree.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any intrusive container.
	insert_result insert(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		auto const r = _find(vsm_as_const(m_key_selector)(element));

		if (r.node != nullptr)
		{
			return { iterator(r.node->children), false };
		}

		_avl::insert(
			detail::linker::construct<hook, tag_type>(std::addressof(element)),
			const_pointer_cast<ptr<ptr<hook>>>(r.parent));

		return { iterator(get_hook(std::addressof(element))->children), true };
	}

#if 0
	/// @brief Insert the new element into the tree as close as possible to the position just
	///        before @p hint.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any intrusive container.
	insert_result insert_hint(const_iterator const hint, element_type* const element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		auto const r = _find_from(
			vsm_as_const(m_key_selector)(*element),
			std::to_address(hint));

		if (r.node != nullptr)
		{
			return { get_elem(r.node), false };
		}

		_avl::insert(
			detail::linker::construct<hook, tag_type>(element),
			const_pointer_cast<ptr<ptr<hook>>>(r.parent));

		return { element, true };
	}
#endif

	/// @brief Remove an element from the tree.
	/// @param element Element to be removed.
	/// @pre @p element is part of this tree.
	void erase(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_avl::erase(get_hook(std::addressof(element)));
	}

	void erase(const_iterator const position)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_avl::erase(reinterpret_cast<hook*>(_avl::get_iterator_ptr(position)));
	}

	/// @brief Replace an element with a new equivalent element.
	/// @param existing_element The element to be removed.
	/// @param new_element The element to be inserted.
	/// @pre @p existing_element is part of this tree.
	/// @pre @p new_element is not part of any intrusive container.
	/// @pre The selected keys of @p existing_element and @p new_element are equivalent.
	void replace(element_type& existing_element, element_type& new_element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);

		vsm_assert(
			vsm_as_const(m_key_selector)(existing_element) ==
			vsm_as_const(m_key_selector)(new_element));

		_avl::replace(
			get_hook(std::addressof(existing_element)),
			get_hook(std::addressof(new_element)));
	}

	/// @brief Replace an element with a new equivalent element.
	/// @param position The position of the element to be removed.
	/// @param new_element The element to be inserted.
	/// @pre @p position is an iterator referring to an element in this tree.
	/// @pre @p new_element is not part of any intrusive container.
	/// @pre The selected keys of @p position and @p new_element are equivalent.
	void replace(const_iterator const position, element_type& new_element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);

		vsm_assert(position != end());
		vsm_assert(
			vsm_as_const(m_key_selector)(position) ==
			vsm_as_const(m_key_selector)(new_element));

		_avl::replace(
			reinterpret_cast<hook*>(_avl::get_iterator_ptr(position)),
			get_hook(std::addressof(new_element)));
	}

	/// @brief Remove all elements from the tree.
	using _avl::clear;

	/// @brief Flatten the tree into a linked list using an in-order traversal.
	[[nodiscard]] list<T> flatten()
	{
		size_t const size = this->size();
		return list<T>(_avl::flatten(), size);
	}


	/// @brief Create an iterator referring to an element.
	/// @param element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] iterator make_iterator(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		return iterator(get_hook(std::addressof(element)));
	}

	/// @brief Create an iterator referring to an element.
	/// @param element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] const_iterator make_iterator(element_type const& element) const
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		return const_iterator(get_hook(std::addressof(element)));
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
		swap(static_cast<_avl&>(lhs), static_cast<_avl&>(rhs));
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
	ptr<hook> const* _lower_bound(Key const& key) const
	{
		auto const r = _find(key);

		return r.node != nullptr
			? r.node->children
			: _avl::lower_bound(r.parent);
	}

	template<typename Key>
	find_result _find(Key const& key) const
	{
		// The "current" node at each iteration is the "left" (given by l) child of parent.

		// The parent pointer itself never becomes null. Initially it points to the container root.
		ptr<hook> const* parent = &m_root;

		// The container root only has a left child. Therefore l must initially be 0.
		bool l = 0;

		while (!parent[l].is_zero())
		{
			hook* const child = parent[l].ptr();

			auto const ordering = m_comparator(
				key,
				m_key_selector(*get_elem(child)));

			if (ordering == 0)
			{
				return { child, { parent, l } };
			}

			parent = child->children;

			// Of the two potential children of the "current" node, which we should look at in the
			// next iteration, depends on the ordering of the search key K_s and the key of the
			// "current" node K_n. If K_s < K_n, then it is the left child at index 0. The boolean
			// result of the comparison, if the condition is true, is 1 and must be negated.
			l = !(ordering < 0);
		}

		return find_result{ nullptr, { parent, l } };
	}
};

} // namespace vsm::intrusive
