#include <vsm/base64_streaming.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

static std::pair<std::string, std::string> get_test_data()
{
	std::pair<std::string, std::string> pair;
	auto& [source, target] = pair;

	SECTION("''")
	{
	}

	SECTION("'Hello, world!!!'")
	{
		source = "Hello, world!!!";
		target = "SGVsbG8sIHdvcmxkISEh";
	}

	SECTION("'Single padding'")
	{
		source = "Single padding";
		target = "U2luZ2xlIHBhZGRpbmc=";
	}

	SECTION("'Exactly 13...'")
	{
		source = "Exactly 13...";
		target = "RXhhY3RseSAxMy4uLg==";
	}

	return pair;
}

TEST_CASE("base64_encoder", "[base64]")
{
	auto [source, target] = get_test_data();

	size_t const slice = GENERATE(0u, 1u, 2u, 3u);
	auto const encode_into = [&]<typename OutputIterator>(OutputIterator out) -> OutputIterator
	{
		base64_encoder encoder;

		for (char const x : std::string_view(source).substr(0, slice))
		{
			out = encoder.encode_one<char>(x, out);
		}

		if (source.size() > slice)
		{
			out = encoder.encode<char>(std::string_view(source).substr(slice), out).out;
		}

		auto const r = encoder.finalize<char>(out);
		REQUIRE(r.completed);

		return r.out;
	};

	std::string output;
	encode_into(std::back_inserter(output));
	CHECK(output == target);

	std::ranges::fill(output, '\0');
	{
		output.push_back('\0');
		auto const end = encode_into(output.data());

		REQUIRE(output.back() == '\0');
		output.pop_back();

		REQUIRE(end == output.data() + output.size());
	}
	CHECK(output == target);
}

TEST_CASE("base64_decoder", "[base64]")
{
	auto [source, target] = get_test_data();
	source.swap(target);

	size_t const slice = GENERATE(0u, 1u, 2u, 3u, 4u);
	auto const decode_into = [&]<typename OutputIterator>(OutputIterator out) -> OutputIterator
	{
		base64_decoder decoder;

		for (char const x : std::string_view(source).substr(0, slice))
		{
			out = decoder.decode_one<char>(x, out);
		}

		if (source.size() > slice)
		{
			out = decoder.decode<char>(std::string_view(source).substr(slice), out).out;
		}

		return out;
	};

	std::string output;
	decode_into(std::back_inserter(output));
	CHECK(output == target);

	std::ranges::fill(output, '\0');
	{
		output.push_back('\0');
		auto const end = decode_into(output.data());

		REQUIRE(output.back() == '\0');
		output.pop_back();

		REQUIRE(end == output.data() + output.size());
	}
	CHECK(output == target);
}

} // namespace
