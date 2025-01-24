#include <vsm/unique_resource.hpp>
#include <vsm/out_resource.hpp>

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <tuple>
#include <vector>

using namespace vsm;

namespace {

using resource = size_t;
static constexpr resource null = static_cast<size_t>(-1);


class dispenser
{
	std::vector<bool> m_resources;

public:
	dispenser()
	{
		if (s_global == nullptr)
		{
			s_global = this;
		}
	}

	dispenser(dispenser const&) = delete;
	dispenser& operator=(dispenser const&) = delete;

	~dispenser()
	{
		if (s_global == this)
		{
			s_global = nullptr;
		}

		CHECK(std::ranges::all_of(m_resources, std::identity()));
	}

	resource acquire()
	{
		size_t const resource = m_resources.size();
		m_resources.push_back(false);
		return resource;
	}

	void release(resource const resource)
	{
		CHECK(!m_resources[resource]);
		m_resources[resource] = true;
	}

	static dispenser* global()
	{
		return s_global;
	}

private:
	static dispenser* s_global;
};

dispenser* dispenser::s_global = nullptr;


class stateless_deleter
{
public:
	stateless_deleter(dispenser& dispenser)
	{
		REQUIRE(&dispenser == dispenser.global());
	}

	void operator()(resource const resource) const
	{
		dispenser::global()->release(resource);
	}
};

class trivial_stateful_deleter
{
protected:
	dispenser* m_dispenser;

public:
	trivial_stateful_deleter(dispenser& dispenser)
		: m_dispenser(&dispenser)
	{
	}

	void operator()(resource const resource) const
	{
		m_dispenser->release(resource);
	}
};

class non_trivial_stateful_deleter : public trivial_stateful_deleter
{
public:
	using trivial_stateful_deleter::trivial_stateful_deleter;

	non_trivial_stateful_deleter(non_trivial_stateful_deleter&& src) noexcept
		: trivial_stateful_deleter(src)
	{
		src.m_dispenser = nullptr;
	}

	non_trivial_stateful_deleter& operator=(non_trivial_stateful_deleter&& src) & noexcept
	{
		m_dispenser = src.m_dispenser;
		src.m_dispenser = nullptr;
		return *this;
	}
};


using unique_resource_types = std::tuple
<
	unique_resource<resource, stateless_deleter>,
	unique_resource<resource, stateless_deleter, null>,

	unique_resource<resource, trivial_stateful_deleter>,
	unique_resource<resource, trivial_stateful_deleter, null>,

	unique_resource<resource, non_trivial_stateful_deleter>,
	unique_resource<resource, non_trivial_stateful_deleter, null>
>;

TEMPLATE_LIST_TEST_CASE("unique_resource construction", "[unique_resource]", unique_resource_types)
{
	dispenser dispenser;

	bool const init = GENERATE(0, 1);

	TestType r = init
		? TestType(dispenser.acquire(), dispenser)
		: TestType(null_resource, dispenser);

	CHECK(static_cast<bool>(r) == init);
}

TEMPLATE_LIST_TEST_CASE("unique_resource move construction", "[unique_resource]", unique_resource_types)
{
	dispenser dispenser;

	bool const init = GENERATE(0, 1);

	TestType r1 = init
		? TestType(dispenser.acquire(), dispenser)
		: TestType(null_resource, dispenser);

	TestType r2 = std::move(r1);

	CHECK(!static_cast<bool>(r1)); // NOLINT(bugprone-use-after-move)
	CHECK(static_cast<bool>(r2) == init);
}

TEMPLATE_LIST_TEST_CASE("unique_resource move assignment", "[unique_resource]", unique_resource_types)
{
	dispenser dispenser;

	bool const init = GENERATE(0, 1);

	TestType r1 = init
		? TestType(dispenser.acquire(), dispenser)
		: TestType(null_resource, dispenser);

	TestType r2 = GENERATE(0, 1)
		? TestType(dispenser.acquire(), dispenser)
		: TestType(null_resource, dispenser);

	r2 = std::move(r1);

	CHECK(!static_cast<bool>(r1)); // NOLINT(bugprone-use-after-move)
	CHECK(static_cast<bool>(r2) == init);
}

TEMPLATE_LIST_TEST_CASE("unique_resource reset", "[unique_resource]", unique_resource_types)
{
	dispenser dispenser;

	bool const init = GENERATE(0, 1);

	TestType r = init
		? TestType(dispenser.acquire(), dispenser)
		: TestType(null_resource, dispenser);

	r.reset();

	CHECK(!static_cast<bool>(r));
}

TEMPLATE_LIST_TEST_CASE("unique_resource release", "[unique_resource]", unique_resource_types)
{
	dispenser dispenser;

	TestType r = TestType(dispenser.acquire(), dispenser);

	dispenser.release(r.release());

	CHECK(!static_cast<bool>(r));
}


using compact_unique_resource = unique_resource<
	resource,
	stateless_deleter,
	null>;

static_assert(sizeof(compact_unique_resource) == sizeof(resource));

TEST_CASE("unique_resource can be passed via out_resource or inout_resource", "[unique_resource]")
{
	dispenser dispenser;

	compact_unique_resource r(null_resource, dispenser);
	//if (GENERATE(0, 1))
	{
		r.reset(dispenser.acquire());
	}

#if 0
	SECTION("out_resource")
	{
		auto const produce_resource = [&](resource* const out) -> resource
		{
			if (GENERATE(0, 1))
			{
				return *out = dispenser.acquire();
			}

			return null;
		};

		resource const new_resource = produce_resource(out_resource(r));
		REQUIRE(static_cast<bool>(r) == (new_resource != null));

		if (r)
		{
			REQUIRE(r.get() == new_resource);
		}
	}
#endif

	SECTION("inout_resource")
	{
		resource const old_resource = r ? r.get() : null;

		auto const produce_resource = [&](resource* const inout) -> resource
		{
			REQUIRE(*inout == old_resource);

			if (GENERATE(0, 1))
			{
				return old_resource;
			}

			if (*inout != null)
			{
				dispenser.release(*inout);
			}

			if (GENERATE(0, 1))
			{
				return *inout = dispenser.acquire();
			}
			else
			{
				return *inout = null;
			}
		};

		resource const new_resource = produce_resource(inout_resource(r));
		REQUIRE(static_cast<bool>(r) == (new_resource != null));

		if (r)
		{
			REQUIRE(r.get() == new_resource);
		}
	}
}

} // namespace
