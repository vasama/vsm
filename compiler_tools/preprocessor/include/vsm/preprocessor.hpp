#pragma once

#include <vsm/compilers/diagnostics.hpp>
#include <vsm/defer.hpp>
#include <vsm/flags.hpp>

#include <concepts>
#include <optional>
#include <string_view>

namespace vsm::compilers {

class preprocessor_file
{
};

class preprocessor_context
{
public:
	virtual void mark_once(std::string_view current_path) = 0;
	virtual std::optional<preprocessor_file> include(std::string_view current_path, std::string_view include_path, bool search_relative) const = 0;
};

class preprocessor2
{
public:
	class builtin_macro
	{
	};

	class macro_context
	{
	};
	typedef void macro_handler(macro_context& context);

	class directive_context
	{

	};
	typedef void directive_handler(directive_context& context);

	struct customizations
	{
		template<typename Customization>
		struct customization
		{
			std::string_view name;
			Customization object;
		};

		template<typename Customization>
		using customization_span = std::span<customization<Customization> const>;

		customization_span<builtin_macro> macros;
		customization_span<directive_handler*> directives;
		customization_span<directive_handler*> pragmas;
	};


	void include(source const& source);
	source const& render() const;
};

class preprocessor_base
{
	struct preprocessor_impl;

	typedef void builtin_handler(preprocessor_base& preprocessor);
	typedef void directive_handler(preprocessor_base& preprocessor);

	class alignas(8) macro
	{
	protected:
		macro() = default;
		macro(macro const&) = default;
		macro& operator=(macro const&) = default;
		~macro() = default;
	};


	preprocessor_impl* m_impl;
	preprocessor_context* m_context;
	diagnostics m_diagnostics;
	builtin_handler* m_next_builtin_handler;

public:
	class integer
	{
		bool m_is_unsigned;
		uintmax_t m_unsigned_bits;

	public:
		integer(bool) = delete;

		constexpr integer(std::signed_integral auto const value)
			: m_is_unsigned(false)
			, m_unsigned_bits(static_cast<intmax_t>(value))
		{
		}

		constexpr integer(std::unsigned_integral auto const value)
			: m_is_unsigned(true)
			, m_unsigned_bits(static_cast<uintmax_t>(value))
		{
		}


		bool is_signed() const
		{
			return !m_is_unsigned;
		}

		intmax_t get_signed() const
		{
			vsm_assert(is_signed());
			return static_cast<intmax_t>(m_unsigned_bits);
		}

		bool is_unsigned() const
		{
			return m_is_unsigned;
		}

		uintmax_t get_unsigned() const
		{
			vsm_assert(is_unsigned());
			return m_unsigned_bits;
		}


		uintmax_t get_unsigned_bits() const
		{
			return m_unsigned_bits;
		}
	};

	struct arithmetic_result
	{
		integer value;
		bool overflow;
		bool promoted;
	};

	static arithmetic_result neg(integer rhs);
	static arithmetic_result add(integer lhs, integer rhs);
	static arithmetic_result sub(integer lhs, integer rhs);
	static arithmetic_result mul(integer lhs, integer rhs);
	static arithmetic_result div(integer lhs, integer rhs);
	static arithmetic_result mod(integer lhs, integer rhs);
	static arithmetic_result shl(integer lhs, integer rhs);
	static arithmetic_result shr(integer lhs, integer rhs);

	static arithmetic_result bitwise_not(integer rhs);
	static arithmetic_result bitwise_and(integer lhs, integer rhs);
	static arithmetic_result bitwise_xor(integer lhs, integer rhs);
	static arithmetic_result bitwise_or(integer lhs, integer rhs);

	static arithmetic_result logical_not(integer rhs);
	static arithmetic_result logical_and(integer lhs, integer rhs);
	static arithmetic_result logical_or(integer lhs, integer rhs);

	static arithmetic_result eq(integer lhs, integer rhs);
	static arithmetic_result ne(integer lhs, integer rhs);
	static arithmetic_result lt(integer lhs, integer rhs);
	static arithmetic_result gt(integer lhs, integer rhs);
	static arithmetic_result le(integer lhs, integer rhs);
	static arithmetic_result ge(integer lhs, integer rhs);

