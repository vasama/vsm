// https://godbolt.org/z/vfs1T5csn

#include <vsm/preprocessor.hpp>

#include <vsm/compilers/diagnostics.hpp>

#include <vsm/defer.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/vector.hpp>
#include <vsm/inspect.hpp>

#include <array>
#include <charconv>

using namespace vsm;
using namespace vsm::compilers;

namespace {

namespace pp_diag {

vsm_compilers_diag_module(pp);

vsm_compilers_diag(pp, error, missing_macro_name);
vsm_compilers_diag(pp, error, macro_name_is_not_an_identifier);
vsm_compilers_diag(pp, error, redefining_builtin_macro_as_parameter, std::string_view);
vsm_compilers_diag(pp, error, duplicate_macro_parameter, std::string_view);
vsm_compilers_diag(pp, error, invalid_tokens_in_macro_parameter_list);
vsm_compilers_diag(pp, error, missing_l_paren);
vsm_compilers_diag(pp, error, missing_matching_l_paren);
vsm_compilers_diag(pp, error, missing_matching_r_paren, source_location);
vsm_compilers_diag(pp, error, missing_matching_r_angle, source_location);
vsm_compilers_diag(pp, error, duplicate_macro_ellipsis);
vsm_compilers_diag(pp, error, concat_at_start_of_macro);
vsm_compilers_diag(pp, error, concat_at_end_of_macro);
vsm_compilers_diag(pp, error, concat_at_start_of_va_opt);
vsm_compilers_diag(pp, error, concat_at_end_of_va_opt);
vsm_compilers_diag(pp, error, nested_va_opt);
vsm_compilers_diag(pp, error, va_opt_in_non_variadic_macro);
vsm_compilers_diag(pp, error, missing_va_opt_l_paren);
vsm_compilers_diag(pp, error, va_args_in_non_variadic_macro);
vsm_compilers_diag(pp, error, else_without_if);
vsm_compilers_diag(pp, error, else_after_else, std::string_view);
vsm_compilers_diag(pp, error, non_dec_integer_in_line_directive);
vsm_compilers_diag(pp, error, missing_integer_in_line_directive);
vsm_compilers_diag(pp, error, endif_without_if);
vsm_compilers_diag(pp, error, redefining_builtin_macro, std::string_view);
vsm_compilers_diag(pp, error, undefining_builtin_macro, std::string_view);
vsm_compilers_diag(pp, error, missing_include_path);
vsm_compilers_diag(pp, error, include_file_not_found, std::string_view);
vsm_compilers_diag(pp, error, user_error, std::string_view);
vsm_compilers_diag(pp, error, invalid_directive);
vsm_compilers_diag(pp, error, expected_expression);
vsm_compilers_diag(pp, error, invalid_binary_operator);
vsm_compilers_diag(pp, error, defined_macro_expansion);
vsm_compilers_diag(pp, error, has_include_outside_of_conditional);
vsm_compilers_diag(pp, error, has_cpp_attribute_outside_of_conditional);
vsm_compilers_diag(pp, error, missing_cpp_attribute_name);
vsm_compilers_diag(pp, error, missing_macro_name_string);
vsm_compilers_diag(pp, error, missing_pragma_diagnostic_name);
vsm_compilers_diag(pp, error, unrecognized_diagnostic_name, std::string_view);
vsm_compilers_diag(pp, error, missing_pragma_diagnostic_directive);
vsm_compilers_diag(pp, error, invalid_pragma_diagnostic_directive, std::string_view);
vsm_compilers_diag(pp, error, missing_pragma_message_string);
vsm_compilers_diag(pp, error, pushing_builtin_macro, std::string_view);
vsm_compilers_diag(pp, warning, reserved_identifier, std::string_view);
vsm_compilers_diag(pp, warning, oct_integer_in_line_directive);
vsm_compilers_diag(pp, warning, redefining_macro, std::string_view);
vsm_compilers_diag(pp, warning, user_warning, std::string_view);
vsm_compilers_diag(pp, warning, va_opt_outside_of_macro);
vsm_compilers_diag(pp, warning, va_args_outside_of_macro);
vsm_compilers_diag(pp, warning, pragma_once_in_main_file);
vsm_compilers_diag(pp, message, user_message, std::string_view);

vsm_compilers_diag_name(pp, "pp-reserved-identifier", reserved_identifier);
vsm_compilers_diag_name(pp, "pp-octal-line-directive", oct_integer_in_line_directive);
vsm_compilers_diag_name(pp, "pp-macro-redefinition", redefining_macro);
vsm_compilers_diag_name(pp, "pp-user-warning", user_warning);

} // namespace pp_diag

enum class token_kind : uint8_t
{
	hash,
	hash_hash,
	identifier,
	string,
	character,
	l_paren,
	r_paren,
	l_angle,
	r_angle,
	integer_bin,
	integer_oct,
	integer_dec,
	integer_hex,
	true_,
	false_,
	plus,
	minus,
	star,
	slash,
	percent,
	less_less,
	greater_greater,
	tilde,
	ampersand,
	caret,
	pipe,
	exclamation,
	ampersand_ampersand,
	pipe_pipe,
	equal_equal,
	exclamation_equal,
	less,
	greater,
	less_equal,
	greater_equal,
	comma,
	ellipsis,
	end_of_input,
	end_of_file,
};
using tk = token_kind;

class token
{
	token_kind m_kind;
	source_location m_location;
	bool m_has_leading_whitespace : 1;
	bool m_is_at_beginning_of_line : 1;
	bool m_is_expanded_from_macro : 1;

public:
	token_kind kind() const
	{
		return m_kind;
	}

	source_location location() const
	{
		return m_location;
	}

	std::string_view content() const;

	bool has_leading_whitespace() const;
	bool is_at_beginning_of_line() const;
	bool is_expanded_from_macro() const;
};

class token_stream
{
	token const* m_tokens_beg = nullptr;
	token const* m_tokens_end = nullptr;
	token const* m_tokens_pos = nullptr;

public:
	token_stream(token_stream const&) = delete;
	token_stream& operator=(token_stream const&) = delete;

	token const& peek()
	{

	}

	void consume_one()
	{
	}

protected:
	token_stream() = default;
	~token_stream() = default;

	void set_tokens(std::span<token const> const tokens)
	{
		m_tokens_beg = tokens.data();
		m_tokens_end = tokens.data() + tokens.size();
		m_tokens_pos = tokens.data();
	}

	virtual void refill() = 0;
};

class lexer final : public token_stream
{
	source_reference m_reference;

