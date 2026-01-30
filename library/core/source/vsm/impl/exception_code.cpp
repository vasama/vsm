#include <vsm/exception_code.hpp>

using namespace vsm;

char const* exception_category::name() const noexcept
{
	return "exception_category";
}

std::string exception_category::message(int const code) const
{
	switch (static_cast<exception_code>(code))
	{
	case exception_code::none:
		return "none";

	case exception_code::bad_alloc:
		return "bad_alloc";

	default:
		return "?";
	}
}

exception_category const detail::exception_category_instance;


void vsm::throw_error_code(std::error_code const error)
{
	if (error == std::error_code(exception_code::bad_alloc))
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}
	else
	{
		vsm_except_throw_or_terminate(std::system_error(error));
	}
}
