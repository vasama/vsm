#pragma once

#include <vsm/concepts.hpp>
#include "vsm/preprocessor.h"
#include <vsm/utility.hpp>

namespace vsm::detail {

template<typename Lambda>
class deferral
{
	Lambda m_lambda;

public:
	deferral(Lambda&& lambda)
		: m_lambda(vsm_forward(lambda))
	{
	}
	
	deferral(deferral const&) = delete;
	deferral& operator=(deferral const&) = delete;

	~deferral()
	{
		m_lambda();
	}
};

template<typename Lambda>
deferral(Lambda) -> deferral<Lambda>;

} // namespace vsm::detail

#define vsm_defer \
	::vsm::detail::deferral vsm_pp_cat(vsm_defer_,__LINE__) = [&]() noexcept -> void
