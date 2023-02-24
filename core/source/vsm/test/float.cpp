#include <vsm/float.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

TEST_CASE("float16_t arithmetic", "[core][float16_t]")
{
	CHECK(static_cast<float>(static_cast<float16_t>(+INFINITY)) == +INFINITY);
	CHECK(static_cast<float>(static_cast<float16_t>(-INFINITY)) == -INFINITY);

	CHECK(static_cast<float>(static_cast<float16_t>(+0.0f)) == +0.0f);
	CHECK(static_cast<float>(static_cast<float16_t>(-0.0f)) == -0.0f);
	CHECK(static_cast<float>(static_cast<float16_t>(+1.0f)) == +1.0f);
	CHECK(static_cast<float>(static_cast<float16_t>(-1.0f)) == -1.0f);

	CHECK(static_cast<float>(+static_cast<float16_t>(+1.0f)) == +1.0f);
	CHECK(static_cast<float>(+static_cast<float16_t>(-1.0f)) == -1.0f);
	CHECK(static_cast<float>(+static_cast<float16_t>(+INFINITY)) == +INFINITY);
	CHECK(static_cast<float>(+static_cast<float16_t>(-INFINITY)) == -INFINITY);
	CHECK(static_cast<float>(-static_cast<float16_t>(+1.0f)) == -1.0f);
	CHECK(static_cast<float>(-static_cast<float16_t>(-1.0f)) == +1.0f);
	CHECK(static_cast<float>(-static_cast<float16_t>(+INFINITY)) == -INFINITY);
	CHECK(static_cast<float>(-static_cast<float16_t>(-INFINITY)) == +INFINITY);

	CHECK(float16_t(2.0f) + float16_t(3.0f) == 5.0f);
	CHECK(float16_t(5.0f) - float16_t(2.0f) == 3.0f);
	CHECK(float16_t(2.0f) * float16_t(3.0f) == 6.0f);
	CHECK(float16_t(6.0f) / float16_t(2.0f) == 3.0f);

	CHECK(static_cast<float16_t>(+0.0f) == static_cast<float16_t>(-0.0f));
	CHECK(static_cast<float16_t>(-0.0f) == static_cast<float16_t>(+0.0f));
	CHECK(static_cast<float16_t>(+7.0f) == static_cast<float16_t>(+7.0f));
	CHECK(static_cast<float16_t>(+7.0f) != static_cast<float16_t>(+7.1f));

	CHECK(static_cast<float16_t>(+0.0f) < static_cast<float16_t>(+1.0f));
	CHECK(static_cast<float16_t>(+0.0f) > static_cast<float16_t>(-1.0f));
	CHECK(static_cast<float16_t>(+0.0f) <= static_cast<float16_t>(-0.0f));
	CHECK(static_cast<float16_t>(+0.0f) <= static_cast<float16_t>(+1.0f));
	CHECK(static_cast<float16_t>(+0.0f) >= static_cast<float16_t>(-0.0f));
	CHECK(static_cast<float16_t>(+0.0f) >= static_cast<float16_t>(-1.0f));
}
