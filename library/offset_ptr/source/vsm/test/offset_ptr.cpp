#include <vsm/offset_ptr.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

static void magic_memcpy(void* const dst, void const* const src, size_t const size)
{
	static constinit void*       volatile s_dst = nullptr;
	static constinit void const* volatile s_src = nullptr;

	s_dst = dst;
	s_src = src;

	std::memcpy(s_dst, s_src, size);

	s_dst = nullptr;
	s_src = nullptr;
}

struct s
{
	offset_ptr<s> peer;
};

struct pair
{
	s a;
	s b;
};

TEST_CASE("offset_ptr memcpy", "[offset_ptr]")
{
	pair a;
	a.a.peer = &a.b;
	a.b.peer = &a.a;

	pair b;
	magic_memcpy(&b, &a, sizeof(pair));

	REQUIRE(b.a.peer.get() == &b.b);
	REQUIRE(b.b.peer.get() == &b.a);
}

} // namespace
