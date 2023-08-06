#include <vsm/intrusive/avl_tree.hpp>

#include <array>

#include <cmath>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail::avl_tree_;

namespace tree_namespace = vsm::intrusive::detail::avl_tree_;
namespace list_namespace = vsm::intrusive::detail::list_;


static_assert(sizeof(hook) == sizeof(avl_tree_link));
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

static bool invariant(base const& self)
{
	if (self.m_root.tag() != 0)
	{
		return false;
	}

	size_t size = 0;
	if (hook const* node = self.m_root.ptr())
	{
		struct frame
		{
			uint8_t visit : 2;
			uint8_t l_height : 6;
		};

		frame stack[sizeof(size_t) * CHAR_BIT];
		static_assert(std::size(stack) <= 1 << 6);

		int8_t height = 0;
		stack[0].visit = 0;
		int8_t r_height = 0;

		while (height >= 0)
		{
		next_iteration:
			frame& frame = stack[height];

			for (int8_t visit; (visit = frame.visit++) < 2;)
			{
				frame.l_height = std::exchange(r_height, 0);

				if (hook const* const child = node->children[visit].ptr())
				{
					if (child->parent != node->children)
					{
						return false;
					}

					node = child;
					stack[++height].visit = 0;
					goto next_iteration;
				}

				if (node->children[visit].tag())
				{
					return false;
				}
			}

			int8_t const l_height = frame.l_height;

			if (std::abs(l_height - r_height) > 1)
			{
				return false;
			}
			if (node->children[0].tag() != (l_height > r_height))
			{
				return false;
			}
			if (node->children[1].tag() != (r_height > l_height))
			{
				return false;
			}

			r_height = std::max(l_height, r_height) + 1;

			node = vsm_detail_avl_hook_from_children(node->parent);
			--height;

			++size;
		}
	}

	return size == self.m_size;
}


void base::insert(hook* const node, ptr<ptr<hook>> const parent_and_side)
{
	vsm_intrusive_link_insert(*this, *node);

	++m_size;

	ptr<hook>* const parent = parent_and_side.ptr();
	bool const l = parent_and_side.tag();

	node->children[0] = nullptr;
	node->children[1] = nullptr;
	node->parent = parent;
	parent[l].set_ptr(node);

	rebalance(&m_root, parent, l, true);

	vsm_assert_slow(invariant(*this));
}

void base::remove(hook* const node)
{
	vsm_intrusive_link_remove(*this, *node);

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

	vsm_assert_slow(invariant(*this));
}

void base::clear()
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

				vsm_intrusive_link_remove(*this, *node);
			}
		}

		m_root = nullptr;
		m_size = 0;
	}

	vsm_assert_slow(invariant(*this));
}

list_namespace::hook* base::flatten()
{
	hook* root = m_root.ptr();

	if (root != nullptr)
	{
		m_root = nullptr;
		m_size = 0;

		auto const flatten_side = [](hook* node, bool const l) -> hook*
		{
			bool const r = !l;

			while (node->children[l] == nullptr)
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

			return node;
		};

		hook* const head = flatten_side(root, 0);
		hook* const tail = flatten_side(root, 1);

		head->children[0] = tail;
		tail->children[1] = head;

		root = head;
	}

	vsm_assert_slow(invariant(*this));

	return reinterpret_cast<list_::hook*>(root);
}


ptr<hook>* tree_namespace::iterator_begin(ptr<hook>* const root)
{
	hook* const node = root->ptr();
	return node != nullptr ? leftmost(node, 0)->children : root;
}

ptr<hook>* tree_namespace::iterator_advance(ptr<hook>* const children, bool const l)
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
