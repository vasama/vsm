#include <vsm/intrusive_ptr.hpp>

#include <vsm/test/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct thing : intrusive_ref_count, test::instance_counter<thing> {};

using ptr = intrusive_ptr<thing>;

TEST_CASE("intrusive_ptr::intrusive_ptr()", "[intrusive_ptr]")
{
	{
		ptr p;
		CHECK(p == nullptr);
	}
	CHECK(test::instance_count<thing>() == 0);
}

TEST_CASE("intrusive_ptr::intrusive_ptr(T*)", "[intrusive_ptr]")
{
	{
		thing* const raw = new thing;

		ptr p(raw);
		CHECK(p.get() == raw);
	}
	CHECK(test::instance_count<thing>() == 0);
}

TEST_CASE("intrusive_ptr::intrusive_ptr(intrusive_ptr&&)", "[intrusive_ptr]")
{
	{
		thing* const raw = new thing;

		ptr src(raw);

		ptr p(std::move(src));
		CHECK(src == nullptr);
		CHECK(p.get() == raw);
	}
	CHECK(test::instance_count<thing>() == 0);
}

TEST_CASE("intrusive_ptr::intrusive_ptr(intrusive_ptr const&)", "[intrusive_ptr]")
{
	{
		thing* const raw = new thing;

		ptr src(raw);

		ptr p(src);
		CHECK(src.get() == raw);
		CHECK(p.get() == raw);
	}
	CHECK(test::instance_count<thing>() == 0);
}

TEST_CASE("intrusive_ptr::operator=(intrusive_ptr&&)", "[intrusive_ptr]")
{
	{
		thing* const raw = new thing;

		ptr src(raw);

		ptr p;
		p = std::move(src);
		CHECK(src == nullptr);
		CHECK(p.get() == raw);
	}
	CHECK(test::instance_count<thing>() == 0);
}

TEST_CASE("intrusive_ptr::operator=(intrusive_ptr const&)", "[intrusive_ptr]")
{
	{
		thing* const raw = new thing;

		ptr src(raw);

		ptr p;
		p = src;
		CHECK(src.get() == raw);
		CHECK(p.get() == raw);
	}
	CHECK(test::instance_count<thing>() == 0);
}

TEST_CASE("intrusive_ptr::acquire", "[intrusive_ptr]")
{
	{
		thing* const raw = new thing;

		(void)ptr(raw).release();
		(void)ptr::acquire(raw);
	}
	CHECK(test::instance_count<thing>() == 0);
}

} // namespace
