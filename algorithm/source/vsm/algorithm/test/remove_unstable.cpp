#include <vsm/algorithm/remove_unstable.hpp>

#include <catch2/catch_all.hpp>

#include <string_view>
#include <vector>

using namespace vsm;

namespace {

TEST_CASE("remove_unstable", "[algorithm]")
{
	static constexpr std::string_view string = "abcdefghijklmnopqrstuvxyz";
	static constexpr std::string_view remove = "a   ef     l n    s  v  z";
	static constexpr std::string_view expect = "ybcdxughijktmropq";

	std::string buffer = std::string(string);

	buffer.erase(
		remove_if_unstable(
			buffer.begin(),
			buffer.end(),
			[](char const x) { return remove.contains(x); }),
		buffer.end());

	CHECK(buffer == expect);
}

} // namespace