	char const* m_beg;
	char const* m_end;
	char const* m_pos;

public:
	explicit lexer(source const& source)
	{
	}

protected:
	void refill() override
	{

	}
};

class synthetic_token_stream : public token_stream
{
	vector<token> m_tokens;
	size_t m_index = 0;

public:
	void emit(token const& t)
	{
		m_tokens.push_back(t);
	}

protected:
	void refill() override
	{

	}
};

#if 0
class lexer
{
	static constexpr size_t max_lookback = 1;
	static constexpr size_t max_lookahead = 0;

	static constexpr size_t buffer_size = 1 + max_lookback + max_lookahead;
	
public:
	class position
	{
		size_t offset;
		uint32_t line;
		std::array<tk, max_lookback> lookback;
	};

private:
	char const* m_source_beg;
	char const* m_source_pos;
	char const* m_source_end;

	char const* m_token_beg;
	char const* m_token_end;

	tk m_tokens[buffer_size];
	size_t m_token_offset;
	uint32_t m_current_line;
	
public:
	lexer()
	{
		set_source("");
	}

	tk peek(ptrdiff_t const offset = 0)
	{
	}
	
	std::string_view peek_content()
	{
	}
	
	void set_source(std::string_view const source, position const position = {})
	{
		vsm_assert(position.offset <= source.size());
	
		m_source_beg = source.data();
		m_source_pos = m_source_beg + position.offset;
		m_source_end = m_source_beg + source.size();

		m_token_beg = nullptr;
		m_token_end = nullptr;
	}
	
	position get_position() const
	{
		return {};
	}
	
	uint32_t get_current_line()
	{
		return m_current_line;
	}
};

struct include_context
{
	preprocessor_include file;
	lexer::position position;
};

enum class macro_instruction : uint8_t
{
	token,
	argument,
	str_enter,
	str_leave,
	concat,
	va_opt,
	va_args,
	end_of_stream,
};

struct macro_definition
{
	size_t min_arguments;
	bool is_function_like;
	bool is_variadic;
	std::byte stream[];
};

class macro
{
	typedef void builtin_macro_handler(preprocessor& pp);

	bool m_is_builtin;
	bool m_is_poisoned;
	bool m_is_dynamic;
	bool m_is_variable_like;
	bool m_is_function_like;

	union
	{
		builtin_macro_handler* m_handler;
		macro_definition const* m_definition;
		diagnostic m_poison_diagnostic;
	};

public:
	bool is_poisoned() const
	{
		return m_is_poisoned;
	}


	static macro poisoned_macro(diagnostic const diagnostic)
	{
		macro m = {};
		m.m_is_poisoned = true;
		m.m_diagnostic = diagnostic;
		return m;
	}

	static macro variable_macro(builtin_macro_handler* const handler)
	{
		macro m = {};
		m.m_is_builtin = true;
		m.m_is_variable_like = true;
		m.m_handler = handler;
		return m;
	}

	static macro function_macro(builtin_macro_handler* const handler)
	{
		macro m = {};
		m.m_is_builtin = true;
		m.m_is_function_like = true;
		m.m_handler = handler;
		return m;
	}

	static macro operator_macro(builtin_macro_handler* const handler)
	{
		macro m = {};
		m.m_is_builtin = true;
		m.m_is_variable_like = true;
		m.m_is_function_like = true;
		m.m_handler = handler;
		return m;
	}

	static macro user_defined_macro(macro_definition const* const definition, bool const function_like, bool const dynamic = true)
	{
		macro m = {};
		m.m_is_dynamic = dynamic;
		m.m_is_variable_like = !function_like;
		m.m_is_function_like = function_like;
		m.m_definition = definition;
		return m;
	}

private:
	macro() = default;
};
#endif

} // namespace

struct preprocessor_base::preprocessor_impl : preprocessor_base
{
	enum class macro_type
	{
		builtin,
		builtin_stacked,
		user_defined,
		user_defined_stacked,
	};

	enum class macro_flags : uint8_t
	{
		builtin                     = 1 << 0,
		stacked                     = 1 << 1,
		expanding                   = 1 << 2,

		type_mask                   = builtin | stacked,
	};

	template<typename T>
	struct macro_traits
	{
		static constexpr macro_type type_value = T::static_type;
		using view_type = T;
	};

	template<>
	struct macro_traits<builtin_macro>
	{
		static constexpr macro_type type_value = macro_type::builtin;
		using view_type = builtin_macro const;
	};

	template<typename T>
	struct macro_traits<T const> : macro_traits<T> {};

	struct macro_deleter
	{
		using pointer = tag_ptr<macro const, macro_type>;

		void vsm_static_operator_invoke(pointer const ptr)
		{
			if (ptr.tag() != macro_type::builtin)
			{
				delete_macro(const_cast<macro*>(ptr.ptr()), ptr.tag());
			}
		}
	};
	using macro_ptr = std::unique_ptr<macro const, macro_deleter>;

	struct builtin_stacked_macro : macro
	{
		static constexpr macro_type static_type = macro_type::builtin_stacked;

		small_vector<builtin_macro const*, 2> stack;
	};

	struct user_defined_macro : macro
	{
		static constexpr macro_type static_type = macro_type::user_defined;

		enum class macro_instruction : uint8_t
		{
			token,
			argument,
			str_enter,
			str_leave,
			concat,
			va_opt,
			va_args,
			end_of_stream,
		};

		size_t formal_arguments;
		bool is_function_like;
		bool variadic_arguments;
		std::byte stream[];

		explicit user_defined_macro(bool const is_function_like,
			size_t const formal_arguments, bool const is_variadic,
			std::span<std::byte const> const stream)
			: formal_arguments(formal_arguments)
			, is_function_like(is_function_like)
			, variadic_arguments(is_variadic)
		{
			std::uninitialized_copy_n(stream.data(), stream.size(), this->stream);
		}
	};

	struct user_defined_stacked_macro : macro
	{
		static constexpr macro_type static_type = macro_type::user_defined_stacked;

		struct stack_element
		{
			macro_ptr macro;

			// How many times this definition has been pushed.
			size_t push_count;
		};

		small_vector<stack_element, 2> stack;

		explicit user_defined_stacked_macro(macro_ptr macro)
		{
			stack.emplace_back(vsm_move(macro), 0);
		}
	};

	friend macro_type tag_invoke(inspect_cpo, macro_ptr const& ptr)
	{
		return ptr.get().tag();
	}

	template<typename T>
	friend constexpr macro_type tag_invoke(match_index_cpo, tag<macro_ptr>, tag<T*>)
	{
		return macro_traits<T>::type_value;
	}

