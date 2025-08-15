def left(str):
	if len(str) != 0:
		str = " " + str
	return str

def specialization(cv, ref, ref_view, noexcept, nothrow):
	print(f"""
template<typename R, typename... Ps>
struct _any_traits<R(Ps...){left(cv + ref)}{left(noexcept)}>
{{
	template<typename T, typename F>
	static void type_constraint() requires _any_type{nothrow}_constraint_2<T{left(cv)}{ref_view}, F, R, Ps...>;

	void call_constraint(Ps...){left(cv + ref)}{left(noexcept)};

	using return_type = R;
	using function_type = R(void const*, Ps...){left(noexcept)};

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{{
		return F::invoke(
			const_cast<T{left(cv)}{ref_view}>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}}
}};
""")

def ref(cv, noexcept):
	nothrow = "_nothrow" if noexcept == "noexcept" else ""

	specialization(cv, ""  , "&" , noexcept, nothrow)
	specialization(cv, "&" , "&" , noexcept, nothrow)
	specialization(cv, "&&", "&&", noexcept, nothrow)

def cv(noexcept):
	ref(""     , noexcept)
	ref("const", noexcept)

if __name__ == "__main__":
	print("#if 1 // auto-generated")
	cv("")
	cv("noexcept")
	print("#endif // auto-generated")
