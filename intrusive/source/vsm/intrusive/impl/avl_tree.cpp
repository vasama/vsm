#include <vsm/intrusive/avl_tree.hpp>

#include <array>

#include <cmath>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _avl::hook;
static_assert(sizeof(hook) == sizeof(avl_tree_link));

template<typename T>
using ptr = _avl::ptr<T>;

static_assert(check_incomplete_tag_ptr<ptr<hook>>());


static hook* leftmost(hook* node, bool const l)
{
	while (node->children[l] != nullptr)
	{
		node = node->children[l].ptr();
	}
	return node;
}

// Rotate from left to right.
static void rotate(hook* const root, bool const l, bool const single, bool const root_balance)
{
	bool const r = !l;

	ptr<hook>* const parent = root->parent;
	hook* const pivot = root->children[l].ptr();
	hook* const child = pivot->children[r].ptr();

	bool const balance = !pivot->children[l].tag() & single;

	root->children[l].set(child, balance);
	root->children[r].set_tag(root_balance);
	root->parent = pivot->children;

	pivot->children[l].set_tag(0);
	pivot->children[r].set(root, balance);
	pivot->parent = parent;

	if (child != nullptr)
	{
		child->parent = root->children;
	}
	parent[root != parent->ptr()].set_ptr(pivot);
}

static void rebalance(ptr<hook>* const root, ptr<hook>* node, bool l, bool const insert)
{
	while (node != root)
	{
		l ^= !insert;
		bool const r = !l;

		// The node is not the root, so it is a hook.
		hook* const parent = vsm_detail_avl_hook_from_children(node);
		hook* new_parent = parent;

		// If the node is now +2 on the l side.
		if (parent->children[l].tag())
		{
			hook* const child = parent->children[l].ptr();

			new_parent = child;

			bool const double_rotation = child->children[r].tag();
			bool const removal_balance = child->children[l].tag();

			bool balance = 0;
			if (double_rotation)
			{
				hook* const pivot = child->children[r].ptr();
				balance = pivot->children[l].tag();

				// Rotate node from r to l to allow the
				// subsequent parent rotation to balance the parent.
				rotate(child, r, false, pivot->children[r].tag());

				new_parent = pivot;
			}

			// Rotate parent from l to r to balance.
			rotate(parent, l, !double_rotation, balance);

			// On insertion a single or double rotation always balances the tree.
			// On removal a single rotation balances the tree if the pivot is balanced.
			if (insert || double_rotation == removal_balance)
			{
				break;
			}
		}
		else
		{
			// The node is either 0 or +1 on the l side.
			// Update the node's balancing factors.
			bool const balance = parent->children[r].tag();

			// The l side becomes 1, or cancels out with the r side.
			parent->children[l].set_tag(parent->children[l].tag() | !balance);

			// The r side either is already 0, or it becomes 0.
			parent->children[r].set_tag(0);

			// If the r side is 1, the height of this subtree does not change.
			if (balance == insert)
			{
				break;
			}
		}

		node = new_parent->parent;
		l = node->ptr() != new_parent;
	}
}


ptr<hook> const* _avl::lower_bound(ptr<ptr<hook> const> const parent_and_side) const
{
	ptr<hook> const* const parent = parent_and_side.ptr();
	vsm_assert(parent[parent_and_side.tag()] == nullptr);

	if (parent_and_side.tag() == 0)
	{
		return parent;
	}

	hook* const sibling = parent[!parent_and_side.tag()].ptr();

	return sibling == nullptr
		? parent
		: leftmost(sibling, 0)->children;
}

void _avl::insert(hook* const node, ptr<ptr<hook>> const parent_and_side)
{
	vsm_assert(parent_and_side[parent_and_side.tag()] == nullptr);

	++m_size;

	ptr<hook>* const parent = parent_and_side.ptr();
	bool const l = parent_and_side.tag();

	node->children[0] = nullptr;
	node->children[1] = nullptr;
	node->parent = parent;
	parent[l].set_ptr(node);

	rebalance(&m_root, parent, l, true);
}

