#include <vsm/async/timer.hpp>

#include <catch2/catch_all.hpp>

#include <chrono>

using namespace vsm;
using namespace vsm::async;

namespace {

struct clock
{
	using rep = int32_t;
	using period = std::ratio<1>;
	using duration = std::chrono::duration<rep, period>;
	using time_point = std::chrono::time_point<Clock, duration>;

	static constexpr bool is_steady = true;

	static time_point now()
	{
		return now_value;
	}

	static time_point now_value;
};

clock::time_point clock::now_value = {};


struct timer : async_timer
{
	clock::time_point m_time_point;

	clock::time_point const& get_time_point() const
	{
		return m_time_point;
	}

	void get_time_point(clock::time_point const time_point)
	{
		m_time_point = time_point;
	}

	void timer_completed(clock::time_point const time_point)
	{
		
	}

	void timer_abandoned()
	{
		
	}
};
using timer_queue = async_timer_queue<timer>;


TEST_CASE("async_timer_queue default constructor", "[async][async_timer]")
{
	timer_queue queue;
	CHECK(queue.is_empty());
}

TEST_CASE("async_timer_queue schedule & expire", "[async][async_timer]")
{
	timer_queue queue;

	timer t1;
	queue.schedule()
}

TEST_CASE("async_timer_queue::cancel", "[async][async_timer]")
{
}

} // namespace