	template<typename T>
	friend T* tag_invoke(match_value_cpo, macro_ptr const& ptr, tag<T*>)
	{
		return const_cast<macro_traits<T>::view_type*>(static_cast<T const*>(ptr.get().ptr()));
	}

	static void delete_macro(macro* const macro, macro_type const type)
	{
		switch (type)
		{
		case macro_type::builtin_stacked:
			return delete static_cast<builtin_stacked_macro*>(macro);

		case macro_type::user_defined:
			return delete static_cast<user_defined_macro*>(macro);

		case macro_type::user_defined_stacked:
			return delete static_cast<user_defined_stacked_macro*>(macro);
		}
	}

	template<typename Macro>
	static macro_ptr make_macro_ptr(Macro* const macro)
	{
		return macro_ptr(macro_ptr::pointer(macro, macro_traits<Macro>::type_value));
	}

	template<std::same_as<builtin_macro> Macro>
	static bool is_macro(macro_ptr const& macro)
	{
		return macro != nullptr && macro.get().tag() == macro_type::builtin;
	}

	template<typename Macro>
	static bool is_macro(macro_ptr const& macro)
	{
		return macro.get().tag() == Macro::static_type;
	}

	template<std::same_as<builtin_macro> Macro>
	static builtin_macro const* cast_macro(macro_ptr const& macro)
	{
		return is_macro<Macro>(macro)
			? static_cast<Macro const*>(macro.get().ptr())
			: nullptr;
	}

	template<typename Macro>
	static Macro* cast_macro(macro_ptr const& macro)
	{
		return is_macro<Macro>(macro)
			? const_cast<Macro*>(static_cast<Macro const*>(macro.get().ptr()))
			: nullptr;
	}


	struct builtin_handler_pair
	{
		builtin_handler* default_handler;
		builtin_handler* user_handler;

		void evaluate(preprocessor_base& self) const
		{
			//self.m_default_builtin_handler = default_handler;
			//vsm_defer { self.m_default_builtin_handler = nullptr; };
			//(user_handler ? user_handler : default_handler)(self);
		}
	};


	struct include_level
	{
		size_t conditional_depth;
	};

	struct conditional_level
	{
		bool was_true;
		bool got_true;
		bool got_else;
	};


	diagnostics_scope const* m_parent_diagnostics_scope;
	vector<std::unique_ptr<diagnostics_scope>> m_diagnostics_stack;

	vector<include_level> m_include_stack;
	vector<conditional_level> m_conditional_stack;

	hash_map<std::string_view, macro_ptr> m_macros;
	hash_map<std::string_view, builtin_handler_pair> m_pragmas;
	hash_map<std::string_view, builtin_handler_pair> m_directives;

	source_location m_directive_location;
	source_location m_macro_location;

	bool m_is_virtual_file = false;
	bool m_conditional = true;
	bool m_evaluate_conditionals = true;
	bool m_in_conditional_expression = false;
	bool m_in_macro_expansion = false;

	size_t m_counter = 0;

	


	std::string_view get_remaining_input() const;

	token const& peek_token();

	token_kind peek_token_kind()
	{
		return peek_token().kind();
	}

	token consume_token();

	std::optional<token> try_consume_token(token_kind const kind, auto&& predicate)
	{
		token const& t = peek_token();
		
		if (t.kind() != kind)
		{
			return std::nullopt;
		}
		
		if (!predicate(t))
		{
			return std::nullopt;
		}
		
		return consume_token();
	}

	std::optional<token> try_consume_token(token_kind const kind)
	{
		return try_consume_token(kind, [](token const& t) { return true; });
	}


	template<int Radix>
	static uintmax_t evaluate_integer(std::string_view string)
	{
		/**/ if constexpr (Radix == 0b10)
		{
			vsm_assert(string.starts_with("0b"));
			string.remove_prefix(2);
		}
		else if constexpr (Radix == 0x10)
		{
			vsm_assert(string.starts_with("0x"));
			string.remove_prefix(2);
		}

		uintmax_t value = 0;
		auto const r = std::from_chars(string.data(), string.data() + string.size(), value, Radix);
		vsm_assert(r.ec == std::errc{} && r.ptr == string.data() + string.size());
		return value;
	}

	static uintmax_t evaluate_integer(token const& t)
	{
		switch (t.kind())
		{
		case tk::integer_bin: return evaluate_integer<0b10>(t.content());
		case tk::integer_oct: return evaluate_integer<0'10>(t.content());
		case tk::integer_dec: return evaluate_integer<0+10>(t.content());
		case tk::integer_hex: return evaluate_integer<0x10>(t.content());
		}

		vsm_assume(false);
	}

	std::string_view evaluate_string(token const& t);


	void emit_token(token const& t);


	void push_conditional()
	{
		m_evaluate_conditionals = m_conditional;
		m_conditional_stack.push_back(
		{
			.was_true = std::exchange(m_conditional, false),
		});
	}

	void pop_conditional()
	{
		m_conditional = m_conditional_stack._pop_back_value().was_true;
		m_evaluate_conditionals = m_conditional_stack.empty() || m_conditional_stack.back().was_true;
	}
	
	void select_conditional();


	void file_changed();

	void emit_line();

	void push_include(preprocessor_file&& file);

	void pop_include()
	{
		m_include_stack.pop_back();
	}


	void check_user_identifier(token const& token)
	{
		vsm_assert(token.kind() == tk::identifier);
		std::string_view const id = token.content();

		if (id.front() == '_')
		{
			diagnostics()(token.location(), pp_diag::reserved_identifier, id);
		}
	}

	std::optional<token> try_consume_macro_name()
	{
		auto id = try_consume_token(tk::identifier);
		
		if (!id)
		{
			if (peek_token_kind() == tk::end_of_input)
			{
				diagnostics()(get_current_location(), pp_diag::missing_macro_name);
			}
			else
			{
				diagnostics()(get_current_location(), pp_diag::macro_name_is_not_an_identifier);
			}
		}

		return id;
	}

	bool is_builtin_macro(macro_ptr const& macro)
	{
		return macro != nullptr && macro.get().tag() < macro_type::user_defined;
	}

	bool is_builtin_macro_name(std::string_view const id)
	{
		if (macro_ptr const* const macro = m_macros.find_value(id))
		{
			return is_builtin_macro(*macro);
		}
		return false;
	}