void _avl::erase(hook* const node)
{
	--m_size;

	ptr<hook>* const parent = node->parent;
	bool const l = parent->ptr() != node;

	ptr<hook>* balance_node = parent;
	bool balance_l = l;

	// If node is not a leaf.
	if (node->children[0] != nullptr || node->children[1] != nullptr)
	{
		// Higher side of the tree on the left.
		bool const succ_l = node->children[1].tag();
		bool const succ_r = !succ_l;

		hook* const l_child = node->children[succ_l].ptr();
		hook* const r_child = node->children[succ_r].ptr();

		// Find the in-order successor of node on the higher side of the tree.
		hook* successor = l_child;

		balance_node = l_child->children;
		balance_l = succ_l;

		if (l_child->children[succ_r] != nullptr)
		{
			successor = leftmost(l_child, succ_r);

			ptr<hook>* const succ_parent = successor->parent;
			hook* const succ_child = successor->children[succ_l].ptr();

			// Attach the successor's child to the successor's parent.
			succ_parent[succ_r].set_ptr(succ_child);
			if (succ_child != nullptr)
			{
				succ_child->parent = succ_parent;
			}

			// Attach node's direct child to the successor.
			successor->children[succ_l].set(l_child, node->children[succ_l].tag());
			l_child->parent = successor->children;

			balance_node = successor->parent;
			balance_l = succ_r;
		}

		successor->children[succ_l].set_tag(node->children[succ_l].tag());

		// Attach the node's other child to the successor.
		// tag is known to be zero on the lower side.
		successor->children[succ_r] = r_child;
		if (r_child != nullptr)
		{
			r_child->parent = successor->children;
		}

		// Attach the successor to the removed node's parent.
		parent[l].set_ptr(successor);
		successor->parent = parent;
	}
	else
	{
		parent[l].set_ptr(nullptr);
	}

	rebalance(&m_root, balance_node, balance_l, false);
}

void _avl::replace(hook* const existing_node, hook* const new_node)
{
	hook const node = *existing_node;
	*new_node = node;

	node.children[0]->parent = new_node->children;
	node.children[1]->parent = new_node->children;
	node.parent[node.parent->ptr() != existing_node].set_ptr(new_node);
}

void _avl::clear()
{
	if (m_root != nullptr)
	{
		ptr<hook>* children = leftmost(m_root.ptr(), 0)->children;

		while (children != &m_root)
		{
			if (children[1] != nullptr)
			{
				children = leftmost(children[1].ptr(), 0)->children;
			}
			else
			{
				hook* const node = vsm_detail_avl_hook_from_children(children);

				children = std::exchange(node->parent, nullptr);
				children[node != children[0].ptr()] = nullptr;
			}
		}

		m_root = nullptr;
		m_size = 0;
	}
}

_list::hook* _avl::flatten()
{
	hook* const root = m_root.ptr();

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
			// It is important that each non-zero child pointer is overwritten to reset tags.

			hook* const tail = node->children[l].ptr();
			hook* head = tail;

			while (head->children[r] != nullptr)
			{
				hook* const pivot = head->children[r].ptr();

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


ptr<hook>* _avl::iterator_begin(ptr<hook>* const root)
{
	hook* const node = root->ptr();
	return node != nullptr ? leftmost(node, 0)->children : root;
}

ptr<hook>* _avl::iterator_advance(ptr<hook>* const children, bool const l)
{
	bool const r = l ^ 1;

	// If node has a right child: stop at its leftmost descendant.
	if (children[r] != nullptr)
	{
		return leftmost(children[r].ptr(), l)->children;
	}

	hook* node = vsm_detail_avl_hook_from_children(children);

	// Iterate through ancestors until the branch where the node is the left child.
	while (node != node->parent->ptr())
	{
		node = vsm_detail_avl_hook_from_children(node->parent);
	}

	return node->parent;
}
