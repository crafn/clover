
/// By value
template <typename T>
struct Context::ValueTranslator {
	static int32 setArgument(asIScriptContext& context, SizeType n, T& value){
		return context.SetArgObject(n, const_cast<typename std::remove_const<T>::type*>(&value));
	}
	
	/// Won't compile e.g. T = const SomeAbstractType& if the return value is plain T.. why? (tries to instantiate SomeAbstractType)
	/// Worked with gcc 4.7.3 but not 4.8.0
	static T& getReturnValue(asIScriptContext& context){
		return *static_cast<T*>(context.GetReturnObject());
	}
	
};

template <>
struct Context::ValueTranslator<void> {
	static void getReturnValue(asIScriptContext& context){}
};

/// By reference
template <typename T>
struct Context::ValueTranslator<T&> : public Context::ValueTranslator<T> {
	static int32 setArgument(asIScriptContext& context, SizeType n, T& value){
		return context.SetArgObject(n, const_cast<typename std::remove_const<T>::type*>(&value));
	}
	
	static T& getReturnValue(asIScriptContext& context){
		return *static_cast<T*>(context.GetReturnAddress());
	}
};

/// By pointer
template <typename T>
struct Context::ValueTranslator<T*> {
	static int32 setArgument(asIScriptContext& context, SizeType n, T* value){
		return context.SetArgObject(n, const_cast<typename std::remove_const<T>::type*>(value));
	}
	
	static T* getReturnValue(asIScriptContext& context){
		return static_cast<T*>(context.GetReturnAddress());
	}
};

/// real32
template <>
struct Context::ValueTranslator<real32> {
	static int32 setArgument(asIScriptContext& context, SizeType n, real32 value){
		return context.SetArgFloat(n, value);
	}
	
	static real32 getReturnValue(asIScriptContext& context){
		return context.GetReturnFloat();
	}
};

/// real64
template <>
struct Context::ValueTranslator<real64> {
	static int32 setArgument(asIScriptContext& context, SizeType n, real64 value){
		return context.SetArgDouble(n, value);
	}
	
	static real64 getReturnValue(asIScriptContext& context){
		return context.GetReturnDouble();
	}
};

/// int32
template <>
struct Context::ValueTranslator<int32> {
	static int32 setArgument(asIScriptContext& context, SizeType n, int32 value){
		return context.SetArgDWord(n, value);
	}
	
	static int32 getReturnValue(asIScriptContext& context){
		return context.GetReturnDWord();
	}
};

/// int64
template <>
struct Context::ValueTranslator<int64> {
	static int32 setArgument(asIScriptContext& context, SizeType n, int64 value){
		return context.SetArgQWord(n, value);
	}
	
	static int64 getReturnValue(asIScriptContext& context){
		return context.GetReturnQWord();
	}
};

/// Meta-class for setting all arguments in the template pack
template <SizeType N>
struct Context::ArgumentListSetter {
	template <typename... Args>
	void operator()(asIScriptContext& c, util::Tuple<Args...>& args) const {
		int32 ret= ValueTranslator<typename util::TupleElement<N-1,util::Tuple<Args...>>::type>::setArgument(c, N-1, std::get<N-1>(args));
		Utils::devErrorCheck("ArgumentListSetter<N>", ret);
		ArgumentListSetter<N-1>()(c, args);
	}
};

template <>
struct Context::ArgumentListSetter<1> {
	template <typename... Args>
	void operator()(asIScriptContext& c, util::Tuple<Args...>& args) const {
		int32 ret= ValueTranslator<typename util::TupleElement<0,util::Tuple<Args...>>::type>::setArgument(c, 0, std::get<0>(args));
		Utils::devErrorCheck("ArgumentListSetter<1>", ret);
	}
};

template <>
struct Context::ArgumentListSetter<0> {
	template <typename... Args>
	void operator()(asIScriptContext& c, util::Tuple<Args...>& args) const {}
};



template <typename... Args>
void Context::setArguments(util::Tuple<Args...>& args){
	ensure(context);
	/// @todo Don't set arguments if not necessary
	ArgumentListSetter<sizeof...(Args)>()(*context, args);
}

template <typename F>
typename FunctionEntry<F>::ReturnValue Context::execute(FunctionEntry<F>& f){
	ensure(context);

	prepare(f);
	setArguments(f.getArguments());
	
	executeImpl();
	
	return ValueTranslator<typename FunctionEntry<F>::ReturnValue>::getReturnValue(*context);
}

template <typename F>
typename FunctionEntry<F>::ReturnValue Context::execute(FunctionEntry<F>&& f){
	return execute(f);
}

template <typename F>
typename FunctionEntry<F>::ReturnValue Context::execute(Object& o, FunctionEntry<F>& f){
	ensure(context);

	prepare(f);
	context->SetObject(&o.getAsObject());
	
	setArguments(f.getArguments());
	
	executeImpl();
	
	return ValueTranslator<typename FunctionEntry<F>::ReturnValue>::getReturnValue(*context);
}

template <typename F>
typename FunctionEntry<F>::ReturnValue Context::execute(Object& o, FunctionEntry<F>&& f){
	return execute(o, f);
}

template <typename F>
void Context::prepare(FunctionEntry<F>& f){
	/// @todo Don't prepare if not necessary
	ensure(context);
	int32 ret= context->Prepare(&f.getAsFunction());
	Utils::devErrorCheck(util::Str8::format("Context preparing failed (state %i)", context->GetState()), ret);
}