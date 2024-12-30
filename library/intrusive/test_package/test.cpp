#include <vsm/intrusive/avl_tree.hpp>

#include <cstdlib>

struct element : vsm::intrusive::avl_tree_link
{
	int x;

	explicit element(int const x)
		: x(x)
	{
	}
};

struct selector
{
	int operator()(element const& e) const
	{
		return e.x;
	}
};

int main()
{
	vsm::intrusive::avl_tree<element, selector> t;

	element a(1);
	element b(2);
	element c(3);

	t.insert(b);
	t.insert(c);
	t.insert(a);

	auto beg = t.begin();
	auto end = t.end();

	if (beg == end || beg++->x != 1) return EXIT_FAILURE;
	if (beg == end || beg++->x != 2) return EXIT_FAILURE;
	if (beg == end || beg++->x != 3) return EXIT_FAILURE;
	if (beg != end) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
