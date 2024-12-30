#include <vsm/atomic_intrusive_ptr.hpp>

#include <vsm/testing/instance_counter.hpp>

#include <catch2/catch_all.hpp>

#include <thread>

using namespace vsm;

namespace {

struct alignas(sizeof(void*) * 2) s {};
static_assert(atomic<s>::is_always_lock_free);


TEST_CASE("atomic_intrusive_ptr: single thread", "[intrusive_ptr][atomic]")
{
	struct shared_object : intrusive_refcount, test::counted {};

	test::scoped_count const instance_count;
	{
		shared_object* const shared = new shared_object();
		atomic_intrusive_ptr<shared_object> atomic(shared);

		intrusive_ptr pointer = atomic.load();
		REQUIRE(pointer.get() == shared);

		pointer = nullptr;

		pointer = atomic.exchange(vsm_move(pointer));
		REQUIRE(pointer == shared);

		pointer = atomic.exchange(vsm_move(pointer));
		REQUIRE(pointer == nullptr);
	}
	CHECK(instance_count.empty());
}


struct alignas(64) mt_shared_object : intrusive_refcount
{
	std::atomic_flag destroyed;
	std::atomic_flag* failed = nullptr;

	friend void tag_invoke(
		decltype(intrusive_ptr_deleter),
		default_refcount_manager const&,
		mt_shared_object* const p)
	{
		if (p->destroyed.test_and_set())
		{
			p->failed->test_and_set();
		}
	}
};

TEST_CASE("atomic_intrusive_ptr: multiple threads", "[intrusive_ptr][atomic]")
{
	using shared_object = mt_shared_object;

	static constexpr size_t thread_count = 8;

	struct
	{
		alignas(64) shared_object objects[thread_count + 1];
		alignas(64) atomic_intrusive_ptr<shared_object> p;
		alignas(64) std::atomic<size_t> started;
		alignas(64) std::atomic_flag timeout;
	}
	shared;

	std::atomic_flag failed;
	for (shared_object& object : shared.objects)
	{
		object.failed = &failed;
	}

	using iptr = intrusive_ptr<shared_object>;
	shared.p.store_relaxed(iptr(&shared.objects[thread_count]));

	{
		std::jthread threads[thread_count];
		for (size_t i = 0; i < thread_count; ++i)
		{
			threads[i] = std::jthread([&shared, i]()
			{
				iptr p = iptr(&shared.objects[i]);
				iptr q;

				shared.started.fetch_add(1);
				while (shared.started.load() < thread_count)
				{
					std::this_thread::yield();
				}

				for (bool exchange = false; !shared.timeout.test(); exchange = !exchange)
				{
					if (exchange)
					{
						q = shared.p.load();
					}
					else
					{
						p = shared.p.exchange(vsm_move(p));
					}
				}
			});
		}

		while (shared.started.load() < thread_count)
		{
			std::this_thread::yield();
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
		shared.timeout.test_and_set();
	}

	REQUIRE(!failed.test());
}

} // namespace
