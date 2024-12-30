#include <vsm/unique_resource.hpp>

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <tuple>
#include <vector>

using namespace vsm;

namespace {

using resource = size_t;

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

		CHECK(std::ranges::all_of(m_resources, [](bool const x) { return x; }));
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
	unique_resource<resource, stateless_deleter, static_cast<size_t>(-1)>,

	unique_resource<resource, trivial_stateful_deleter>,
	unique_resource<resource, trivial_stateful_deleter, static_cast<size_t>(-1)>,

	unique_resource<resource, non_trivial_stateful_deleter>,
	unique_resource<resource, non_trivial_stateful_deleter, static_cast<size_t>(-1)>
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

} // namespace
