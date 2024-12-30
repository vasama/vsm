#include <vsm/intrusive/rb_tree.hpp>

// NOLINTBEGIN(modernize-use-bool-literals)
// NOLINTBEGIN(readability-implicit-bool-conversion)

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _rb::hook;
static_assert(sizeof(hook) == sizeof(rb_tree_link));
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

static hook* get_parent(hook** const root, hook* const node)
{
	vsm_assert(node->parent.ptr() != root);
	return reinterpret_cast<hook*>(node->parent.ptr());
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
	root->parent.set_ptr(pivot->children);

	pivot->children[r] = root;
	pivot->parent.set_ptr(parent);

	if (child != nullptr)
	{
		child->parent.set_ptr(root->children);
	}
	parent[root != parent[0]] = pivot;
}


static bool is_black_or_null(hook const* const node)
{
	return node == nullptr || get_color(node) == black;
};

static void rebalance_after_insert(hook** const root, hook* node)
{
	while (true)
	{
		if (node->parent.ptr() == root)
		{
			return;
		}

		hook* const parent = get_parent(root, node);

		if (get_color(parent) == black)
		{
			return;
		}

		if (parent->parent.ptr() == root)
		{
			set_color(parent, black);
			return;
		}

		hook* const grandparent = get_parent(root, parent);
		bool const parent_side = parent != grandparent->children[0];
		hook* const parent_sibling = grandparent->children[!parent_side];

		if (is_black_or_null(parent_sibling))
		{
			if (node != parent->children[parent_side])
			{
				rotate(parent, !parent_side);
				node = parent;
			}

			rotate(grandparent, parent_side);

			set_color(get_parent(root, node), black);
			set_color(grandparent, red);

			return;
		}

		set_color(parent, black);
		set_color(parent_sibling, black);
		set_color(grandparent, red);

		node = grandparent;
	}
}

static void rebalance_after_erase(hook** const root, hook* node)
{
	while (true)
	{
		vsm_assert(node != nullptr);

		bool const node_l = node != node->parent[0];
		bool const node_r = node_l ^ 1;

		if (get_color(node) == red)
		{
			hook* const parent = get_parent(root, node);

			set_color(node, black);
			set_color(parent, red);

			rotate(parent, node_l);

			node = node->children[node_r]->children[node_l];
		}

		if (is_black_or_null(node->children[0]) && is_black_or_null(node->children[1]))
		{
			hook* const parent = get_parent(root, node);

			set_color(node, red);

			if (parent->parent.ptr() == root || get_color(parent) == red)
			{
				set_color(parent, black);
				break;
			}

			node = parent->parent[parent == parent->parent[0]];
		}
		else // node has a red child:
		{
			if (is_black_or_null(node->children[node_l]))
			{
				vsm_assert(!is_black_or_null(node->children[node_r]));

				set_color(node->children[node_r], black);
				set_color(node, red);

				rotate(node, node_r);

				node = get_parent(root, node);
			}

			vsm_assert(!is_black_or_null(node->children[node_l]));

			hook* const parent = get_parent(root, node);

			set_color(node, get_color(parent));
			set_color(parent, black);
			set_color(node->children[node_l], black);

			rotate(parent, node_l);

			break;
		}
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
}

void _rb::erase(hook* const node)
{
	// If node has two children, then hole is the successor of node, and otherwise it is node, but
	// in either case has at most one child.
	hook* const hole = node->children[0] == nullptr || node->children[1] == nullptr
		? node
		: leftmost(node->children[1], 0);

	bool const hole_side = hole != *hole->parent;

	// The possibly null only child of hole.
	hook* const hole_child = hole->children[0] != nullptr
		? hole->children[0]
		: hole->children[1];

	// The hole's possibly null sibling:
	hook* const hole_sibling = hole->parent.ptr() == &m_root
		? nullptr
		: hole->parent[hole_side ^ 1];

	// Replace hole with its potentially null only child, removing it from the tree:
	{
		if (hole_child != nullptr)
		{
			hole_child->parent.set_ptr(hole->parent.ptr());
		}

		hole->parent[hole_side] = hole_child;
	}


	bool const hole_color = get_color(hole);


	// If hole is the successor of the node to be erased, insert it back in place of node:
	if (hole != node)
	{
		// Because hole is different from node, and thus it is the in-order successor of node, node
		// originally had two children. However, it is possible that hole is a direct child of node,
		// in which case, when hole was removed from the tree, the right child of node became null.
		// In any case, the left child of node is definitely not null.
		vsm_assert(node->children[0] != nullptr);

		// Copy the color of node as well:
		hole->parent = node->parent;
		node->parent[node != *node->parent] = hole;

		hole->children[0] = node->children[0];
		node->children[0]->parent.set_ptr(hole->children);

		hole->children[1] = node->children[1];
		if (hook* const child = node->children[1])
		{
			child->parent.set_ptr(hole->children);
		}
	}

	if (hole_color == black && m_root != nullptr)
	{
		if (hole_child != nullptr)
		{
			// If hole was black and it had a child, we can simply color that child black:
			set_color(hole_child, black);
		}
		else
		{
			// The tree must now be rebalanced, starting at the sibling of hole. The sibling cannot
			// be null in this case, because the hole was black, meaning it was not the only child.
			rebalance_after_erase(&m_root, hole_sibling);
		}
	}

	--m_size;
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

// NOLINTEND(readability-implicit-bool-conversion)
// NOLINTEND(modernize-use-bool-literals)
