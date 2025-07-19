#include <vsm/cli.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

TEST_CASE("Subcommands are accepted", "[cli]")
{
	auto const app = cli::make_app("test");
	cli::app& sub = app->command("sub");

	std::string value;
	sub.option("--test").set(value);

	std::string_view const arguments[] =
	{
		"sub",
		"--test",
		"value",
	};

	app->parse(arguments).value();

	REQUIRE(value == "value");
}

TEST_CASE("Usage without required subcommand is rejected", "[cli]")
{
	auto const app = cli::make_app("test");

	app->command("sub");
	app->require_command();

	auto const r = app->parse(std::span<std::string_view>());
	REQUIRE(!r);
	REQUIRE(r.error() == cli::error::command_not_given);
}

TEST_CASE("Positional options are accepted", "[cli]")
{
	auto const app = cli::make_app("test");

	std::vector<std::string> values;
	app->option("test").push_back(values);

	std::string_view const arguments[] =
	{
		"hello",
		"world",
	};

	app->parse(arguments).value();

	REQUIRE(values.size() == 2);
	CHECK(values[0] == "hello");
	CHECK(values[1] == "world");
}

TEST_CASE("Short options are accepted", "[cli]")
{
	auto const app = cli::make_app("test");

	std::string flag_value;
	app->flag("-f").set(flag_value);

	std::vector<std::string> values;
	app->option("-t").push_back(values);

	std::string first_argument = "-";
	std::vector<std::string_view> arguments;

	std::string expected_flag_value = "";

	if (GENERATE(0, 1))
	{
		first_argument += "f";
		expected_flag_value = "true";
	}

	first_argument += "t";
	if (GENERATE(0, 1))
	{
		first_argument += "hello";
	}
	else
	{
		arguments.push_back("hello");
	}

	arguments.insert(arguments.begin(), first_argument);

	app->parse(arguments).value();

	REQUIRE(flag_value == expected_flag_value);

	REQUIRE(values.size() == 1);
	CHECK(values[0] == "hello");
}

TEST_CASE("Unrecognized short options are rejected", "[cli]")
{
	auto const app = cli::make_app("test");

	std::string_view const arguments[] =
	{
		"-t"
	};

	auto const r = app->parse(arguments);
	REQUIRE(!r);
	REQUIRE(r.error() == cli::error::unrecognized_option);
}

TEST_CASE("Long options are accepted", "[cli]")
{
	auto const app = cli::make_app("test");

	std::vector<std::string> values;
	app->option("--test").push_back(values);

	std::vector<std::string_view> arguments;

	SECTION("Separate arguments")
	{
		arguments.push_back("--test");
		arguments.push_back("hello");
	}

	SECTION("Single argument with equals")
	{
		arguments.push_back("--test=hello");
	}

	app->parse(arguments).value();

	REQUIRE(values.size() == 1);
	CHECK(values[0] == "hello");
}

TEST_CASE("Unrecognized long options are rejected", "[cli]")
{
	auto const app = cli::make_app("test");

	std::string_view const arguments[] =
	{
		"--test"
	};

	auto const r = app->parse(arguments);
	REQUIRE(!r);
	REQUIRE(r.error() == cli::error::unrecognized_option);
}

TEST_CASE("Options without a value are rejected", "[cli]")
{
	auto const app = cli::make_app("test");

	app->option("--test");

	std::string_view const arguments[] =
	{
		"--test"
	};

	auto const r = app->parse(arguments);
	REQUIRE(!r);
	REQUIRE(r.error() == cli::error::invalid_syntax);
}

TEST_CASE("Flags are accepted without a value", "[cli]")
{
	auto const app = cli::make_app("test");

	std::string_view const name = GENERATE("-t", "--test");

	std::string spec = std::string(name);
	std::string expected_value = "true";

	if (GENERATE(0, 1))
	{
		spec += "=false";
		expected_value = "false";
	}

	std::string value;
	app->flag(spec).set(value);

	std::string_view const arguments[] =
	{
		name
	};

	app->parse(arguments).value();

	REQUIRE(value == expected_value);
}

TEST_CASE("Option syntax is treated as positional after --", "[cli]")
{
	auto const app = cli::make_app("test");

	std::string const option_name = GENERATE("-t", "--test");

	std::vector<std::string> values;
	app->option("positional").push_back(values);
	app->option(option_name);

	std::string_view const arguments[] =
	{
		"first",
		"--",
		option_name,
	};

	app->parse(arguments).value();

	REQUIRE(values.size() == 2);
	REQUIRE(values[0] == "first");
	REQUIRE(values[1] == option_name);
}

} // namespace