	macro_ptr parse_macro_definition()
	{
		using instruction = user_defined_macro::macro_instruction;

		small_hash_map<std::string_view, size_t, 16> parameters;

		bool is_function_like = false;
		size_t is_variadic = 0;

		if (auto const l_paren = try_consume_token(tk::l_paren,
			[](token const& t) { return !t.has_leading_whitespace(); }))
		{
			is_function_like = true;
			bool report_invalid_tokens = true;

			while (true)
			{
				token const param = consume_token();
				
				switch (param.kind())
				{
				case tk::identifier:
					{
						auto const id = param.content();

						if (is_builtin_macro_name(id))
						{
							diagnostics()(param.location(), pp_diag::redefining_builtin_macro_as_parameter, id);
						}

						if (parameters.insert(id, parameters.size()).inserted)
						{
							check_user_identifier(param);
						}
						else
						{
							diagnostics()(param.location(), pp_diag::duplicate_macro_parameter, id);
						}
					}
					break;

				case tk::ellipsis:
					if (is_variadic++ == 1)
					{
						diagnostics()(param.location(), pp_diag::duplicate_macro_ellipsis);
					}
					break;

				default:
					if (report_invalid_tokens)
					{
						diagnostics()(param.location(), pp_diag::invalid_tokens_in_macro_parameter_list);
						report_invalid_tokens = false;
					}
					break;

				case tk::end_of_input:
					diagnostics()(param.location(), pp_diag::missing_matching_r_paren, l_paren->location());
					[[fallthrough]];

				case tk::r_paren:
					{
						
					}
					break;
				}
				break;
			}
		parameter_list_end: {}

			if (!try_consume_token(tk::r_paren))
			{
				diagnostics()(get_current_location(), pp_diag::missing_matching_r_paren, l_paren->location());
			}
		}

		small_vector<std::byte, 256> stream;
		size_t scope_stream_offset = 0;

		auto const push_stream = [&](auto const& object)
		{
			memcpy(stream._push_back_default(sizeof(object)), &object, sizeof(object));
		};

		auto const fill_stream = [&](size_t const offset, auto const& object)
		{
			vsm_assert(offset + sizeof(object) <= stream.size());
			memcpy(stream.data() + offset, &object, sizeof(object));
		};

		small_vector<bool, 8> va_opt_parentheses;
		size_t va_opt_stream_offset = 0;
		source_location va_opt_location;

		bool stringize_next = false;

		while (true)
		{
			token const t = consume_token();

			switch (t.kind())
			{
			case tk::hash:
				{
					push_stream(instruction::str_enter);
				}
				break;

			case tk::hash_hash:
				{
					if (scope_stream_offset == stream.size())
					{
						if (va_opt_parentheses.empty())
						{
							diagnostics()(t.location(), pp_diag::concat_at_start_of_macro);
						}
						else
						{
							diagnostics()(t.location(), pp_diag::concat_at_start_of_va_opt);
						}
					}
					else if (peek_token_kind() == tk::end_of_input)
					{
						if (va_opt_parentheses.empty())
						{
							diagnostics()(t.location(), pp_diag::concat_at_end_of_macro);
						}
					}
					else if (peek_token_kind() == tk::r_paren && va_opt_parentheses.size() == 1)
					{
						diagnostics()(t.location(), pp_diag::concat_at_end_of_va_opt);
					}
					else
					{
						push_stream(instruction::concat);
					}
				}
				break;

			case tk::l_paren:
				{
					if (va_opt_parentheses.empty())
					{
						goto push_token_instruction;
					}

					va_opt_parentheses.push_back(true);
				}
				break;

			case tk::r_paren:
				{
					if (va_opt_parentheses.empty())
					{
						goto push_token_instruction;
					}

					if (va_opt_parentheses._pop_back_value())
					{
						goto push_token_instruction;
					}

					if (va_opt_parentheses.empty())
					{
						fill_stream(va_opt_stream_offset, stream.size());
						// leave_argument(); //TODO: What was this supposed to do?
					}
				}
				break;

			case tk::identifier:
				{
					/**/ if (t.content() == "__VA_OPT__")
					{
						if (auto const l_paren = try_consume_token(tk::l_paren))
						{
							if (!va_opt_parentheses.empty())
							{
								diagnostics()(t.location(), pp_diag::nested_va_opt);
							}
							else if (is_variadic == 0)
							{
								diagnostics()(t.location(), pp_diag::va_opt_in_non_variadic_macro);
							}

							va_opt_parentheses.push_back(false);
							push_stream(instruction::va_opt);
							va_opt_stream_offset = stream.size();
							push_stream(static_cast<size_t>(-1));
							scope_stream_offset = stream.size();
							va_opt_location = l_paren->location();
						}
						else
						{
							diagnostics()(get_current_location(), pp_diag::missing_va_opt_l_paren);
						}
					}
					else if (t.content() == "__VA_ARGS__")
					{
						if (is_variadic == 0)
						{
							diagnostics()(t.location(), pp_diag::va_args_in_non_variadic_macro);
						}
						else
						{
							push_stream(instruction::va_args);
						}
					}
					else if (auto const parameter = parameters.find_value(t.content()))
					{
						push_stream(instruction::argument);
						push_stream(*parameter);
					}
					else
					{
						goto push_token_instruction;
					}
				}
				break;

			default:
				{
				push_token_instruction:
					push_stream(instruction::token);
					push_stream(t);
				}
				break;
			}
		}

		if (!va_opt_parentheses.empty())
		{
			diagnostics()(get_current_location(), pp_diag::missing_matching_r_paren, va_opt_location);
		}

		push_stream(instruction::end_of_stream);

		static constexpr size_t fam_offset = offsetof(user_defined_macro, stream);
		void* const storage = operator new(fam_offset + stream.size());

		return make_macro_ptr(new (storage) user_defined_macro(
			is_function_like, parameters.size(), is_variadic != 0, stream));
	}

	macro_ptr& insert_macro(std::string_view const name)
	{
		return m_macros.emplace(name).element->value;
	}


	template<arithmetic_result Arithmetic(integer)>
	static integer checked_arithmetic(preprocessor_base& self, source_location const& location, integer, integer const rhs)
	{
		auto const r = Arithmetic(rhs);
		self.check_arithmetic(location, r);
		return r.value;
	}

	template<arithmetic_result Arithmetic(integer, integer)>
	static integer checked_arithmetic(preprocessor_base& self, source_location const& location, integer const lhs, integer const rhs)
	{
		auto const r = Arithmetic(lhs, rhs);
		self.check_arithmetic(location, r);
		return r.value;
	}

