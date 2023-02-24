#pragma once

#include <cstdint>

namespace vsm::unit_test {
namespace detail {

class branch_base
{

};

} // namespace detail

struct advance_result
{
	bool value;
	uint32_t count;
};

class branch : public detail::branch_base
{
public:
	[[nodiscard]] virtual advance_result advance(uint32_t count) = 0;
};

} // namespace vsm::unit_test
