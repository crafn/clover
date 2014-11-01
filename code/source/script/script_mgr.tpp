
template <>
struct TypeString<TemplatePlaceholder*, false> : public TypeString<TemplatePlaceholder> {};

template <>
struct TypeString<TemplateClassPlaceholder*, false> : public TypeString<TemplateClassPlaceholder> {};

template <typename T>
T* genericNew(){
	return new T();
}

/// A to B
template <typename A, typename B, bool = util::IsBaseOf<Reference, B>::value>
struct GenericPtrCast {
	static B* cast(A* a){
		if (!a) return nullptr;
		
		B* b= dynamic_cast<B*>(a);
		if (b)
			b->addRef();
			
		return b;
	}
};

template <typename A, typename B>
struct GenericPtrCast<A, B, false> {
	static B* cast(A* a){
		if (!a) return nullptr;
	
		B* b= dynamic_cast<B*>(a);
			
		return b;
	}
};

template <typename T, typename... Args>
void genericConstruct(Args... args, T *memory){
	new (memory) T{args...};
}

template <typename T>
void genericCopyConstruct(T *other, T* memory){
	new (memory) T(*other);
}

template <typename T>
void genericDestruct(T* memory){
	memory->~T();
}

template <typename T>
util::ObjectNode objectNodeSerialized(T* value){
	return util::ObjectNodeTraits<T>::serialized(*value);
}

template <typename T>
T objectNodeDeserialized(util::ObjectNode* value){
	return (util::ObjectNodeTraits<T>::deserialized(*value));
}

/// @see MakeStdFunction<Ret, Args...>
template <typename T>
struct MakeStdFunction;

/// Used to create std::functions from function handles in script
template <typename R, typename... Args>
struct MakeStdFunction<R(Args...)> {
	static std::function<R(Args...)> invoke(asIScriptFunction* as_f){
		ensure(as_f);
		
		std::function<R(Args...)> call_to_script;
		
		if (as_f->GetFuncType() == asFUNC_SCRIPT){
			// Wrap calling of the script function call into a lambda
			call_to_script= [as_f] (Args... args) -> R {
				try {
					Function<R(Args...)> function(*as_f);
					return gScriptMgr->getFreeContext().execute(function(args...));
				}
				catch(ScriptException& e){
					throw global::Exception("std::function call to script failed: %s", e.what());
				}
			};
		}
		else if (as_f->GetFuncType() == asFUNC_DELEGATE){
			asIScriptObject* as_obj= static_cast<asIScriptObject*>(as_f->GetDelegateObject());
			asIObjectType* as_obj_type= as_f->GetDelegateObjectType();
			asIScriptFunction* as_method= as_f->GetDelegateFunction();
			ensure(as_obj);
			ensure(as_obj_type);
			ensure(as_method);
			
			// Wrap calling of the script method into a lambda
			call_to_script= [as_obj_type, as_obj, as_method] (Args... args) -> R {
				try {
					Object object(ObjectType(*as_obj_type), *as_obj, Object::RefType::Weak);
					Function<R(Args...)> method(*as_method);
					return gScriptMgr->getFreeContext().execute(object, method(args...));
				}
				catch (ScriptException& e){
					throw global::Exception("std::function call to script failed: %s", e.what());
				}
			};
		}
		else {
			release_ensure_msg(0, "Unsupported function type: %i", as_f->GetFuncType());
		}
		
		// Don't leave any dependencies behind
		as_f->Release();
		
		return call_to_script;
	}
};

template <typename Signature>
void constructStdFunctionFromHandle(asIScriptFunction* as_f, std::function<Signature>* memory){
	genericConstruct(memory);
	(*memory)= MakeStdFunction<Signature>::invoke(as_f);
}

template <typename Signature>
std::function<Signature>& assignStdFunctionFromHandle(asIScriptFunction* as_f, std::function<Signature>* memory){
	(*memory)= MakeStdFunction<Signature>::invoke(as_f);
	return *memory;
}

