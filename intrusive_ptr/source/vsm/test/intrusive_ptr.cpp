#include <vsm/intrusive_ptr.hpp>

#include <vsm/test/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct thing : intrusive_refcount, test::counted {};

using ptr = intrusive_ptr<thing>;

TEST_CASE("intrusive_ptr::intrusive_ptr()", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		ptr const p;
		CHECK(p == nullptr);
	}
	CHECK(instance_count.empty());
}

TEST_CASE("intrusive_ptr::intrusive_ptr(T*)", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		thing* const raw = new thing;

		ptr const p(raw);
		CHECK(p.get() == raw);
	}
	CHECK(instance_count.empty());
}

TEST_CASE("intrusive_ptr::intrusive_ptr(intrusive_ptr&&)", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		thing* const raw = new thing;

		ptr src(raw);

		ptr const p(std::move(src));
		CHECK(src == nullptr); // NOLINT(bugprone-use-after-move)
		CHECK(p.get() == raw);
	}
	CHECK(instance_count.empty());
}

TEST_CASE("intrusive_ptr::intrusive_ptr(intrusive_ptr const&)", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		thing* const raw = new thing;

		ptr const src(raw);

		ptr const p(src);
		CHECK(src.get() == raw);
		CHECK(p.get() == raw);
	}
	CHECK(instance_count.empty());
}

TEST_CASE("intrusive_ptr::operator=(intrusive_ptr&&)", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		thing* const raw = new thing;

		ptr src(raw);

		ptr p;
		p = std::move(src);
		CHECK(src == nullptr); // NOLINT(bugprone-use-after-move)
		CHECK(p.get() == raw);
	}
	CHECK(instance_count.empty());
}

TEST_CASE("intrusive_ptr::operator=(intrusive_ptr const&)", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		thing* const raw = new thing;

		ptr const src(raw);

		ptr p;
		p = src;
		CHECK(src.get() == raw);
		CHECK(p.get() == raw);
	}
	CHECK(instance_count.empty());
}

TEST_CASE("intrusive_ptr::adopt", "[intrusive_ptr]")
{
	test::scoped_count const instance_count;
	{
		thing* const raw = new thing;

		(void)ptr(raw).release();
		(void)ptr::adopt(raw);
	}
	CHECK(instance_count.empty());
}

} // namespace
