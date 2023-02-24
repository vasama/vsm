#include <vsm/main.hpp>

vsm::result<int> vsm_nothrow_main(int const argc, char const* const* const argv)
{
	return vsm_main(argc, argv);
}
