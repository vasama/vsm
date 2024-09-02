#include <vsm/main.hpp>

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
	try
	{
		auto const r = vsm_nothrow_main(argc, argv);

		if (r)
		{
			return *r;
		}

		report_error_code(r.error());
	}
	catch (std::system_error const& e)
	{
		report_error_code(e.code());
	}
	catch (std::bad_expected_access<std::error_code> const& e)
	{
		report_error_code(e.error());
	}
	catch (std::exception const& e)
	{
		std::print(stderr, "{}\n", e.what());
	}
	catch (...)
	{
		std::print(stderr, "Unhandled non-standard exception was encountered.\n");
	}

	fflush(stderr);
	return EXIT_FAILURE;
}
