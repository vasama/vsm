#include <vsm/shared_string.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

static constexpr std::string_view lorem_ipsum =
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
	"sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";


static std::string_view generate_string()
{
	return GENERATE
	(
		as<std::string_view>()
		, ""
		, "1"
		, "small"
		, lorem_ipsum
	);
}

static shared_string copy_or_borrow(std::string_view const string)
{
	return GENERATE(1, 0)
		? shared_string(string)
		: shared_string::borrow(string);
}

static shared_string copy_or_share(shared_string const& string)
{
	if (GENERATE(1, 0))
	{
		return std::string_view(string);
	}
	else
	{
		return string;
	}
}

template<typename OutString, typename InString>
static OutString copy_or_move(InString& string)
{
	if (GENERATE(1, 0))
	{
		return OutString(string);
	}
	else
	{
		return vsm_move(string);
	}
}


TEST_CASE("shared_string construction", "[shared_string]")
{
	std::string_view const string = generate_string();
	shared_string const shared = copy_or_borrow(string);
	CHECK(shared == string);
	CHECK(shared[shared.size()] == '\0');
}

TEST_CASE("shared_string mutation", "[shared_string]")
{
	shared_string shared = copy_or_borrow(generate_string());
	shared_string const shared_copy = copy_or_share(shared);

	unique_string unique = vsm_move(shared);
	REQUIRE(unique == shared_copy);

	unique.append(generate_string());
	auto const mutated = std::string(unique);
	REQUIRE(unique == mutated);

	shared = copy_or_move<shared_string>(unique);
	REQUIRE(shared == mutated);
}
