#include <vsm/atomic_intrusive_ptr.hpp>

#include <vsm/test/instance_counter.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

struct alignas(16) s {};
static_assert(std::atomic_ref<s>::is_always_lock_free);


TEST_CASE("atomic_intrusive_ptr: single thread", "[intrusive_ptr][atomic]")
{
	struct shared_object : intrusive_ref_count, test::instance_counter<shared_object> {};

	{
		shared_object* const shared = new shared_object();
		atomic_intrusive_ptr atomic = atomic_intrusive_ptr(shared);

		intrusive_ptr pointer = atomic.load();
		REQUIRE(pointer.get() == shared);

		pointer = nullptr;

		pointer = atomic.exchange(vsm_move(pointer));
		REQUIRE(pointer == pointer);

		pointer = atomic.exchange(vsm_move(pointer));
		REQUIRE(pointer == nullptr);
	}
	CHECK(test::instance_count<shared_object>() == 0);
}

TEST_CASE("atomic_intrusive_ptr: multiple threads", "[intrusive_ptr][atomic]")
{
	std::atomic_flag failed;

	struct alignas(64) shared_object : intrusive_ref_count, test::instance_counter<shared_object>
	{
		std::atomic_flag destroyed;
		std::atomic_flag* failed = nullptr;

		friend void tag_invoke(decltype(intrusive_ptr_delete), shared_object* const p)
		{
			if (p->destroyed.test_and_set())
			{
				p->failed->test_and_set();
			}
		}
	};

	static constexpr size_t thread_count = 8;

	struct
	{
		alignas(64) shared_object objects[thread_count + 1];

		alignas(64) atomic_intrusive_ptr<shared_object> p;

		struct alignas(64)
		{
			std::atomic<size_t> started;
			std::atomic_flag timeout;
		};
	}
	shared;

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
				while (shared.started.load() < thread_count);

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

		while (shared.started.load() < thread_count);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		shared.timeout.test_and_set();
	}

	REQUIRE(!failed.test());
}

} // namespace