	bool evaluate_if()
	{
		vsm_assert(!m_in_conditional_expression);
		m_in_conditional_expression = true;
		vsm_defer{ m_in_conditional_expression = false; };

		auto macro_expansion = enable_macro_expansion(true);

		enum class precedence : uint8_t
		{
			parentheses,
			conditional,
			logical_or,
			logical_and,
			bitwise_or,
			bitwise_xor,
			bitwise_and,
			equality,
			relational,
			shift,
			additive,
			multiplicative,
			primary,
		};

		typedef integer function(preprocessor_base& self, source_location const& location, integer lhs, integer rhs);

		struct function_stack_level
		{
			precedence previous_precedence;
			integer previous_value;

			source_location location;
			function* function;
		};
		small_vector<function_stack_level, 8> function_stack;

		bool requires_primary = true;
		precedence current_precedence = precedence();
		integer current_value = 0;

		auto const end_expression = [&](source_location const& location)
		{
			if (requires_primary)
			{
				diagnostics()(location, pp_diag::expected_expression);
			}
		};

		auto const enter_primary = [&](source_location const& location)
		{
			if (!requires_primary)
			{
				diagnostics()(location, pp_diag::invalid_binary_operator);
			}
		};

		auto const leave_primary = [&](integer const value)
		{
			current_value = value;
			requires_primary = false;
		};

		auto const primary = [&](source_location const& location, integer const value)
		{
			enter_primary(location);
			leave_primary(value);
		};

		auto const enter_function = [&](source_location const& location, precedence const p, function* const f)
		{
			function_stack.emplace_back(
				std::exchange(current_precedence, p),
				std::exchange(current_value, 0),
				location, f);
			requires_primary = true;
		};

		auto const leave_function = [&]() -> integer
		{
			auto const f = function_stack._pop_back_value();
			current_value = f.function(*this, f.location, f.previous_value, current_value);
			current_precedence = f.previous_precedence;
		};

		auto const unwind_function_stack = [&](precedence const p)
		{
			while (current_precedence > p)
			{
				leave_function();
			}
		};

		auto const unary_operator = [&](source_location const& location, function* const f)
		{
			enter_function(location, precedence::primary, f);
		};

		auto const binary_operator = [&](source_location const& location, precedence const p, function* const f)
		{
			end_expression(location);
			unwind_function_stack(p);
			enter_function(location, p, f);
		};

		auto const identity = [](preprocessor_base&, source_location const&, integer, integer const rhs)
		{
			return rhs;
		};

		while (true)
		{
			token const t = consume_token();
			auto const location = t.location();

			if (t.kind() == tk::end_of_input)
			{
				break;
			}

			switch (t.kind())
			{
			case tk::true_:
				primary(location, 1);
				break;

			case tk::false_:
				primary(location, 0);
				break;

			case tk::integer_bin:
			case tk::integer_oct:
			case tk::integer_dec:
			case tk::integer_hex:
				primary(location, evaluate_integer(t));
				break;

			case tk::identifier:
				{
					enter_primary(location);

					// Undefined identifiers evaluate to 0.
					integer value = 0;

					auto const consume_parenthesized = [&](auto const& body)
					{
						if (auto const l_paren = try_consume_token(tk::l_paren))
						{
							if (body() && !try_consume_token(tk::r_paren))
							{
								diagnostics()(get_current_location(), pp_diag::missing_matching_r_paren, l_paren->location());
							}
						}
						else
						{
							diagnostics()(get_current_location(), pp_diag::missing_l_paren);
						}
					};

					std::string_view const name = t.content();
					/**/ if (name == "defined")
					{
						auto const parse_defined = [&]() -> bool
						{
							if (auto const id = ((void)enable_macro_expansion(false), try_consume_token(tk::identifier)))
							{
								value = m_macros.contains_key(id->content()) ? 1 : 0;
								return true;
							}

							diagnostics()(get_current_location(), pp_diag::missing_macro_name);
							return false;
						};

						peek_token_kind() == tk::l_paren
							? consume_parenthesized(parse_defined)
							: (void)parse_defined();
					}
					else if (name == "__has_include")
					{
						consume_parenthesized([&]() -> bool
						{
							if (auto const include = parse_include())
							{
								value = find_file(include->path, include->search_relative).has_value() ? 1 : 0;
								return true;
							}

							return false;
						});
					}
					else if (name == "__has_cpp_attribute")
					{
						consume_parenthesized([&]() -> bool
						{
							if (auto const id = try_consume_token(tk::identifier))
							{
								value = has_cpp_attribute(id->content()) ? 1 : 0;
								return true;
							}

							diagnostics()(get_current_location(), pp_diag::missing_cpp_attribute_name);
							return false;
						});
					}

					leave_primary(value);
				}
				break;

			case tk::l_paren:
				enter_primary(location);
				enter_function(location, precedence::parentheses, nullptr);
				break;

			case tk::r_paren:
				end_expression(location);
				unwind_function_stack(precedence::parentheses);

				if (!function_stack.empty())
				{
					leave_function();
				}
				else
				{
					diagnostics()(location, pp_diag::missing_matching_l_paren);
				}
				break;

			case tk::plus:
				requires_primary
					? unary_operator(location, identity)
					: binary_operator(location, precedence::additive, checked_arithmetic<add>);
				break;

			case tk::minus:
				requires_primary
					? unary_operator(location, checked_arithmetic<neg>)
					: binary_operator(location, precedence::additive, checked_arithmetic<sub>);
				break;
		
			// Simple unary operators.
			#if 1
			#define unary_operator(token, operator_name) \
			case token_kind::token: \
				unary_operator(location, checked_arithmetic<operator_name>); \
				break

				unary_operator(exclamation,     logical_not);
				unary_operator(tilde,           bitwise_not);
			#undef unary_operator
			#endif

			// Simple binary operators.
			#if 1
			#define binary_operator(token, operator_precedence, operator_name) \
			case token_kind::token: \
				binary_operator(location, precedence::operator_precedence, checked_arithmetic<operator_name>); \
				break

				binary_operator(pipe_pipe,                  logical_or,         logical_or);
				binary_operator(ampersand_ampersand,        logical_and,        logical_and);

				binary_operator(pipe,                       bitwise_or,         bitwise_or);
				binary_operator(caret,                      bitwise_xor,        bitwise_xor);
				binary_operator(ampersand,                  bitwise_and,        bitwise_and);

				binary_operator(equal_equal,                equality,           eq);
				binary_operator(exclamation_equal,          equality,           ne);

				binary_operator(less,                       relational,         lt);
				binary_operator(greater,                    relational,         gt);
				binary_operator(less_equal,                 relational,         le);
				binary_operator(greater_equal,              relational,         ge);

				binary_operator(less_less,                  shift,              shl);
				binary_operator(greater_greater,            shift,              shr);

				binary_operator(star,                       multiplicative,     mul);
				binary_operator(slash,                      multiplicative,     div);
				binary_operator(percent,                    multiplicative,     mod);
			#undef binary_operator
			#endif

			default:
				diagnostics()(location, requires_primary
					? pp_diag::expected_expression
					: pp_diag::invalid_binary_operator);
				break;
			}
		}

		end_expression(get_current_location());
		unwind_function_stack(precedence());

		if (!function_stack.empty())
		{
			vsm_assert(current_precedence == precedence::parentheses);
			diagnostics()(get_current_location(), pp_diag::missing_matching_r_paren, function_stack.back().location);
		}

		return current_value.get_unsigned_bits() != 0;
	}