template <typename F>
void ScriptMgr::registerGlobalFunction(const F& function, const util::Str8& name){
	registerGlobalFunction(function, name, TypeString<F>(namespacesRemoved(name))());
}
	
template <typename F>
void ScriptMgr::registerGlobalFunction(const F& function, const util::Str8& name, const util::Str8& decl){
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering global function to script: %s", decl.cStr());
	setCurrentNamespaceFromName(name);
	int32 ret= engine->RegisterGlobalFunction(decl.cStr(), asFUNCTION(function), asCALL_CDECL);
	Utils::devErrorCheck("registerGlobalFunction", ret);
	resetCurrentNamespace();
}

template <typename T>
void ScriptMgr::registerGlobalProperty(T& property, const util::Str8& name){
	registerGlobalProperty(
		property,
		name,
		TypeString<T>(namespacesRemoved(name))());
}

template <typename T>
void ScriptMgr::registerGlobalProperty(T& property, const util::Str8& name, const util::Str8& decl){
	setCurrentNamespaceFromName(name);
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering global property to script: %s", decl.cStr());
	int32 ret= engine->RegisterGlobalProperty(decl.cStr(), const_cast<typename std::remove_const<T>::type*>(&property));
	Utils::devErrorCheck("registerGlobalProperty", ret);
	resetCurrentNamespace();
}

template <typename T>
void ScriptMgr::registerInterface(){
	util::Str8 name= TypeString<T>()();
	setCurrentNamespaceFromName(name);

	print(debug::Ch::Script, debug::Vb::Trivial, "Registering interface: %s", name.cStr());

	int32 ret= engine->RegisterInterface(namespacesRemoved(name).cStr());
	Utils::devErrorCheck("registerInterface", ret);
	
	resetCurrentNamespace();
	
	tryRegisterArraySpecialization<T*>();
}

template <typename F>
void ScriptMgr::registerInterfaceMethod(F f, const util::Str8& name){
	typedef typename util::FunctionTraits<F>::Class Class;
	typedef typename util::FunctionTraits<F>::Signature Signature;
	
	util::Str8 type_name= TypeString<Class>()();
	setCurrentNamespaceFromName(type_name);
	
	int32 ret= engine->RegisterInterfaceMethod(namespacesRemoved(type_name).cStr(), TypeString<Signature>(name)().cStr());
	Utils::devErrorCheck("registerInterfaceMethod", ret);
	
	resetCurrentNamespace();
}

