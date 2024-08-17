#include <vsm/casting.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

class shape
{
protected:
	enum class kind
	{
		circle,
		rectangle,
	};

private:
	kind m_kind;

protected:
	explicit shape(kind const kind)
		: m_kind(kind)
	{
	}

	shape(shape const&)
	{
	}

	shape& operator=(shape const&) &
	{
		return *this;
	}

	static kind get_kind(shape const& shape)
	{
		return shape.m_kind;
	}
};

class circle : public shape
{
public:
	circle()
		: shape(kind::circle)
	{
	}

	static bool is(casting::member_t, shape const& shape)
	{
		return get_kind(shape) == kind::circle;
	}
};

class rectangle : public shape
{
public:
	rectangle()
		: shape(kind::rectangle)
	{
	}

	static bool is(casting::member_t, shape const& shape)
	{
		return get_kind(shape) == kind::rectangle;
	}
};


TEST_CASE("", "[casting]")
{
	circle c;
	rectangle r;

	bool const is_circle = GENERATE(1, 0);

	shape* const p = is_circle
		? static_cast<shape*>(&c)
		: static_cast<shape*>(&r);

	SECTION("is")
	{
		REQUIRE(is<circle>(p) == is_circle);
		REQUIRE(is<rectangle>(p) == !is_circle);
	}

	SECTION("cast")
	{
		if (is_circle)
		{
			std::same_as<circle*> auto const q = cast<circle>(p);
			REQUIRE(q == &c);
		}
		else
		{
			std::same_as<rectangle*> auto const q = cast<rectangle>(p);
			REQUIRE(q == &r);
		}
	}

	SECTION("try_cast")
	{
		SECTION("circle")
		{
			std::same_as<circle*> auto const q = try_cast<circle>(p);
			if (is_circle)
			{
				REQUIRE(q == &c);
			}
			else
			{
				REQUIRE(q == nullptr);
			}
		}

		SECTION("rectangle")
		{
			std::same_as<rectangle*> auto const q = try_cast<rectangle>(p);
			if (is_circle)
			{
				REQUIRE(q == nullptr);
			}
			else
			{
				REQUIRE(q == &r);
			}
		}
	}
}

} // namespace
