#include <vsm/propagate_const.hpp>

#include <vsm/test/dependent_context.hpp>

#include <catch2/catch_all.hpp>

#include <memory>

using namespace vsm;

namespace {

template<typename T>
using p = propagate_const<T>;

struct s
{
	int v;
};

vsm_dependent_context // pointer
(
	using s = vsm_dep_t(::s);

	static_assert(requires (p<s*>& x)
	{
		x = nullptr;
		{ x.get() } -> std::same_as<s*>;
		{ *x } -> std::same_as<s&>;
		{ x->v } -> std::same_as<int&>;
		{ x[0] } -> std::same_as<s&>;
		{ x == nullptr } -> std::same_as<bool>;
	});

	static_assert(requires (p<s* const>& x)
	{
		{ x.get() } -> std::same_as<s*>;
		{ *x } -> std::same_as<s&>;
		{ x->v } -> std::same_as<int&>;
		{ x[0] } -> std::same_as<s&>;
		{ x == nullptr } -> std::same_as<bool>;
	});

	static_assert(!requires (p<s* const>& x)
	{
		x = nullptr;
	});

	static_assert(requires (p<s*> const& x)
	{
		{ x.get() } -> std::same_as<s const*>;
		{ *x } -> std::same_as<s const&>;
		{ x->v } -> std::same_as<int const&>;
		{ x[0] } -> std::same_as<s const&>;
		{ x == nullptr } -> std::same_as<bool>;
	});
);

vsm_dependent_context // unique_ptr
(
	using s = vsm_dep_t(::s);

	using uptr = std::unique_ptr<s>;
	
	static_assert(requires (p<uptr>& x)
	{
		x = nullptr;
		{ x.get() } -> std::same_as<s*>;
		{ *x } -> std::same_as<s&>;
		{ x->v } -> std::same_as<int&>;
		{ x == nullptr } -> std::same_as<bool>;
	});

	static_assert(!requires (p<uptr>& x)
	{
		x[0];
	});
	
	static_assert(requires (p<uptr const>& x)
	{
		{ x.get() } -> std::same_as<s*>;
		{ *x } -> std::same_as<s&>;
		{ x->v } -> std::same_as<int&>;
		{ x == nullptr } -> std::same_as<bool>;
	});

	static_assert(!requires (p<uptr const>& x)
	{
		x = nullptr;
	});

	static_assert(!requires (p<uptr const>& x)
	{
		x[0];
	});

	static_assert(requires (p<uptr> const& x)
	{
		{ x.get() } -> std::same_as<s const*>;
		{ *x } -> std::same_as<s const&>;
		{ x->v } -> std::same_as<int const&>;
		{ x == nullptr } -> std::same_as<bool>;
	});

	static_assert(!requires (p<uptr> const& x)
	{
		x[0];
	});
);

vsm_dependent_context // span
(
	using s = vsm_dep_t(::s);

	using span = std::span<s>;

	static_assert(requires (p<span>& x)
	{
		x = span();
		{ x.get() } -> std::same_as<span>;
		{ x[0] } -> std::same_as<s&>;
	});
	
	static_assert(requires (p<span const>& x)
	{
		{ x.get() } -> std::same_as<span>;
		{ x[0] } -> std::same_as<s&>;
	});
	
	static_assert(!requires (p<span const>& x)
	{
		x = span();
	});
	
	static_assert(requires (p<span> const& x)
	{
		{ x.get() } -> std::same_as<std::span<s const>>;
		{ x[0] } -> std::same_as<s const&>;
	});
);

} // namespace