template <typename OrigT>
void ScriptMgr::registerObjectType(){
	static_assert(!std::is_reference<OrigT>::value, "References not allowed as types");
	
	using PlainT= typename std::remove_pointer<OrigT>::type;
	
	util::Str8 full_typestr= TypeString<PlainT>()();
	setCurrentNamespaceFromName(full_typestr);
	util::Str8 typestr= namespacesRemoved(full_typestr);
	
	if (registeredStrings[full_typestr].count(""))
		return; // Already registered
	registeredStrings[full_typestr].pushBack("");
	
	ensure(std::is_class<PlainT>::value);
	
	int32 ret= 0;
	uint32 class_flags= asOBJ_APP_CLASS;
	
	constexpr bool is_pod= std::is_pod<PlainT>::value;
	constexpr bool is_default_constructible= std::is_default_constructible<PlainT>::value;
	constexpr bool is_copy_constructible= std::is_copy_constructible<PlainT>::value;
	constexpr bool is_destructible= std::is_destructible<PlainT>::value;
	constexpr bool is_assignable= std::is_assignable<PlainT, PlainT>::value;
	constexpr bool is_reference= util::IsBaseOf<script::Reference, PlainT>::value;
	constexpr bool is_nocount_reference= util::IsBaseOf<script::NoCountReference, PlainT>::value || (std::is_pointer<OrigT>::value && !is_reference);
	
	static_assert(	!std::is_abstract<PlainT>::value || is_nocount_reference || is_reference,
					"Can't register an abstract value type, register as an interface or a reference type instead");
	
	util::Str8 kind_str= "value";
	if (is_pod)
		kind_str= "POD";
	if (is_reference)
		kind_str= "reference";
	else if (is_nocount_reference)
		kind_str= "no-count reference";
	
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering %s type to script: %s", kind_str.cStr(), full_typestr.cStr());
	
	if (is_pod){
		ret= engine->RegisterObjectType(typestr.cStr(), sizeof(PlainT), asOBJ_VALUE | asOBJ_POD);
		Utils::devErrorCheck("RegisterObjectType for a POD-type", ret);
		resetCurrentNamespace();
		return;
	}
	
	SizeType size= 0;
	if (is_reference){
		class_flags= asOBJ_REF;
	}
	else if (is_nocount_reference){
		class_flags= asOBJ_REF | asOBJ_NOCOUNT; 
	}
	else {
		size= sizeof(PlainT);
	
		class_flags |= asOBJ_VALUE;
		
		if (is_default_constructible){
			class_flags |= asOBJ_APP_CLASS_CONSTRUCTOR;
			//print(debug::Ch::Script, debug::Vb::Trivial, "CONSTRUCTOR");
		}
		
		if (is_copy_constructible){
			class_flags |= asOBJ_APP_CLASS_COPY_CONSTRUCTOR;
			//print(debug::Ch::Script, debug::Vb::Trivial, "COPY-CONSTRUCTOR");
		}
		
		if (is_destructible){
			class_flags |= asOBJ_APP_CLASS_DESTRUCTOR;
			//print(debug::Ch::Script, debug::Vb::Trivial, "DESTRUCTOR");
		}
		
		if (is_assignable){
			class_flags |= asOBJ_APP_CLASS_ASSIGNMENT;
			//print(debug::Ch::Script, debug::Vb::Trivial, "ASSIGNMENT");
		}
		
	}
	
	ret= engine->RegisterObjectType(typestr.cStr(), size, class_flags);
	Utils::devErrorCheck("RegisterObjectType", ret);
	
	
	if (is_reference){
		//print(debug::Ch::Script, debug::Vb::Trivial, "Reference");
		tryRegisterReferenceBeh<PlainT, OrigT>();
		tryRegisterArraySpecialization<PlainT*, OrigT>();
	}
	else if (is_nocount_reference){
		//print(debug::Ch::Script, debug::Vb::Trivial, "No-count reference");
		tryRegisterNoCountReference<PlainT, OrigT>();
		tryRegisterArraySpecialization<PlainT*, OrigT>();
	}
	else {
		//print(debug::Ch::Script, debug::Vb::Trivial, "Value");
		tryRegisterConstructorBeh<PlainT, OrigT>();
		tryRegisterCopyConstructorBeh<PlainT, OrigT>();
		tryRegisterDestructorBeh<PlainT, OrigT>();
		tryRegisterAssignmentOperator<PlainT, OrigT>();
		tryRegisterEqualsOperator<PlainT, OrigT>();
		
		tryRegisterObjectNodeSupport<PlainT, OrigT>();
		tryRegisterArraySpecialization<PlainT, OrigT>();
	}
	
	resetCurrentNamespace();
}

template <template <typename> class U>
void ScriptMgr::registerTemplateType(){
	util::Str8 full_typestr= TypeString<U<TemplateClassPlaceholder>>()();
	
	setCurrentNamespaceFromName(full_typestr);
	util::Str8 typestr= namespacesRemoved(full_typestr);
	
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering template type: %s", full_typestr.cStr());
	
	int32 ret= engine->RegisterObjectType(typestr.cStr(), 0, asOBJ_REF | asOBJ_TEMPLATE);
	Utils::devErrorCheck("RegisterObjectType (template)", ret);
	
	typestr= namespacesRemoved(TypeString<U<TemplatePlaceholder>>()());
	ret= engine->RegisterObjectBehaviour(typestr.cStr(), asBEHAVE_ADDREF, "void f()", asMETHOD(Reference,addRef), asCALL_THISCALL);
	Utils::devErrorCheck("RegisterObjectBehaviour addref (template)", ret);
	ret= engine->RegisterObjectBehaviour(typestr.cStr(), asBEHAVE_RELEASE, "void f()", asMETHOD(Reference,release), asCALL_THISCALL);
	Utils::devErrorCheck("RegisterObjectBehaviour release (template)", ret);
	
	resetCurrentNamespace();
}

