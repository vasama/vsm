#include <vsm/assert.h>

#include <string_view>

#include <fcntl.h>
#include <unistd.h>

static bool is_process_traced()
{
	int const fd = open("/proc/self/status", O_RDONLY);

	if (fd == -1)
	{
		return false;
	}

	bool has_tracer_pid = false;
	
	while (true)
	{
		char buffer[256];

		ssize_t const read_size = read(fd, buffer, sizeof(buffer));

		if (read_size == -1)
		{
			break;
		}

		
	}

	close(fd);

	return has_tracer_pid;
}

extern "C"
bool vsm_assert_fail(char const* const file, int const line, char const* const expr) __attribute__((weak))
{
	if (is_process_traced())
	{
		return true;
	}

	
}
