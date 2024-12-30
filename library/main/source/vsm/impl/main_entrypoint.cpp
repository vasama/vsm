#include <vsm/main.hpp>

#include <vsm/exceptions.hpp>

#include <print>

static void report_error_code(std::error_code const e)
{
	std::print(
		stderr,
		"{}: {:08X} ({})\n",
		e.category().name(),
		static_cast<unsigned>(e.value()),
		e.message());
}

int vsm_main_entrypoint(int const argc, char const* const* const argv)
{
	vsm_except_try
	{
		auto const r = vsm_nothrow_main(argc, argv);

		if (r)
		{
			return *r;
		}

		report_error_code(r.error());
	}
	vsm_except_catch (std::system_error const& e)
	{
		report_error_code(e.code());
	}
	vsm_except_catch (std::bad_expected_access<std::error_code> const& e)
	{
		report_error_code(e.error());
	}
	vsm_except_catch (std::exception const& e)
	{
		std::print(stderr, "{}\n", e.what());
	}
	vsm_except_catch (...)
	{
		std::print(stderr, "Unhandled non-standard exception was encountered.\n");
	}

	return EXIT_FAILURE;
}