template <typename T, typename... Args>
void ScriptMgr::registerConstructor(){
	registerConstructor<T>(genericConstruct<T, Args...>);
}

template <typename T, typename F>
void ScriptMgr::registerConstructor(const F& function){
	static_assert(!std::is_reference<T>::value, "References not allowed as types");
	
	setCurrentNamespaceFromName(TypeString<T>()());

	int32 ret=
		engine->RegisterObjectBehaviour(namespacesRemoved(TypeString<T>()()).cStr(),
										asBEHAVE_CONSTRUCT,
										TypeString<F>("f").lastParamDiscardedStr().cStr(),
										asFUNCTION(function),
										asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("ScriptMgr::registerConstructor: RegisterObjectBehaviour", ret);
	
	resetCurrentNamespace();
}

template <typename C, typename T>
void ScriptMgr::registerMember(T ptr_to_member, const util::Str8& name){
	typedef typename util::MemberPtrTraits<T>::Member M;
	
	util::Str8 classname= TypeString<C>()();
	util::Str8 membername= TypeString<M>(name)();
	
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering member to script: %s, %s ", classname.cStr(), membername.cStr());
	
	int32 ret=
		engine->RegisterObjectProperty(
			classname.cStr(), 
			membername.cStr(),
			((size_t)(&(reinterpret_cast<C*>(100000)->*ptr_to_member))-100000)); // as in asOFFSET, but parenthesis added
			
	Utils::devErrorCheck("registerObjectProperty", ret);
}
/*
template <typename B, typename T>
void ScriptMgr::registerBaseMember(T ptr_to_member, const util::Str8& name){
	typedef B ClassType;
	typedef typename util::MemberPtrTraits<T>::MemberType MemberType;
	
	registerMemberImpl<ClassType, MemberType>(ptr_to_member, name);
}*/

template <typename C, typename F>
void ScriptMgr::registerMethod(F f, const util::Str8& name){
	typedef C Class;
	typedef typename util::FunctionTraits<F>::Return Return;
	typedef typename util::FunctionTraits<F>::Signature Signature;
	typedef typename util::FunctionTraits<F>::Ptr Ptr;
	
	util::Str8 classname= TypeString<Class>()().cStr();
	util::Str8 methodname= TypeString<Signature>(name)();
	
	if (registeredStrings[classname].count(methodname)) return; // Already registered
	registeredStrings[classname].pushBack(methodname);
	
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering method to script: %s, %s", classname.cStr(), methodname.cStr());
	
	int32 ret=	engine->RegisterObjectMethod(
					classname.cStr(),
					methodname.cStr(),
					asSMethodPtr<sizeof(void (Class::*)())>::Convert(AS_METHOD_AMBIGUITY_CAST(Ptr)(f)),
					asCALL_THISCALL);

	Utils::devErrorCheck("RegisterObjectMethod", ret);
	
}

/*template <template <typename> class T, typename C, template <typename> class F>
void ScriptMgr::registerTemplateMethod(F f, const util::Str8& base_name){
	const util::DynArray<util::Str8>& typenames= typesByTraitsSupport[traitsId<T>()];
	

}*/

template <typename B, typename D>
void ScriptMgr::registerInheriting(){
	util::Str8 derived_local_typestr= namespacesRemoved(TypeString<D>()());
	util::Str8 base_local_typestr= namespacesRemoved(TypeString<B>()());
	
	setCurrentNamespaceFromName(TypeString<B>()());
	int32 ret= engine->RegisterObjectBehaviour(
				base_local_typestr.cStr(),
				asBEHAVE_REF_CAST,
				util::Str8(TypeString<D>()() + "@ f()").cStr(),
				asFUNCTION((GenericPtrCast<B,D>::cast)),
				asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("Register cast from base to derived", ret);
	
	setCurrentNamespaceFromName(TypeString<D>()());
	ret= engine->RegisterObjectBehaviour(
			derived_local_typestr.cStr(),
			asBEHAVE_IMPLICIT_REF_CAST, 
			util::Str8(TypeString<B>()() + "@ f()").cStr(),
			asFUNCTION((GenericPtrCast<D,B>::cast)),
			asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("Register cast from derived to base", ret);
	
	resetCurrentNamespace();
}

template <typename E>
void ScriptMgr::registerEnumType(){
	int32 ret=
		engine->RegisterEnum(namespacesRemoved(TypeString<E>()()).cStr());
	
	Utils::devErrorCheck("RegisterEnumType", ret);
}

template <typename E>
void ScriptMgr::registerEnumValue(const util::Str8& name, int32 value){
	int32 ret=
		engine->RegisterEnumValue(namespacesRemoved(TypeString<E>()()).cStr(), name.cStr(), value);
	
	Utils::devErrorCheck("RegisterEnumValue", ret);
}

template <typename Signature>
void ScriptMgr::registerFuncdef(){
	
	util::Str8 funcdef_name= TypeString<Signature>().getFuncdefStr();
	util::Str8 decl= TypeString<Signature>(funcdef_name)();
	
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering funcdef: %s", decl.cStr());
	int32 ret= engine->RegisterFuncdef(decl.cStr());
	Utils::devErrorCheck("Funcdef", ret);
	
	//
	// std::function interface
	//
	
	registerObjectType<std::function<Signature>>();
	
	util::Str8 std_func_type_str= TypeString<std::function<Signature>>()();
	
	// makeStdFunction
	util::Str8 make_func_decl= std_func_type_str + 
					" makeStdFunction_" + funcdef_name + "(" + funcdef_name + "@)";
	ret= engine->RegisterGlobalFunction(make_func_decl.cStr(), asFUNCTION(MakeStdFunction<Signature>::invoke), asCALL_CDECL);
	Utils::devErrorCheck("makeStdFunction", ret);
	
	// Constructing from function handle
	ret= engine->RegisterObjectBehaviour(std_func_type_str.cStr(), asBEHAVE_CONSTRUCT,
				util::Str8("void f(" + funcdef_name + "@)").cStr(), asFUNCTION(constructStdFunctionFromHandle<Signature>), asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("std::function construction from function handle", ret);
	
	// Copying from function handle
	ret= engine->RegisterObjectMethod(std_func_type_str.cStr(),
				util::Str8(std_func_type_str + "& opAssign(" + funcdef_name + "@)").cStr(), asFUNCTION(assignStdFunctionFromHandle<Signature>), asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("std::function copying from function handle", ret);
}

template <typename T>
void ScriptMgr::registerConstructorBeh(){
	auto ns= engine->GetDefaultNamespace();
	registerConstructor<T>();
	engine->SetDefaultNamespace(ns);
}

template <typename T>
void ScriptMgr::registerCopyConstructorBeh(){
	util::Str8 name= namespacesRemoved(TypeString<T>()());
	int32 ret= engine->RegisterObjectBehaviour(name.cStr(), asBEHAVE_CONSTRUCT,
				util::Str8("void f(const " + name + " & in)").cStr(), asFUNCTION(genericCopyConstruct<T>), asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("RegisterObjectBehaviour copy-construct", ret);
}

template <typename T>
void ScriptMgr::registerDestructorBeh(){
	util::Str8 name= namespacesRemoved(TypeString<T>()());
	int32 ret= engine->RegisterObjectBehaviour(name.cStr(), asBEHAVE_DESTRUCT,
			"void f()", asFUNCTION(genericDestruct<T>), asCALL_CDECL_OBJLAST);
	Utils::devErrorCheck("RegisterObjectBehaviour destruct", ret);
	//print(debug::Ch::Script, debug::Vb::Trivial, "Destructor registered: %s", name.cStr());
}

template <typename T>
void ScriptMgr::registerAssignmentOperator(){
	util::Str8 name= namespacesRemoved(TypeString<T>()());
	int32 ret=
		engine->RegisterObjectMethod(name.cStr(),
		util::Str8(name + "& opAssign(const " + name + " &in)").cStr(),  asMETHODPR(T, operator=, (const T&), T&), asCALL_THISCALL);
	Utils::devErrorCheck("RegisterObjectBehaviour assignment", ret);
}

template <typename T>
void ScriptMgr::registerEqualsOperator(){
	util::Str8 name= namespacesRemoved(TypeString<T>()());
	
	int32 ret= engine->RegisterObjectMethod(name.cStr(), util::Str8("bool opEquals(const " + name + "& in) const").cStr(), 
						asMETHODPR(T, operator==, (const T& in) const, bool), asCALL_THISCALL);
	Utils::devErrorCheck("RegisterObjectMethod assignment", ret);
}

template <typename T>
void ScriptMgr::registerReferenceBeh(){
	util::Str8 name= namespacesRemoved(TypeString<T>()());
	print(debug::Ch::Script, debug::Vb::Trivial, "Registering reference behaviour for type %s", name.cStr());
		
	int32 ret= engine->RegisterObjectBehaviour(name.cStr(), asBEHAVE_FACTORY, util::Str8(name + "@ f()").cStr(), asFUNCTION(genericNew<T>), asCALL_CDECL);
	Utils::devErrorCheck("RegisterObjectBehaviour factory", ret);
	
	ret= engine->RegisterObjectBehaviour(name.cStr(), asBEHAVE_ADDREF, "void f()", asMETHOD(T, addRef), asCALL_THISCALL);
	Utils::devErrorCheck("RegisterObjectBehaviour addRef", ret);
		
	ret= engine->RegisterObjectBehaviour(name.cStr(), asBEHAVE_RELEASE, "void f()", asMETHOD(T, release), asCALL_THISCALL);
	Utils::devErrorCheck("RegisterObjectBehaviour release", ret);
}

template <typename T>
void ScriptMgr::registerNoCountReference(){
	util::Dynamic<T>::registerToScript(*this);
	util::UniquePtr<T>::registerToScript(*this);
	util::SharedPtr<T>::registerToScript(*this);
}

template <typename T>
void ScriptMgr::registerObjectNodeSupport(){
	/// @todo util::ObjectNode support :p
	
	// Everything that is compatible with util::ObjectNode is compatible with CVar as well
	registerMethod(&global::CVar::set<T>, "opAssign");
	
	registerMethod(&global::CVar::set<T>, "set");
	registerMethod(&global::CVar::get<T>, "get" + asTemplatePostfix(TypeString<T>()()));
}

template <typename T>
void ScriptMgr::registerArraySpecialization(){
	registerObjectType<util::DynArray<T>>();
	
	util::Str8 name= TypeString<T>()();
	util::Str8 arrname= TypeString<util::DynArray<T>>()();
	
	registerMethod(static_cast<const T& (util::DynArray<T>::*)(SizeType) const>(&util::DynArray<T>::at), "opIndex");
	registerMethod(static_cast<T& (util::DynArray<T>::*)(SizeType)>(&util::DynArray<T>::at), "opIndex");
	registerMethod(static_cast<void (util::DynArray<T>::*)(const T&)>(&util::DynArray<T>::pushBack), "pushBack");
	registerMethod(&util::DynArray<T>::clear, "clear");
	registerMethod(&util::DynArray<T>::empty, "empty");
	registerMethod(&util::DynArray<T>::size, "size");
	
	tryRegisterArraySpecializationEqualityDependent<T, T>();
}

template <typename T>
void ScriptMgr::registerArraySpecializationEqualityDependent(){
	registerMethod(&util::DynArray<T>::count, "count");
}