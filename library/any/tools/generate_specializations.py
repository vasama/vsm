def left(str):
	if len(str) != 0:
		str = " " + str
	return str

def specialization(cv, ref, ref_view, noex, noex_bool, trait):
	print(f"""
template<typename R, typename... Ps>
struct _any_traits<R(Ps...){left(cv + ref)}{left(noex)}>
{{
	template<typename T>
	using member_type = R(T::*)(Ps...){left(cv + ref)}{left(noex)};

	void test_invoke(Ps...){left(cv + ref)};

	template<typename F, typename T>
	static constexpr bool requirement = _any_requirement<T, F, R, Ps...>;

	using context_type = void{left(cv)};

	using function_type = R(context_type*, Ps...);

	template<typename F, typename T, bool Packed>
	static R invoke(context_type* const context, Ps... args)
	{{
		if constexpr (Packed)
		{{
			return F::invoke(static_cast<T const&>(bit_unpack<T>(context)), vsm_move(args)...);
		}}
		else
		{{
			return F::invoke(*static_cast<T const*>(context), vsm_move(args)...);
		}}
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
