#include <vsm/intrusive/heap.hpp>

#include <array>
#include <bit>
#include <limits>

#include <climits>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _heap::hook;
static_assert(sizeof(hook) == sizeof(heap_link));


static hook* leftmost(hook* node, bool const l)
{
	while (node->children[l] != nullptr)
	{
		node = node->children[l];
	}
	return node;
}

static std::pair<hook**, hook**> find_last(hook** const root, size_t const size)
{
	static constexpr size_t high_bit_index = sizeof(size_t) * CHAR_BIT - 1;

	vsm_assert(size > 0);

	hook** parent = root;
	hook** child = root;

	if (size > 1)
	{
		for (size_t i = high_bit_index - std::countl_zero(size); i-- > 0;)
		{
			parent = (*child)->children;
			child = parent + (size >> i & 1);
		}
	}

	return { parent, child };
}

static void swap_nodes(hook* const parent, hook* const child)
{
	vsm_assert(child->parent == parent->children);

	// Attach child to grandparent.
	hook** const grandparent = parent->parent;
	grandparent[grandparent[0] != parent] = child;

	// Attach parent to grandchildren.
	for (hook* const grandchild : child->children)
	{
		if (grandchild != nullptr)
		{
			grandchild->parent = parent->children;
		}
	}

	bool const side = parent->children[0] != child;

	// Attach child to its sibling as a parent.
	if (hook* const sibling = parent->children[side ^ 1])
	{
		sibling->parent = child->children;
	}

	// swap_nodes the children of parent and child.
	std::swap(parent->children, child->children);

	// Attach parent to child as a child on the appropriate side.
	child->children[side] = parent;

	// Attach child to parent as a parent.
	parent->parent = child->children;

	// attach grandparent to child as a parent.
	child->parent = grandparent;
}

// Walk towards the root and restore the heap property along the way.
static void percolate_to_root(
	_heap const& self,
	hook* const node,
	_heap::comparator* const comparator)
{
	while (true)
	{
		hook** const parent = node->parent;

		// When the min element is reached, the heap property is restored.
		if (parent == &self.m_root)
		{
			break;
		}

		// If the parent is not the root, it is a hook.
		hook* const parent_node = reinterpret_cast<hook*>(parent);

		// If the last node is not less than its parent, the heap property is restored.
		if (!comparator(self, parent_node, node))
		{
			break;
		}

		// swap_nodes last with its parent.
		swap_nodes(parent_node, node);
	}
}

[[maybe_unused]] static bool invariant(_heap const& self)
{
	size_t size = 0;
	if (hook const* node = self.m_root)
	{
		int8_t min_height = std::numeric_limits<int8_t>::max();
		int8_t max_height = 0;

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

			for (uint8_t visit; (visit = frame.visit++) < 2;)
			{
				frame.l_height = std::exchange(r_height, static_cast<int8_t>(0));

				if (const hook* const child = node->children[visit])
				{
					if (child->parent != node->children)
					{
						return false;
					}

					node = child;
					stack[++height].visit = 0;
					goto next_iteration;
				}
			}

			int8_t const l_height = frame.l_height;
			if (l_height < r_height)
			{
				return false;
			}

			if (l_height == 0)
			{
				min_height = std::min(min_height, height);
				max_height = std::max(max_height, height);
			}

			r_height = std::max(l_height, r_height) + 1;

			node = reinterpret_cast<const hook*>(node->parent);
			--height;

			++size;
		}

		vsm_assert(min_height <= max_height);
		if (max_height - min_height > 1)
		{
			return false;
		}
	}
	return size == self.m_size;
}


void _heap::push(hook* const node, comparator* const comparator)
{
	// Find the parent of, and the pointer to the last node.
	auto const [last_parent, last_parent_child] = find_last(&m_root, ++m_size);

	// Clear node's children and attach its new parent.
	node->children[0] = nullptr;
	node->children[1] = nullptr;
	node->parent = last_parent;

	// Attach node to its new parent.
	*last_parent_child = node;

	// Percolate node toward the root.
	percolate_to_root(*this, node, comparator);

	//vsm_assert_slow(invariant(*this));
}

void _heap::remove(hook* const node, comparator* const comparator)
{
	// Find the pointer to the last node.
	hook** const last_parent_child = find_last(&m_root, m_size--).second;

	// Remove the last node from the tree.
	hook* const last = std::exchange(*last_parent_child, nullptr);

	// If the last node is being removed, exit.
	if (last == node)
	{
		return;
	}

	// Replace the node being removed with the last node.
	*last = *node;

	// Attach last to node's children as parent.
	for (hook* const child : node->children)
	{
		if (child != nullptr)
		{
			child->parent = last->children;
		}
	}

	// Attach last to node's parent as a child.
	node->parent[node->parent[0] != node] = last;

	// Walk towards the leaves and restore the heap property along the way.
	while (true)
	{
		hook* max = last;

		// Find the maximum of last and its children.
		for (hook* const child : last->children)
		{
			if (child != nullptr && comparator(*this, max, child))
			{
				max = child;
			}
		}

		// If the last is ordered before its children, the heap property is restored.
		if (last == max)
		{
			break;
		}

		// swap_nodes last with the maximum of its children.
		swap_nodes(last, max);
	}

	percolate_to_root(*this, last, comparator);

	//vsm_assert_slow(invariant(*this));
}

hook* _heap::pop(comparator* const comparator)
{
	vsm_assert(m_size > 0);
	hook* const node = m_root;
	remove(node, comparator);
	return node;
}

void _heap::clear()
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
				hook* const node = vsm_detail_heap_hook_from_children(children);

				children = std::exchange(node->parent, nullptr);
				children[node != children[0]] = nullptr;
			}
		}

		m_root = nullptr;
		m_size = 0;
	}

	//vsm_assert_slow(invariant(*this));
}
