template <typename F>
Function<F> Module::getGlobalFunction(const util::Str8& name) const {
	util::Str8 decl= TypeString<F>(name)();
	
	asIScriptFunction* func= module->GetFunctionByDecl(decl.cStr());
	
	if (!func) throw ScriptException(
		"Couldn't find script function %s", decl.cStr());
		
	return Function<F>(*func);
}