	void check_arithmetic(source_location const& location, arithmetic_result result);


	preprocessor_base(preprocessor_base const&) = delete;
	preprocessor_base& operator=(preprocessor_base const&) = delete;


	[[nodiscard]] preprocessor_context& get_context() const
	{
		return *m_context;
	}

	[[nodiscard]] diagnostics diagnostics() const
	{
		return m_diagnostics;
	}


	[[nodiscard]] std::string_view get_current_file() const;
	[[nodiscard]] size_t get_current_line() const;
	[[nodiscard]] source_location const& get_current_location() const;


	[[nodiscard]] auto enable_macro_expansion(bool const enable)
	{
		return make_deferral([this, value = set_enable_macro_expansion(enable)]()
		{
			(void)set_enable_macro_expansion(value);
		});
	}


	// Emit
	void emit(source_location const& location, std::string_view source);
	void emit_string(source_location const& location, std::string_view value);
	void emit_integer(source_location const& location, integer value);


	void next_builtin_handler()
	{
		if (m_next_builtin_handler != nullptr)
		{
			m_next_builtin_handler(*this);
		}
	}


	// Macros

	class builtin_macro : public macro
	{
		bool m_is_variable_like;
		bool m_is_function_like;
		bool m_is_poisoned;
		bool m_is_defined;

		union
		{
			builtin_handler* m_handler;
			diagnostic const* m_diagnostic;
		};

		friend class preprocessor_base;
	};

	static constexpr builtin_macro poisoned_macro(diagnostic const& diagnostic)
	{
		builtin_macro m = {};
		m.m_is_variable_like = true;
		m.m_is_function_like = true;
		m.m_is_poisoned = true;
		m.m_diagnostic = &diagnostic;
		return m;
	}

	static constexpr builtin_macro variable_macro(builtin_handler* const handler)
	{
		builtin_macro m = {};
		m.m_is_variable_like = true;
		m.m_handler = handler;
		return m;
	}

	static constexpr builtin_macro function_macro(builtin_handler* const handler)
	{
		builtin_macro m = {};
		m.m_is_function_like = true;
		m.m_handler = handler;
		return m;
	}

	static constexpr builtin_macro operator_macro(builtin_handler* const handler)
	{
		builtin_macro m = {};
		m.m_is_variable_like = true;
		m.m_is_function_like = true;
		m.m_handler = handler;
		return m;
	}

	void set_macro(std::string_view name, builtin_macro const& macro);
	[[nodiscard]] std::string_view get_macro_name() const;
	[[nodiscard]] source_location const& get_macro_location() const;


	// Directives
	void set_directive(std::string_view name, builtin_handler& handler);
	[[nodiscard]] std::string_view get_directive_name() const;
	[[nodiscard]] source_location const& get_directive_location() const;


	// Pragmas
	void set_pragma(std::string_view name, builtin_handler& handler);
	[[nodiscard]] std::string_view get_pragma_name() const;
	[[nodiscard]] source_location const& get_pragma_location() const;
	void emit_current_pragma();


	// __COUNTER__
	[[nodiscard]] size_t get_counter_value() const;
	[[nodiscard]] size_t inc_counter_value();
	size_t set_counter_value(size_t const value);


	// Filesystem
	virtual void mark_once();
	virtual std::optional<preprocessor_file> find_file(std::string_view file, bool search_relative);


	// C++ attributes
	virtual bool has_cpp_attribute(std::string_view name);



	template<typename Customization>
	struct customization
	{
		std::string_view name;
		Customization object;
	};

	struct customizations
	{

		std::span<customization<builtin_macro> const> macros;
		std::span<customization<builtin_handler*> const> directives;
		std::span<customization<builtin_handler*> const> pragmas;
	};


	void include(source const& source);

	source const& render() const;


private:
	preprocessor_base() = default;
	~preprocessor_base() = default;

	[[nodiscard]] bool set_enable_macro_expansion(bool enable);
};

preprocessed_source preprocess(preprocessor_context& context, preprocessor_file const& file);

} // namespace vsm::compilers
