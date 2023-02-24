#include <vsm/function.hpp>

int main()
{
	return vsm::function<int(int)>([](int x) { return x; })(0);
}