	bool evaluate_ifdef(bool const expectation)
	{
		if (auto const id = try_consume_macro_name())
		{
			return m_macros.contains_key(id->content()) == expectation;
		}
		return false;
	}

	bool handle_else(bool const is_unconditional_else = false)
	{
		if (m_conditional_stack.empty())
		{
			push_conditional();
			diagnostics()(get_directive_location(), pp_diag::else_without_if);
			return true;
		}

		auto& conditional = m_conditional_stack.back();

		if (conditional.got_else)
		{
			diagnostics()(get_directive_location(), pp_diag::else_after_else, get_directive_name());
			return true;
		}

		if (is_unconditional_else)
		{
			conditional.got_else = true;
		}

		return !conditional.got_true;
	}

	struct include_info
	{
		std::string_view path;
		bool search_relative;
	};

	std::optional<include_info> parse_include_2(bool const diagnose)
	{
		switch (peek_token_kind())
		{
		case tk::string:
			return include_info
			{
				.path = evaluate_string(consume_token()),
				.search_relative = true,
			};

		case tk::l_angle:
			for (token const l_angle = consume_token();;)
			{
				token const t = consume_token();
				token_kind const kind = t.kind();

				if (kind == tk::r_angle)
				{
					return include_info
					{
						.path = "", //TODO: Get string between angle brackets.
						.search_relative = false,
					};
				}

				if (kind == tk::end_of_input)
				{
					if (diagnose)
					{
						diagnostics()(t.location(), pp_diag::missing_matching_r_angle, l_angle.location());
					}

					return std::nullopt;
				}
			}
		}

		if (diagnose)
		{
			diagnostics()(get_current_location(), pp_diag::missing_include_path);
		}

		return std::nullopt;
	}

	std::optional<include_info> parse_include()
	{
		auto include = parse_include_2(false);

		if (!include)
		{
			(void)enable_macro_expansion(true), include = parse_include_2(true);
		}

		return include;
	}

	void handle_pragma()
	{
		if (auto const id = try_consume_token(tk::identifier))
		{
			if (auto const pragma = m_pragmas.find_value(id->content()))
			{
				pragma->evaluate(*this);
				
				// Recognised pragmas are not emitted by default.
				return;
			}
		}

		emit_current_pragma();
	}


	void handle_directive_line()
	{
		auto const try_consume_simple_integer = [&]() -> std::optional<size_t>
		{
			// All integer literals are evaluated, but decimal is expected.
			// Octal integer produces a warning and is evaluated as if decimal.
			// Binary and hexadecimal produce errors, but are evaluated normally.

			switch (peek_token_kind())
			{
			case tk::integer_bin:
				diagnostics()(get_current_location(), pp_diag::non_dec_integer_in_line_directive);
				return evaluate_integer<0b10>(consume_token().content());

			case tk::integer_oct:
				diagnostics()(get_current_location(), pp_diag::oct_integer_in_line_directive);
				[[fallthrough]];

			case tk::integer_dec:
				return evaluate_integer<10>(consume_token().content());

			case tk::integer_hex:
				diagnostics()(get_current_location(), pp_diag::non_dec_integer_in_line_directive);
				return evaluate_integer<0x10>(consume_token().content());
			}

			diagnostics()(get_current_location(), pp_diag::missing_integer_in_line_directive);
			return std::nullopt;
		};

		if (auto const line = try_consume_simple_integer())
		{
			if (auto const file = try_consume_token(tk::string))
			{
				//TODO: emit line directive
			}
		}
	}

	void handle_directive_if()
	{
		push_conditional();
		if (m_evaluate_conditionals && evaluate_if())
		{
			select_conditional();
		}
	}

	void handle_directive_ifdef()
	{
		push_conditional();
		if (m_evaluate_conditionals && evaluate_ifdef(true))
		{
			select_conditional();
		}
	}

	void handle_directive_ifndef()
	{
		push_conditional();
		if (m_evaluate_conditionals && evaluate_ifdef(false))
		{
			select_conditional();
		}
	}

	void handle_directive_elif()
	{
		bool const select_else = handle_else();
		if (m_evaluate_conditionals && select_else & evaluate_if())
		{
			select_conditional();
		}
	}

	void handle_directive_elifdef()
	{
		bool const select_else = handle_else();
		if (m_evaluate_conditionals && select_else & evaluate_ifdef(true))
		{
			select_conditional();
		}
	}

	void handle_directive_elifndef()
	{
		bool const select_else = handle_else();
		if (m_evaluate_conditionals && select_else & evaluate_ifdef(false))
		{
			select_conditional();
		}
	}

	void handle_directive_else()
	{
		bool const select_else = handle_else(true);
		if (m_evaluate_conditionals && select_else)
		{
			select_conditional();
		}
	}

	void handle_directive_endif()
	{
		if (m_conditional_stack.empty())
		{
			diagnostics()(get_directive_location(), pp_diag::endif_without_if);
		}
		else
		{
			pop_conditional();
		}
	}

	void handle_directive_define()
	{
		auto const id = try_consume_macro_name();

		if (!id)
		{
			return;
		}

		auto new_macro = parse_macro_definition();

		auto const r = m_macros.emplace(id->content());
		macro_ptr* macro = &r.element->value;

		if (!r.inserted)
		{
			if (is_builtin_macro(*macro))
			{
				return diagnostics()(id->location(), pp_diag::redefining_builtin_macro, id->content());
			}

			bool diagnose_redefining = true;

			if (auto const stacked = cast_macro<user_defined_stacked_macro>(*macro))
			{
				if (stacked->stack.back().push_count != 0)
				{
					stacked->stack.emplace_back();
					diagnose_redefining = false;
				}

				macro = &stacked->stack.back().macro;
			}

			if (diagnose_redefining && *macro != nullptr)
			{
				diagnostics()(id->location(), pp_diag::redefining_macro, id->content());
			}
		}

		*macro = vsm_move(new_macro);
	}

