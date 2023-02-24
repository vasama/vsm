#pragma once

#include <span>
#include <string_view>

#include <cstdint>

namespace vsm {

template<typename ErrorCodeEnum>
struct is_error_condition_enum
{
	static constexpr bool value = false;
};

template<typename ErrorCodeEnum>
inline constexpr bool is_error_condition_enum_v = is_error_condition_enum<ErrorCodeEnum>::value;

template<typename ErrorCodeEnum>
struct is_error_code_enum
{
	static constexpr bool value = false;
};

template<typename ErrorCodeEnum>
inline constexpr bool is_error_code_enum_v = is_error_code_enum<ErrorCodeEnum>::value;

namespace detail {

struct std_error_code;

class error_code;
class error_condition;


template<uintptr_t Addr>
class builtin_error_category;

class error_category
{
	uintptr_t m_addr;

public:
	error_category(error_category const&) = delete;
	error_category& operator=(error_category const&) = delete;

	virtual char const* name() const noexcept = 0;
	virtual std::string_view format(int code, std::span<char> buffer) const noexcept = 0;
	virtual bool equivalent(int code, error_condition const& condition) const noexcept;
	virtual bool equivalent(error_code const& code, int condition) const noexcept;
	virtual error_condition default_error_condition(int code) const noexcept;

	virtual bool get_std_error_code(int code, std_error_code& out) const noexcept;

	auto operator<=>(error_category const& other) const noexcept = default;

protected:
	error_category() noexcept
		: m_addr(reinterpret_cast<uintptr_t>(this))
	{
	}

private:
	explicit constexpr error_category(uintptr_t const addr) noexcept
		: m_addr(addr)
	{
	}

	template<uintptr_t Addr>
	friend class builtin_error_category;
};

template<uintptr_t Addr>
class builtin_error_category : public error_category
{
public:
	constexpr builtin_error_category()
		: error_category(Addr)
	{
	}

	char const* name() const noexcept override;
	std::string_view format(int code, std::span<char> buffer) const noexcept override;
	bool equivalent(int code, error_condition const& condition) const noexcept override;
	bool equivalent(error_code const& code, int condition) const noexcept override;
	error_condition default_error_condition(int code) const noexcept override;
	bool get_std_error_code(int code, std_error_code& out) const noexcept override;
};

extern template class builtin_error_category<1>;
using system_error_category = builtin_error_category<1>;
inline error_category const& system_category() noexcept
{
	static constinit detail::system_error_category const category;
	return category;
}

extern template class builtin_error_category<2>;
using generic_error_category = builtin_error_category<2>;
inline error_category const& generic_category() noexcept
{
	static constinit detail::generic_error_category const category;
	return category;
}


class error_impl
{
protected:
	int m_value;
	error_category const* m_category;

public:
	error_impl(int const code, error_category const& category) noexcept
		: m_value(code)
		, m_category(&category)
	{
	}


	int value() const noexcept
	{
		return m_value;
	}

	error_category const& category() const noexcept
	{
		return *m_category;
	}


	std::string_view format(std::span<char> const buffer) const noexcept
	{
		return m_category->format(m_value, buffer);
	}


	explicit operator bool() const noexcept
	{
		return m_value != 0;
	}

protected:
	auto operator<=>(error_impl const&) const = default;
};

class error_condition : error_impl
{
public:
	using error_impl::error_impl;

	error_condition() noexcept
		: error_impl(0, generic_category())
	{
	}

	template<typename ErrorConditionEnum>
	error_condition(ErrorConditionEnum const e) noexcept
		requires is_error_condition_enum_v<ErrorConditionEnum>
		: error_impl(make_error_condition(e))
	{
	}

	error_condition& operator=(error_condition const&) & = default;


	using error_impl::value;
	using error_impl::category;
	using error_impl::format;
	using error_impl::operator bool;

	auto operator<=>(error_condition const&) const = default;
};

class error_code : public error_impl
{
public:
	using error_impl::error_impl;

	error_code() noexcept
		: error_impl(0, system_category())
	{
	}

	template<typename ErrorCodeEnum>
	error_code(ErrorCodeEnum const e) noexcept
		requires is_error_code_enum_v<ErrorCodeEnum>
		: error_impl(make_error_code(e))
	{
	}

	error_code& operator=(error_code const&) & = default;


	using error_impl::value;
	using error_impl::category;
	using error_impl::format;
	using error_impl::operator bool;

	error_condition default_error_condition() const noexcept
	{
		return category().default_error_condition(value());
	}


	auto operator<=>(error_code const&) const = default;
};

} // namespace detail

using detail::error_category;
using detail::error_condition;
using detail::error_code;

using detail::system_category;
using detail::generic_category;

} // namespace vsm
