#include <vsm/relocate.hpp>

#include <catch2/catch_all.hpp>

using namespace vsm;

namespace {

template<non_ref T>
class forward_pointer
{
	T* m_ptr;

public:
	using value_type = T;
	using difference_type = ptrdiff_t;

	forward_pointer() = default;

	forward_pointer(T* const ptr) noexcept
		: m_ptr(ptr)
	{
	}

	[[nodiscard]] T& operator*() const noexcept
	{
		return *m_ptr;
	}

	[[nodiscard]] T* operator->() const noexcept
	{
		return m_ptr;
	}

	forward_pointer& operator++() & noexcept
	{
		++m_ptr;
		return *this;
	}

	[[nodiscard]] forward_pointer operator++(int) & noexcept
	{
		auto result = *this;
		++m_ptr;
		return result;
	}

	friend bool operator==(forward_pointer const&, forward_pointer const&) = default;
};

TEST_CASE("relocate", "[core][relocate]")
{
	//TODO: Implement relocate unit tests
}

} // namespace
