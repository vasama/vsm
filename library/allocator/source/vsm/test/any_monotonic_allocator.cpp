#include <vsm/any_monotonic_allocator.hpp>

#include <vsm/testing/allocator.hpp>

#include <catch2/catch_all.hpp>

#include <span>

using namespace vsm;

namespace {

class test_resource
{
	std::byte* m_pos;
	std::byte* m_end;

public:
	class position_type
	{
		[[maybe_unused]] std::byte* pos;
	};

	explicit test_resource(std::span<std::byte> const storage) noexcept
		: m_pos(storage.data())
		, m_end(storage.data() + storage.size())
	{
	}

	allocation allocate(size_t const min_size, [[maybe_unused]] size_t const max_size) noexcept
	{
		size_t const available_size = static_cast<size_t>(m_end - m_pos);

		if (min_size > available_size)
		{
			return allocation(nullptr);
		}

		return allocation(std::exchange(m_pos, m_pos + min_size), min_size);
	}

	void deallocate(allocation const allocation) noexcept
	{
	}

	[[nodiscard]] position_type get_position() const noexcept
	{
		return std::bit_cast<position_type>(m_pos);
	}

	void reset_position(position_type const pos) noexcept
	{
		m_pos = std::bit_cast<std::byte*>(pos);
	}
};
static_assert(monotonic_memory_resource<test_resource>);

TEST_CASE("any_monotonic_allocator", "[any][allocator]")
{
	std::byte arena_storage[256];
	test_resource arena(arena_storage);

	(void)any_allocator(any_monotonic_allocator(arena));

	any_monotonic_allocator monotonic_allocator(arena);
	any_allocator allocator(monotonic_allocator);
}

} // namespace
