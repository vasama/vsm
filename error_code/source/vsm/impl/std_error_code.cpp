#include <vsm/error_code.hpp>

#include <vsm/std_error_code.hpp>

using namespace vsm;

template<uintptr_t Addr>
static std::error_category const& get_std_error_category();

template<>
std::error_category const& get_std_error_category<1>()
{
	return std::system_category();
}

template<>
std::error_category const& get_std_error_category<2>()
{
	return std::generic_category();
}


template<uintptr_t Addr>
char const* detail::builtin_error_category<Addr>::name() const noexcept
{
	return get_std_error_category<Addr>().name();
}

template<uintptr_t Addr>
std::string_view detail::builtin_error_category<Addr>::format(int const code, std::span<char> const buffer) const noexcept
{
	try
	{
		std::string const string = get_std_error_category<Addr>().message(code);
		return std::string_view(buffer.data(), string.copy(buffer.data(), buffer.size()));
	}
	catch (...)
	{
		return {};
	}
}

template<uintptr_t Addr>
bool detail::builtin_error_category<Addr>::equivalent(int const code, error_condition const& condition) const noexcept
{
	if (condition.value() == code && condition.category() == *this)
	{
		return true;
	}

	if (std_error_code std_code; condition.category().get_std_error_code(code, std_code))
	{
		std::error_condition const std_condition(std_code.value(), std_code.category());
		return get_std_error_category<Addr>().equivalent(code, std_condition);
	}

	return false;
}

template<uintptr_t Addr>
bool detail::builtin_error_category<Addr>::equivalent(error_code const& code, int const condition) const noexcept
{
	if (code.value() == condition && code.category() == *this)
	{
		return true;
	}

	if (std_error_code sec; code.category().get_std_error_code(condition, sec))
	{
		return get_std_error_category<Addr>().equivalent(sec, condition);
	}

	return false;
}

template<uintptr_t Addr>
error_condition detail::builtin_error_category<Addr>::default_error_condition(int const code) const noexcept
{
	auto const std_condition = get_std_error_category<Addr>().default_error_condition(code);

	return std_condition.category() == std::generic_category()
		? error_condition(std_condition.value(), generic_category())
		: error_condition();
}

template<uintptr_t Addr>
bool detail::builtin_error_category<Addr>::get_std_error_code(int const code, std_error_code& out) const noexcept
{
	out = std_error_code(code, get_std_error_category<Addr>());
	return true;
}


template class detail::builtin_error_category<1>;
template class detail::builtin_error_category<2>;
