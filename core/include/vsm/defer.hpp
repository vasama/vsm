#pragma once

#include <vsm/concepts.hpp>
#include <vsm/preprocessor.h>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

template<std::invocable Lambda>
class deferral
{
	Lambda m_lambda;

public:
	deferral(std::convertible_to<Lambda> auto&& lambda)
		: m_lambda(vsm_forward(lambda))
	{
	}

	deferral(deferral const&) = delete;
	deferral& operator=(deferral const&) = delete;

	~deferral()
	{
		vsm_move(m_lambda)();
	}
};

template<typename Lambda>
deferral(Lambda&&) -> deferral<std::remove_reference_t<Lambda>>;

} // namespace detail

/// @brief Creates a deferral object which invokes the specified invocable upon destruction.
/// @param lambda Invocable object to be invoked upon destruction of the returned object.
template<std::invocable Lambda>
[[nodiscard]] auto make_deferral(Lambda&& lambda)
{
	return detail::deferral(vsm_forward(lambda));
}

} // namespace vsm

#define vsm_defer \
	::vsm::detail::deferral vsm_pp_anonymous(vsm_detail_deferral) = [&]() noexcept -> void
