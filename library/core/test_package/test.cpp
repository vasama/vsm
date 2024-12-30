#include <vsm/memory.hpp>

#include <cstdlib>

int main()
{
	int a = 1;
	int b = 2;

	vsm::memswap(&a, &b, sizeof(int));

	if (a != 2) return EXIT_FAILURE;
	if (b != 1) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
