#pragma once

#include <system_error>

namespace vsm {
namespace detail {

struct exception_category : std::error_category
{
	char const* name() const noexcept override;
	std::string message(int const code) const override;
};

extern exception_category const exception_category_instance;

} // namespace detail

enum class exception_code
{
	none,
	bad_alloc,
};

[[nodiscard]] inline std::error_code make_error_code(exception_code const code) noexcept
{
	return std::error_code(static_cast<int>(code), detail::exception_category_instance);
}

void throw_error_code(std::error_code error);

} // namespace vsm

template<>
struct std::is_error_code_enum<vsm::exception_code>
{
	static constexpr bool value = true;
};
