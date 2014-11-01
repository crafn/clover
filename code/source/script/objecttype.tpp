template <typename F>
Function<F> ObjectType::getMethod(const util::Str8& name) const {
	ensure(type);
	
	util::Str8 decl= TypeString<F>(name)();
	
	asIScriptFunction* f= type->GetMethodByDecl(decl.cStr());
	if (!f) throw ScriptException(
		"Script member function %s not found", decl.cStr());
	
	return Function<F>(*f);
}