	void handle_directive_undef()
	{
		auto const id = try_consume_macro_name();

		if (!id)
		{
			return;
		}

		macro_ptr* macro = m_macros.find_value(id->content());

		if (macro == nullptr)
		{
			return;
		}

		if (is_builtin_macro(*macro))
		{
			return diagnostics()(id->location(), pp_diag::undefining_builtin_macro, id->content());
		}

		if (auto const stacked = cast_macro<user_defined_stacked_macro>(*macro))
		{
			if (stacked->stack.back().push_count != 0)
			{
				stacked->stack.emplace_back();
				return; // Initialized to nullptr.
			}

			macro = &stacked->stack.back().macro;
		}

		*macro = nullptr;
	}

	void handle_directive_include()
	{
		auto const location = get_current_location();

		if (auto const include = parse_include())
		{
			if (auto file = find_file(include->path, include->search_relative))
			{
				push_include(vsm_move(*file));
			}
			else
			{
				diagnostics()(location, pp_diag::include_file_not_found, include->path);
			}
		}
	}

	void handle_directive_error()
	{
		diagnostics()(get_directive_location(), pp_diag::user_error, get_remaining_input());
	}

	void handle_directive_warning()
	{
		diagnostics()(get_directive_location(), pp_diag::user_warning, get_remaining_input());
	}

	void handle_directive_pragma()
	{
		handle_pragma();
	}

	void handle_directive()
	{
		if (auto const id = try_consume_token(tk::identifier))
		{
			if (auto const directive = m_directives.find_value(id->content()))
			{
				m_directive_location = id->location();
				vsm_defer { m_directive_location = {}; };
				directive->evaluate(*this);
			}
			else
			{
				diagnostics()(id->location(), pp_diag::invalid_directive);
			}
		}
		else
		{
			diagnostics()(get_current_location(), pp_diag::invalid_directive);
		}
	}


	void handle_pragma_once()
	{
		if (m_include_stack.empty())
		{
			diagnostics()(get_pragma_location(), pp_diag::pragma_once_in_main_file);
		}

		if (!m_is_virtual_file)
		{
			mark_once();
		}
	}

	void handle_pragma_push_macro()
	{
		auto const l_paren = try_consume_token(tk::l_paren);
		if (!l_paren)
		{
			return diagnostics()(get_current_location(), pp_diag::missing_l_paren);
		}

		auto const string = try_consume_token(tk::string);
		if (!string)
		{
			return diagnostics()(get_current_location(), pp_diag::missing_macro_name_string);
		}

		if (!try_consume_token(tk::r_paren))
		{
			return diagnostics()(get_current_location(), pp_diag::missing_matching_r_paren, l_paren->location());
		}

		std::string_view const name = evaluate_string(*string);

		auto const r = m_macros.emplace(name);
		macro_ptr& macro = r.element->value;

		if (!r.inserted)
		{
			if (is_builtin_macro(macro))
			{
				return diagnostics()(get_pragma_location(), pp_diag::pushing_builtin_macro, name);
			}
		}

		auto stacked = cast_macro<user_defined_stacked_macro>(macro);

		if (stacked == nullptr)
		{
			stacked = new user_defined_stacked_macro(vsm_move(macro));
			macro = make_macro_ptr(stacked);
		}

		++stacked->stack.back().push_count;
	}

	void handle_pragma_pop_macro()
	{
		auto const l_paren = try_consume_token(tk::l_paren);
		if (!l_paren)
		{
			return diagnostics()(get_current_location(), pp_diag::missing_l_paren);
		}

		auto const string = try_consume_token(tk::string);
		if (!string)
		{
			return diagnostics()(get_current_location(), pp_diag::missing_macro_name_string);
		}

		if (!try_consume_token(tk::r_paren))
		{
			return diagnostics()(get_current_location(), pp_diag::missing_matching_r_paren, l_paren->location());
		}

		std::string_view const name = evaluate_string(*string);
		macro_ptr* const macro = m_macros.find_value(name);

		if (macro == nullptr)
		{
			return;
		}

		auto const stacked = cast_macro<user_defined_stacked_macro>(*macro);

		if (stacked == nullptr)
		{
			return;
		}

		auto* top = &stacked->stack.back();

		if (top->push_count == 0)
		{
			stacked->stack.pop_back();
			top = &stacked->stack.back();
			vsm_assert(top->push_count != 0);
		}

		if (--top->push_count == 0)
		{
			if (stacked->stack.size() == 1)
			{
				*macro = vsm_move(top->macro);
			}
			else
			{
				stacked->stack.pop_back();
			}
		}
	}

	void handle_pragma_diagnostic()
	{
		auto const parse_and_set_diagnostic_level = [&](diagnostic_level const level)
		{
			if (auto const string = try_consume_token(tk::string))
			{
				std::string_view const name = evaluate_string(*string);

#if 0
				if (auto const group = m_diagnostics->get_group(name))
				{
					for (diagnostic const* const diagnostic : *group)
					{
						m_diagnostics.set_level(*diagnostic, level);
					}
				}
				else
#endif
				{
					diagnostics()(string->location(), pp_diag::unrecognized_diagnostic_name, name);
				}
			}
			else
			{
				diagnostics()(get_current_location(), pp_diag::missing_pragma_diagnostic_name);
			}
		};

		auto const id = try_consume_token(tk::identifier);

		if (!id)
		{
			return diagnostics()(get_current_location(), pp_diag::missing_pragma_diagnostic_directive);
		}
		
		std::string_view const directive = id->content();
		
		/**/ if (directive == "push")
		{
			//auto p = std::make_unique<diagnostics_scope>(vsm_move(m_diagnostics));
			//m_diagnostics = diagnostics_scope(*p);
			//m_diagnostics_stack.push_back(vsm_move(p));
		}
		else if (directive == "pop")
		{
			//if (m_diagnostics_stack.empty())
			//{
			//	m_diagnostics = diagnostics_scope();
			//}
			//else
			//{
			//	m_diagnostics = vsm_move(*m_diagnostics_stack._pop_back_value());
			//}
		}
		else if (directive == "error")
		{
			parse_and_set_diagnostic_level(diagnostic_level::error);
		}
		else if (directive == "warning")
		{
			parse_and_set_diagnostic_level(diagnostic_level::warning);
		}
		else if (directive == "ignored")
		{
			parse_and_set_diagnostic_level(diagnostic_level::none);
		}
		else
		{
			diagnostics()(id->location(), pp_diag::invalid_pragma_diagnostic_directive, directive);
		}

		emit_current_pragma();
	}

