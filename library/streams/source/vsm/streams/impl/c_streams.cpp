#include <vsm/streams/c_streams.hpp>

#include <cstdio>

using namespace vsm;
using namespace vsm::streams;

vsm::result<size_t> streams::detail::c_write_some(
	FILE* const file,
	std::span<std::byte const> const data)
{
	if (data.size() == 0)
	{
		return 0;
	}

	size_t const transferred = std::fwrite(data.data(), 1, data.size(), file);

	if (transferred == 0)
	{
		return vsm::unexpected(std::error_code(errno, std::generic_category()));
	}

	return transferred;
}

vsm::result<void> streams::detail::c_flush(FILE* const file)
{
	if (std::fflush(file) != 0)
	{
		return vsm::unexpected(std::error_code(errno, std::generic_category()));
	}

	return {};
}

vsm::result<size_t> streams::detail::c_read_some(
	FILE* const file,
	std::span<std::byte> const data)
{
	if (data.size() == 0)
	{
		return 0;
	}

	size_t const transferred = std::fread(data.data(), 1, data.size(), file);

	if (transferred == 0)
	{
		return vsm::unexpected(std::error_code(errno, std::generic_category()));
	}

	return transferred;
}


c_source streams::cin(stdin);
c_sink streams::cout(stdout);
c_sink streams::cerr(stderr);
