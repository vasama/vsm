#include <vsm/log.hpp>

#include <catch2/catch_all.hpp>

#include <format>

using namespace vsm;

namespace {

class test_logger_base
{
	log_level m_level = log_level::trace;

public:
	void set_level(log_level const level)
	{
		m_level = level;
	}

	[[nodiscard]] log_level get_level() const noexcept
	{
		return m_level;
	}
};

class test_logger : public test_logger_base
{
public:
	struct log_entry_type
	{
		std::string_view message;
		std::source_location location;
	};

private:
	std::string m_buffer;

public:
	void log(log_entry_type const& entry, auto const&... args)
	{
		std::format_to(
			std::back_inserter(m_buffer),
			"{}({}): ",
			entry.location.file_name(),
			entry.location.line());

		std::vformat_to(
			std::back_inserter(m_buffer),
			entry.message,
			std::make_format_args(args...));

		m_buffer.push_back('\n');
	}

	std::string flush()
	{
		return vsm_move(m_buffer);
	}
};

#if 0
TEST_CASE("logging", "[log]")
{
	test_logger logger;
	std::string expected;

	vsm_log(logger, vsm::log_level::debug, "x={}", 42);
	expected += std::format(__FILE__ "({}): x=42\n", __LINE__ - 1);

	CHECK(logger.flush() == expected);
}
#endif

} // namespace