	void handle_pragma_message()
	{
		if (auto const string = try_consume_token(tk::string))
		{
			diagnostics()(get_directive_location(), pp_diag::user_message, evaluate_string(*string));
		}
		else
		{
			diagnostics()(get_current_location(), pp_diag::missing_pragma_message_string);
		}
	}


	void builtin_macro_file()
	{
		emit_string(get_macro_location(), get_current_file());
	}

	void builtin_macro_line()
	{
		emit_integer(get_macro_location(), get_current_line());
	}

	void builtin_macro_counter()
	{
		emit_integer(get_macro_location(), inc_counter_value());
	}

#if 0
	void builtin_macro_defined()
	{
		if (m_in_conditional_expression && m_in_macro_expansion)
		{
			diagnostics()(get_macro_location(), pp_diag::defined_macro_expansion);
		}

		//TODO: re-emit unexpandable identifier
	}
#endif

	void builtin_macro_has_include()
	{
		if (!m_in_conditional_expression)
		{
			diagnostics()(get_macro_location(), pp_diag::has_include_outside_of_conditional);
		}

		emit(get_macro_location(), "__has_include");
	}

	void builtin_macro_has_cpp_attribute()
	{
		if (!m_in_conditional_expression)
		{
			diagnostics()(get_macro_location(), pp_diag::has_cpp_attribute_outside_of_conditional);
		}

		emit(get_macro_location(), "__has_cpp_attribute");
	}

	void builtin_macro_pragma()
	{
	}


	void set_default_handlers()
	{
#if 0
		m_directives.insert("line"              , handle_directive_line);
		m_directives.insert("if"                , handle_directive_if);
		m_directives.insert("ifdef"             , handle_directive_ifdef);
		m_directives.insert("ifndef"            , handle_directive_ifndef);
		m_directives.insert("elif"              , handle_directive_elif);
		m_directives.insert("elifdef"           , handle_directive_elifdef);
		m_directives.insert("elifndef"          , handle_directive_elifndef);
		m_directives.insert("else"              , handle_directive_else);
		m_directives.insert("endif"             , handle_directive_endif);
		m_directives.insert("define"            , handle_directive_define);
		m_directives.insert("undef"             , handle_directive_undef);
		m_directives.insert("include"           , handle_directive_include);
		m_directives.insert("error"             , handle_directive_error);
		m_directives.insert("warning"           , handle_directive_warning);
		m_directives.insert("pragma"            , handle_directive_pragma);

		m_pragmas.insert("once"                 , handle_pragma_once);
		m_pragmas.insert("push_macro"           , handle_pragma_push_macro);
		m_pragmas.insert("pop_macro"            , handle_pragma_pop_macro);
		m_pragmas.insert("diagnostic"           , handle_pragma_diagnostic);
		m_pragmas.insert("message"              , handle_pragma_message);

		m_macros.insert("__FILE__"              , variable_macro(builtin_macro_file));
		m_macros.insert("__LINE__"              , variable_macro(builtin_macro_line));
		m_macros.insert("__COUNTER__"           , variable_macro(builtin_macro_counter));
		m_macros.insert("__VA_OPT__"            , poisoned_macro(pp_diag::va_opt_outside_of_macro));
		m_macros.insert("__VA_ARGS__"           , poisoned_macro(pp_diag::va_args_outside_of_macro));
		m_macros.insert("__has_include"         , variable_macro(builtin_macro_has_include));
		m_macros.insert("__has_cpp_attribute"   , variable_macro(builtin_macro_has_cpp_attribute));
		m_macros.insert("_Pragma"               , operator_macro(builtin_macro_pragma));
#endif
	}


#if 0
	void preprocess()
	{
		while (true)
		{
			token const token = consume_token();
		
			switch (token.kind())
			{
			case tk::identifier:
				if (macro_ptr const* macro = m_macros.find_value(token.content()))
				{
					vsm_inspect(*macro)
					{
						vsm_match(builtin_macro const* const m)
						{
							if (m->m_is_poisoned)
							{
								//TODO: diagnose
								emit_token(token);
							}
							else
							{

							}
						}

						vsm_match(builtin_stacked_macro const* const m)
						{
						}

						vsm_match(user_defined_macro const* const m)
						{

						}

						vsm_match(user_defined_stacked_macro const* const m)
						{

						}
					}
					switch (macro->get().tag())
					{
					case macro_type::builtin:
						break;

					case macro_type::builtin_stacked:
						break;

					case macro_type::user_defined_stacked:
						[[fallthrough]];

					case macro_type::user_defined:
						break;
					}
				}
				else
				{
					emit_token(token);
				}
				break;

			case tk::hash:
				if (token.is_at_beginning_of_line())
				{
					handle_directive();
				}
				else
				{
					emit_token(token);
				}
				break;

			default:
				{
					emit_token(token);
				}
				break;

			case tk::end_of_file:
				{
					if (m_include_stack.empty())
					{
						return;
					}

					pop_include();
				}
				break;
			}
		}

		
	}
#endif


	void include(source const& source)
	{
		std::string_view const text = source.get_text();

		lexer lexer(source);

		bool preserve_comments = false;
		bool preserve_whitespace = false;

		while (true)
		{
			token const& t = lexer.peek();

			switch (t.kind())
			{
			case tk::identifier:
				if (macro_ptr const* const macro = m_macros.find_value(t.content()))
				{
					vsm_inspect(*macro)
					{
						vsm_match(builtin_macro const* const m)
						{
							if (m->m_is_poisoned)
							{
								//TODO: diagnose
								emit_token(token);
							}
							else
							{

							}
						}

						vsm_match(builtin_stacked_macro const* const m)
						{
						}

						vsm_match(user_defined_macro const* const m)
						{

						}

						vsm_match(user_defined_stacked_macro const* const m)
						{

						}
					}
				}
				break;

			case tk::line_comment:
			case tk::block_comment:
				if (preserve_comments)
				{

				}
				else if (preserve_whitespace)
				{

				}
				break;

			case tk::whitespace:
				break;

			default:

				break;
			}
		}
	}

	source const& render() const
	{

	}
};

#if 0
preprocessed_source vsm::preprocess(std::string_view const source, preprocessor_context const& context, preprocessor_options const& options)
{
	preprocessor pp;

	pp.push_include({ source, options.source_path });
	pp.preprocess();
}
#endif
