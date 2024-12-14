#include <vsm/intrusive/wb_tree.hpp>

#include <array>

// NOLINTBEGIN(modernize-use-bool-literals)
// NOLINTBEGIN(readability-implicit-bool-conversion)

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _wb::hook;
static_assert(sizeof(hook) == sizeof(wb_tree_link));


template<typename T>
using ptr = _wb::ptr<T>;

static_assert(check_incomplete_tag_ptr<ptr<hook>>());


static constexpr size_t delta = 4;
static constexpr size_t ratio = 2;


static size_t weight(hook* const root)
{
	return root != nullptr ? root->weight : 0;
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

	hook** const parent = root->parent;
	hook* const pivot = root->children[l];
	hook* const child = pivot->children[r];

	size_t const root_weight = root->weight;
	size_t const pivot_weight = pivot->weight;
	size_t const child_weight = weight(child);

	root->children[l] = child;
	root->parent = pivot->children;

	pivot->children[r] = root;
	pivot->parent = parent;

	if (child != nullptr)
	{
		child->parent = root->children;
	}
	parent[root != parent[0]] = pivot;

	root->weight += child_weight - pivot_weight;
	pivot->weight += root_weight - pivot_weight;
}

static void rebalance(hook** const root, hook** node, bool l, bool const insert)
{
	size_t const weight_increment = insert
		? 1
		: static_cast<size_t>(-1);

	while (node != root)
	{
		l ^= !insert;
		bool const r = !l;

		hook* const parent = vsm_detail_wb_hook_from_children(node);
		parent->weight += weight_increment;

		hook* new_parent = parent;

		hook* const l_child = parent->children[l];
		hook* const r_child = parent->children[r];

		if (parent->weight > 2 && weight(l_child) >= weight(r_child) * delta)
		{
			new_parent = l_child;

			hook* const l_l_child = l_child->children[l];
			hook* const l_r_child = l_child->children[r];

			if (weight(l_r_child) >= weight(l_l_child) * ratio)
			{
				rotate(l_child, r);

				// Inner rotation pivot becomes the new subtree root.
				new_parent = l_r_child;
			}

			rotate(parent, l);
		}

		node = new_parent->parent;
		l = *node != new_parent;
	}
}


hook* _wb::select(size_t rank) const
{
	hook* node = m_root;
	vsm_assert(node != nullptr);

	while (true)
	{
		vsm_assert(rank < node->weight);

		size_t const left_weight = weight(node->children[0]);
		if (left_weight == rank)
		{
			return node;
		}

		node = node->children[rank > left_weight];
		if (rank > left_weight)
		{
			rank -= left_weight;
		}
	}
}

size_t _wb::rank(hook const* node) const
{
	size_t rank = 0;
	while (node->parent != &m_root)
	{
		hook const* const parent = vsm_detail_wb_hook_from_children(node->parent);

		if (node != parent->children[0])
		{
			rank += weight(parent->children[0]);
		}

		node = parent;
	}
	return rank;
}

void _wb::insert(hook* const node, ptr<hook*> const parent_and_side)
{
	vsm_assert(parent_and_side[parent_and_side.tag()] == nullptr);

	hook** const parent = parent_and_side.ptr();
	bool const l = parent_and_side.tag();

	node->children[0] = nullptr;
	node->children[1] = nullptr;
	node->parent = parent;
	node->weight = 1;
	parent[l] = node;

	rebalance(&m_root, parent, l, true);
}

void _wb::erase(hook* const node)
{
	hook** const parent = node->parent;
	bool const l = *parent != node;

	hook** balance_node = parent;
	bool balance_l = l;

	hook* l_child = node->children[0];
	hook* r_child = node->children[1];

	// If node is not a leaf.
	if (l_child != nullptr || r_child != nullptr)
	{
		// Heavier side of the tree on the left.
		bool const succ_l = weight(r_child) > weight(l_child);
		bool const succ_r = !succ_l;

		if (succ_l)
		{
			std::swap(l_child, r_child);
		}

		// Find the in-order successor of the node on the heavier side of the tree.
		hook* successor = l_child;

		balance_node = l_child->children;
		balance_l = succ_l;

		// Clang thinks l_child might be null because we test for either l_child or r_child,
		// however in the case that l_child is null, the two are swapped due to the non-null child
		// always having a greater weight than the null child.
		// NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
		if (l_child->children[succ_r] != nullptr)
		{
			successor = leftmost(l_child, succ_r);

			hook** const succ_parent = successor->parent;
			hook* const succ_child = successor->children[succ_l];

			// Attach the successor's child to the successor's parent.
			succ_parent[succ_r] = succ_child;
			if (succ_child != nullptr)
			{
				succ_child->parent = succ_parent;
			}

			// Attach node's direct child to the successor.
			successor->children[succ_l] = l_child;
			l_child->parent = successor->children;

			balance_node = successor->parent;
			balance_l = succ_r;
		}

		successor->weight = node->weight;

		// Attach the node's other child to the successor.
		successor->children[succ_r] = r_child;
		if (r_child != nullptr)
		{
			r_child->parent = successor->children;
		}

		// Attach the successor to the removed node's parent.
		parent[l] = successor;
		successor->parent = parent;
	}
	else
	{
		parent[l] = nullptr;
	}

	rebalance(&m_root, balance_node, balance_l, false);
}

void _wb::clear()
{
	if (m_root != nullptr)
	{
		hook** children = leftmost(m_root, 0)->children;

		while (children != &m_root)
		{
			if (children[1] != nullptr)
			{
				children = leftmost(children[1], 0)->children;
			}
			else
			{
				hook* const node = vsm_detail_wb_hook_from_children(children);

				children = std::exchange(node->parent, nullptr);
				children[node != children[0]] = nullptr;
			}
		}

		m_root = nullptr;
	}
}

_list::hook* _wb::flatten()
{
	hook* const root = m_root;

	if (root == nullptr)
	{
		return nullptr;
	}

	m_root = nullptr;

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


hook** _wb::iterator_begin(hook** const root)
{
	hook* const node = *root;
	return node != nullptr ? leftmost(node, 0)->children : root;
}

hook** _wb::iterator_advance(hook** const children, bool const l)
{
	bool const r = !l;

	// If node has a right child: stop at its leftmost descendant.
	if (children[r] != nullptr)
	{
		return leftmost(children[r], l)->children;
	}

	hook* node = vsm_detail_wb_hook_from_children(children);

	// Iterate through ancestors until the branch where the node is the left child.
	while (node != *node->parent)
	{
		node = vsm_detail_wb_hook_from_children(node->parent);
	}

	return node->parent;
}

// NOLINTEND(readability-implicit-bool-conversion)
// NOLINTEND(modernize-use-bool-literals)
