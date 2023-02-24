#include <vsm/error_code.hpp>

using namespace vsm;

bool error_category::equivalent(int const code, error_condition const& condition) const noexcept
{
	return false;
}

bool error_category::equivalent(error_code const& code, int const condition) const noexcept
{
	return false;
}

error_condition error_category::default_error_condition(int const code) const noexcept
{
	return error_condition();
}

bool error_category::get_std_error_code(int const code, std_error_code& out) const noexcept
{
	return false;
}
