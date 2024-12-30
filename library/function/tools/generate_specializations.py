def left(str):
	if len(str) != 0:
		str = " " + str
	return str

def specialization(cv, ref, ref_view, noex, noex_bool, trait):
	print(f"""
template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...){left(cv + ref)}{left(noex)}>
	: _function_base<Capacity, {noex_bool}, R, Ps...>
{{
	using result_type = R;

	template<typename U>
	using view_type = U{left(cv) + ref_view};

	template<typename U>
	static constexpr bool is_invocable ={trait};

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args){left(cv + ref)}{left(noex)}
	{{
		return this->invoke(vsm_forward(args)...);
	}}
}};
""")

def ref(cv, noex, noex_bool, trait):
	eol = "\n\t\t"

	def args(ref):
		return f"<R, U{left(cv) + ref}, Ps...>"

	args_v = args("")
	args_r = args("&")

	trait_v = f"{eol}std::{trait}{args_v}"
	trait_r = f"{eol}std::{trait}{args_r}"

	specialization(cv, ""  , "&" , noex, noex_bool, f"{trait_v} &&{trait_r}")
	specialization(cv, "&" , "&" , noex, noex_bool, trait_r)
	specialization(cv, "&&", "&&", noex, noex_bool, trait_v)

def cv(noex, noex_bool, trait):
	ref(""     , noex, noex_bool, trait)
	ref("const", noex, noex_bool, trait)

if __name__ == "__main__":
	print("#if 1 // auto-generated")
	cv(""        , "false", "is_invocable_r_v")
	cv("noexcept", "true" , "is_nothrow_invocable_r_v")
	print("#endif // auto-generated")
