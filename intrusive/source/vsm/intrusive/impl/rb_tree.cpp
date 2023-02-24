#include <vsm/intrusive/rb_tree.hpp>

// NOLINTBEGIN(modernize-use-bool-literals)
// NOLINTBEGIN(readability-implicit-bool-conversion)

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _rb::hook;
static_assert(sizeof(hook) == sizeof(rb_tree_link));
//TODO: Add the same assert to other hook types.
static_assert(std::is_standard_layout_v<hook>);

template<typename T>
using ptr = _rb::ptr<T>;

static_assert(check_incomplete_tag_ptr<ptr<hook>>());


static constexpr bool black = false;
static constexpr bool red = true;

static void set_color(hook* const node, bool const color)
{
	node->parent.set_tag(color);
}

static bool get_color(hook const* const node)
{
	return node->parent.tag();
}

static hook* leftmost(hook* node, bool const l)
{
	while (node->children[l] != nullptr)
	{
		node = node->children[l];
	}
	return node;
}

// Rotate from left to right.
static void rotate(hook* const root, bool const l)
{
	bool const r = !l;

	hook** const parent = root->parent.ptr();
	hook* const pivot = root->children[l];
	hook* const child = pivot->children[r];

	root->children[l] = child;
	root->parent = pivot->children;

	pivot->children[r] = root;
	pivot->parent = parent;

	if (child != nullptr)
	{
		child->parent = root->children;
	}
	parent[root != parent[0]] = pivot;
}

[[maybe_unused]] static bool invariant(_rb const& self)
{
	size_t size = 0;

	hook* const* node_children = self.m_root->children;
	bool node_side = 0;

	while (node_children != &self.m_root)
	{
		hook const* node = reinterpret_cast<hook const*>(node_children);

		if (node_side == 0)
		{
			++size;

			uint8_t children = 0;
			uint8_t red_children = 0;

			for (hook const* const child : node->children)
			{
				if (child != nullptr)
				{
					++children;
					if (get_color(child) == red)
					{
						++red_children;
					}
				}
			}

			// Red nodes have no red children.
			if (get_color(node) == red && red_children != 0)
			{
				return false;
			}

			// All only children are red.
			if (children == 1 && red_children != 1)
			{
				return false;
			}

			if (hook const* const child = node->children[0])
			{
				node_children = child->children;
				node_side = 0;
				continue;
			}
		}

		if (hook const* const child = node->children[1])
		{
			node_children = child->children;
			node_side = 0;
			continue;
		}

		node_side = 0;
		while (node_children != &self.m_root)
		{
			node_children = node->parent.ptr();

			if (node_children[0] == node)
			{
				node_side = 1;
				break;
			}

			node = reinterpret_cast<hook const*>(node_children);
		}
	}

	return size == self.m_size;
}


static void rebalance_after_insert(hook** const root, hook* node)
{
	while (true)
	{
		if (node->parent.ptr() == root)
		{
			return;
		}

		hook* const parent = reinterpret_cast<hook*>(node->parent.ptr());

		if (get_color(parent) == black)
		{
			return;
		}

		if (parent->parent.ptr() == root)
		{
			set_color(parent, black);
			return;
		}

		hook* const grandparent = reinterpret_cast<hook*>(parent->parent.ptr());
		bool const parent_side = parent != grandparent->children[0];
		hook* const parent_sibling = grandparent->children[!parent_side];

		if (parent_sibling == nullptr || get_color(parent_sibling) == black)
		{
			if (node != parent->children[parent_side])
			{
				rotate(parent, !parent_side);
				node = parent;
			}

			rotate(grandparent, parent_side);

			vsm_assert(node->parent.ptr() != root);
			set_color(reinterpret_cast<hook*>(node->parent.ptr()), black);
			set_color(grandparent, red);

			return;
		}

		set_color(parent, black);
		set_color(parent_sibling, black);
		set_color(grandparent, red);

		node = grandparent;
	}
}

void _rb::insert(hook* node, ptr<hook*> const parent_and_side)
{
	vsm_assert(parent_and_side[parent_and_side.tag()] == nullptr);

	++m_size;

	node->children[0] = nullptr;
	node->children[1] = nullptr;
	node->parent = ptr<hook*>(parent_and_side.ptr(), red);
	parent_and_side[parent_and_side.tag()] = node;

	rebalance_after_insert(&m_root, node);

	//vsm_assert_slow(invariant(*this));
}

void _rb::remove(hook* const node)
{

}

void _rb::clear()
{
	m_root = nullptr;
	m_size = 0;
}

_list::hook* _rb::flatten()
{
	hook* const root = m_root;

	if (root == nullptr)
	{
		return nullptr;
	}

	m_root = nullptr;
	m_size = 0;

	auto const flatten_side = [](hook* node, bool const l) -> _list::hook*
	{
		bool const r = !l;

		while (node->children[l] != nullptr)
		{
			// Rotate the whole left subtree right to left turning it into a list.

			hook* const tail = node->children[l];
			hook* head = tail;

			while (head->children[r] != nullptr)
			{
				hook* const pivot = head->children[r];

				pivot->children[l] = head;
				head->children[r] = pivot;

				head = pivot;
			}

			node->children[l] = head;
			head->children[r] = node;

			node = tail;
		}

		return start_lifetime_as<_list::hook>(node);
	};

	_list::hook* const head = flatten_side(root, 0);
	_list::hook* const tail = flatten_side(root, 1);

	head->siblings[0] = tail;
	tail->siblings[1] = head;

	return head;
}


hook** _rb::iterator_begin(hook** const root)
{
	hook* const node = *root;
	return node != nullptr ? leftmost(node, 0)->children : root;
}

hook** _rb::iterator_advance(hook** const children, bool const l)
{
	bool const r = l ^ 1;

	// If node has a right child: stop at its leftmost descendant.
	if (children[r] != nullptr)
	{
		return leftmost(children[r], l)->children;
	}

	hook* node = reinterpret_cast<hook*>(children);

	// Iterate through ancestors until the branch where the node is the left child.
	while (node != *node->parent)
	{
		node = reinterpret_cast<hook*>(node->parent.ptr());
	}

	return node->parent.ptr();
}
