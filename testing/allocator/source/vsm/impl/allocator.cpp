#include <vsm/testing/allocator.hpp>

#include <vsm/assert.h>
#include <vsm/impl/allocator.hpp>

#include <algorithm>
#include <unordered_map>

#include <cstring>

using namespace vsm::test;

// NOLINTBEGIN(bugprone-sizeof-expression)

#if 0 // NOLINT(readability-avoid-unconditional-preprocessor-if)
#define vsm_printf(...) printf(__VA_ARGS__)
#else
#define vsm_printf(...) ((void)sizeof(__VA_ARGS__))
#endif

namespace {

[[maybe_unused]]
static void* offset_ptr(void* const ptr, size_t const offset)
{
	return static_cast<unsigned char*>(ptr) + offset;
}

[[maybe_unused]]
static void* offset_ptr(void* const ptr, ptrdiff_t const offset)
{
	return static_cast<unsigned char*>(ptr) + offset;
}


static size_t const page_size = virtual_page_size();

struct allocation_info
{
	bool allocated;
	bool scoped;
	bool reversed;
	bool maximized;
	size_t user_size;

#if __INTELLISENSE__
	// https://developercommunity.visualstudio.com/t/EDG-rejects-parenthesized-aggregate-init/10732572
	allocation_info(auto...);
#endif
};

struct allocation_scope_info
{
	allocation_scope_info* prev;

	bool reversed;
	bool maximized;

	size_t allocation_count = 0;
	std::unordered_map<void*, allocation_info> map;
};

static allocation_scope_info g_root_scope = {};
static allocation_scope_info* g_current_scope = &g_root_scope;


static constexpr unsigned char canary_value = 0xCD;

static void apply_canary(void* const data, size_t const size)
{
	vsm_printf("%p %08x   APPLY\n", data, static_cast<unsigned>(size));

	std::memset(data, canary_value, size);
}

static void check_canary(void const* const data, size_t const size)
{
	vsm_printf("%p %08x   CHECK\n", data, static_cast<unsigned>(size));

	auto const beg = static_cast<unsigned char const*>(data);
	auto const end = beg + size;

	auto const it = std::find_if(
		beg,
		end,
		[](auto const x) { return x != canary_value; });

	vsm_assert(it == end);
}

} // namespace

vsm::allocation detail::allocate(size_t const user_size)
{
	auto const scope = g_current_scope;

	size_t const page_count = (user_size + page_size - 1) / page_size;

	void* const address = virtual_allocate((page_count + 2) * page_size);
	void* const commit_address = offset_ptr(address, page_size);

	size_t const commit_size = page_count * page_size;
	virtual_commit(commit_address, commit_size);

	size_t const canary_size = commit_size - user_size;
	size_t const user_offset = static_cast<size_t>(scope->reversed) * canary_size;
	size_t const canary_offset = static_cast<size_t>(!scope->reversed) * user_size;

	void* const user_address = offset_ptr(commit_address, user_offset);
	void* const canary_address = offset_ptr(commit_address, canary_offset);

	vsm_printf("%p %08x ACQ\n", user_address, static_cast<unsigned>(user_size));

	apply_canary(canary_address, canary_size);

	vsm_verify(scope->map.try_emplace(
		user_address,
		true,
		scope->prev != nullptr,
		scope->reversed,
		scope->maximized,
		user_size).second);

	++scope->allocation_count;

	return { user_address, user_size };
}

void detail::deallocate(vsm::allocation const allocation)
{
	auto const scope = g_current_scope;

	auto const [user_address, user_size] = allocation;

	vsm_printf("%p %08x REL\n", user_address, static_cast<unsigned>(user_size));

	auto const it = scope->map.find(user_address);
	vsm_verify(it != scope->map.end());

	allocation_info& info = it->second;
	vsm_assert(info.allocated);
	vsm_assert(allocation.size == info.user_size);

	size_t const page_count = (user_size + page_size - 1) / page_size;
	size_t const commit_size = page_count * page_size;

	size_t const canary_size = commit_size - user_size;
	size_t const user_offset = static_cast<size_t>(info.reversed) * canary_size;
	size_t const canary_offset = static_cast<size_t>(!info.reversed) * user_size;

	void* const commit_address = offset_ptr(
		user_address,
		-static_cast<ptrdiff_t>(user_offset));

	void* const canary_address = offset_ptr(
		commit_address,
		static_cast<ptrdiff_t>(canary_offset));

	check_canary(canary_address, canary_size);

	if (info.scoped)
	{
		virtual_decommit(commit_address, commit_size);
		info.allocated = false;
	}
	else
	{
		void* const address = offset_ptr(
			commit_address,
			-static_cast<ptrdiff_t>(page_size));

		virtual_deallocate(
			address,
			(page_count + 2) * page_size);

		scope->map.erase(it);
	}

	--scope->allocation_count;
}

void* detail::enter_allocation_scope(allocation_scope_options const& options)
{
	auto const scope = new allocation_scope_info
	{
		g_current_scope,
		options.reversed,
		options.maximized,
	};

	g_current_scope = scope;
	return scope;
}

void detail::leave_allocation_scope(void* const ptr)
{
	auto const scope = static_cast<allocation_scope_info*>(ptr);

	vsm_assert(scope->prev != nullptr);
	g_current_scope = scope->prev;

	for (auto const& [user_address, info] : scope->map)
	{
		if (!info.allocated)
		{
			continue;
		}

		size_t const user_size = info.user_size;

		vsm_printf("%p %08x LEAVE\n", user_address, static_cast<unsigned>(user_size));

		size_t const page_count = (user_size + page_size - 1) / page_size;
		size_t const commit_size = page_count * page_size;

		size_t const canary_size = commit_size - user_size;
		size_t const user_offset = static_cast<size_t>(info.reversed) * canary_size;
		size_t const canary_offset = static_cast<size_t>(!info.reversed) * user_size;

		void* const commit_address = offset_ptr(
			user_address,
			-static_cast<ptrdiff_t>(user_offset));

		void* const canary_address = offset_ptr(
			commit_address,
			static_cast<ptrdiff_t>(canary_offset));

		check_canary(canary_address, canary_size);

		void* const address = offset_ptr(
			commit_address,
			-static_cast<ptrdiff_t>(page_size));

		virtual_deallocate(
			address,
			(page_count + 2) * page_size);
	}
	scope->map.clear();

	delete scope;
}

size_t detail::get_scope_allocation_count(void* const ptr)
{
	auto const scope = static_cast<allocation_scope_info*>(ptr);
	return scope->allocation_count;
}

// NOLINTEND(bugprone-sizeof-expression)
