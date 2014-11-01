#ifndef CLOVER_SCRIPT_SCRIPT_MGR_HPP
#define CLOVER_SCRIPT_SCRIPT_MGR_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "util/hashmap.hpp"
#include "util/stack.hpp"
#include "util/traits.hpp"
// .tpp
#include "global/cvar.hpp"
#include "typestring.hpp"
#include "script/context.hpp"
#include "script/function.hpp"
#include "script/reference.hpp"
#include "util.hpp"
#include "util/shared_ptr.hpp"
#include "util/traits.hpp"
#include "util/unique_ptr.hpp"
#include "util/dynamic.hpp"

#include <angelscript.h>

namespace clover {
namespace script {

class Context;

class ScriptMgr : public global::EventReceiver {
public:
	ScriptMgr();
	virtual ~ScriptMgr();
	
	/// @todo Add namespace handling to all registration methods
	
	template <typename F>
	void registerGlobalFunction(const F& function, const util::Str8& name);
	
	template <typename F>
	void registerGlobalFunction(const F& function, const util::Str8& name, const util::Str8& decl);

	/// Registers a global instance for the script
	/// @todo Accessors
	template <typename T>
	void registerGlobalProperty(T& property, const util::Str8& name);
	
	template <typename T>
	void registerGlobalProperty(T& property, const util::Str8& name, const util::Str8& decl);
	
	/// Registers class as a script interface so that script classes can inherit from it
	/// @note 	If derived types are no-count refs, use register class as object type instead.
	///			Otherwise destructors will be called.
	/// Usage: registerInterface<Class>();
	template <typename T>
	void registerInterface();
	
	/// Registers required method for script interface
	/// Usage: registerInterfaceMethod(&Class::method, "method");
	template <typename F>
	void registerInterfaceMethod(F f, const util::Str8& name);
	
	
	/// Registers an object type for the script
	/// Handles default constructor, destructor and assignment operator automatically
	/// Type has to have TypeStringTraits<T> defined
	/// Can be also used to register template specializations, e.g. Type<char>
	template <typename T>
	void registerObjectType();

	/// Template type registration. Must be called before registering specializations for a template
	/// @todo Automatic factories for template types, now only specializations can be used
	template <template <typename> class U>
	void registerTemplateType();

	/// Registers additional constructor
	template <typename T, typename... Args>
	void registerConstructor();
	
	/// F: void construct(<params>, void* memory){ new (memory) T; }
	template <typename T, typename F>
	void registerConstructor(const F& f);
	
	/// Usage: registerMember(&Class::member, "member");
	/// In case of base class' member is wanted to be registered for inherited class
	/// Usage: registerMember<Class>(&Class::memberOfBaseClass, "member");
	/// @todo Accessors
	template <typename C, typename T>
	void registerMember(T ptr_to_member, const util::Str8& name);
	
	template <typename T>
	void registerMember(T ptr_to_member, const util::Str8& name){
		registerMember<typename util::MemberPtrTraits<T>::Class>(ptr_to_member, name);
	}
	
	/// Registers a method for the script
	/// Usage: registerMethod(&Class::method, "method");
	/// In case of base class' method is wanted to be registered for inherited class
	/// Usage: registerMethod<Class>(&Class::methodOfBaseClass, "method");
	/// In case of overloaded methods
	/// Usage: registerMethod<Class, void (Class::*)(int)>(&Class::overloaded, "overloaded")
	/// @todo Static overload methods
	/// @todo Cast operators
	template <typename F>
	void registerMethod(F f, const util::Str8& name){
		registerMethod<typename util::FunctionTraits<F>::Class>(f, name);
	}
	
	template <typename C, typename F>
	void registerMethod(F f, const util::Str8& name);

	/// @todo Rethink
	/// Registers template method as series of normal methods to script
	/// Example: 	registerTemplateMethod<ObjectNodeSerializableTraits>(&Class::method, "method")
	///				registers methods "Class::method" + typename for every type that supports util::ObjectNode
	///				serialization and is registered to script. If new util::ObjectNode -serializable type is
	///				registered afterwards, then new template method is registered
	/// @param C Class
	/// @param F Method
	/// @param T Template traits type
	/*template <template <typename> class T = AlwaysTrue, template <typename> class F>
	void registerTemplateMethod(F f, const util::Str8& base_name){
		registerTemplateMethod<T, typename FunctionTraits<F>::ClassType, F>(f, base_name);
	}
	
	template <template <typename> class T, typename C, template <typename> class F>
	void registerTemplateMethod(F f, const util::Str8& base_name);*/
	
	/// Allows casting, doesn't register inherited methods
	template <typename B, typename D>
	void registerInheriting();
	
	template <typename E>
	void registerEnumType();
	
	template <typename E>
	void registerEnumValue(const util::Str8& name, int32 value);
	
	/// Registers function signature to be used as a funcdef in script
	/// E.g. void (int) -> funcdef void f_void_int(int)
	template <typename Signature>
	void registerFuncdef();
	
	/// @return Currently executing script context
	Context* getCurrentContext() const { return executingContexts.empty() ? nullptr : executingContexts.top(); }
	
	/// @return Free context from a context pool
	Context& getFreeContext();
	
	asIScriptEngine& getAsEngine() const;
protected:
	friend class Context;
	
	void pushExecutingContext(Context& c){ executingContexts.push(&c); }
	void popExecutingContext(){ executingContexts.pop(); }
	
private:
	
	/// Returns name/type with namespace specifiers removed
	/// Doesn't remove namespaces from template parameters
	util::Str8 namespacesRemoved(const util::Str8& name) const;
	
	/// E.g: "::bool" -> "Bool"
	util::Str8 asTemplatePostfix(const util::Str8& name) const;
	
	/// Sets active namespace from namespace specifiers of name
	void setCurrentNamespaceFromName(const util::Str8& name);
	
	/// Resets current namespace to global
	void resetCurrentNamespace();
	
	/// @todo Rethink
	/*struct TraitsDummyParameter {}; /// Used to convert traits type to data (typeid(Traits<TraitsDummyParameter>).name)
	
	/// Unique (but not the same on different platforms/compilers) id for single parameter template traits class
	template <template <typename> class T>
	util::Str8 traitsId(){ return typeid(T<TraitsDummyParameter>).name; } 
	*/
	
	template <typename T>
	struct IsArray { static constexpr bool value = false; };
	
	template <typename T>
	struct IsArray<util::DynArray<T>> { static constexpr bool value = true; };
	
	template <typename T>
	struct IsArray<util::DynArray<T>*> { static constexpr bool value = true; };
	
	#define DECLARE_REGISTER_METHOD(method_name, condition) \
	template <typename PlainT> \
	void register ## method_name(); \
	template <typename PlainT, typename OrigT, typename B = typename std::enable_if<condition>::type> \
	void tryRegister ## method_name(){ register ## method_name<PlainT>(); } \
	template <typename PlainT, typename OrigT, typename B = typename std::enable_if<!condition>::type, typename = void> \
	void tryRegister ## method_name(){}
	
	DECLARE_REGISTER_METHOD(ConstructorBeh, 	(	std::is_default_constructible<PlainT>::value))
	DECLARE_REGISTER_METHOD(CopyConstructorBeh, (	std::is_copy_constructible<PlainT>::value))
	DECLARE_REGISTER_METHOD(DestructorBeh, 		(	std::is_destructible<PlainT>::value))
	DECLARE_REGISTER_METHOD(AssignmentOperator, (	std::is_copy_assignable<PlainT>::value))
	DECLARE_REGISTER_METHOD(EqualsOperator, 	(	util::HasEqualsOperator<PlainT>::value))
	DECLARE_REGISTER_METHOD(ReferenceBeh, 		(	util::IsBaseOf<script::Reference, PlainT>::value))
	DECLARE_REGISTER_METHOD(NoCountReference, 	(	!std::is_abstract<PlainT>::value && 
													std::is_default_constructible<PlainT>::value &&
													(util::IsBaseOf<NoCountReference, PlainT>::value || std::is_pointer<OrigT>::value)))
	DECLARE_REGISTER_METHOD(ObjectNodeSupport, 	(	util::IsObjectNodeSerializable<PlainT>::value &&
													!std::is_same<PlainT, util::ObjectNode>::value &&
													!IsArray<PlainT>::value))
	
	DECLARE_REGISTER_METHOD(ArraySpecialization,
		(!std::is_abstract<PlainT>::value && (std::is_copy_assignable<PlainT>::value || std::is_pointer<PlainT>::value) && !IsArray<PlainT>::value))
	DECLARE_REGISTER_METHOD(ArraySpecializationEqualityDependent, (util::HasEqualsOperator<PlainT>::value))
	#undef DECLARE_REGISTER_METHOD
	
	void registerEssentials();
	
	asIScriptEngine* engine;
	
	util::Stack<Context*> executingContexts;
	util::LinkedList<Context> contextPool;
	
	/// To prevent double registration
	/// Doesn't contain everything yet so don't expose outside class
	/// <ObjectName, util::DynArray<PropertyName>>
	util::HashMap<util::Str8, util::DynArray<util::Str8>> registeredStrings;
	
	/// @todo Rethink
	/*
	/// Can be used to find types that fulfill a condition Traits<Type>::value == true
	/// Used when registering template function to script as normal functions and allowed
	/// types are util::limited by some traits class
	/// <traitsId<Traits>(), util::DynArray<TypeName>>
	util::HashMap<util::Str8, util::DynArray<util::Str8>> typesByTraitsSupport;

	/// Wraps information that is needed to register method as data
	struct TemplateFunctionInfo {
		util::Str8 className;
		util::Str8 functionName;
		util::Str8 paramList;
	};
	
	using TemplateFunctionRegisterer = std::function<void (const TemplateFunctionInfo&)>;
	
	/// Used to add support for new types for template functions in script, when types are
	/// registered after the method has been registered
	/// <traitsId<Traits>(), util::DynArray<TemplateFunctionRegisterer>>
	util::HashMap<util::Str8, util::DynArray<TemplateFunctionRegisterer>> templateFunctionRegisterersByTraitsSupport;
	*/
};

struct TemplatePlaceholder {};
struct TemplateClassPlaceholder {};

/// ScriptMgr sets in constructor
extern ScriptMgr* gScriptMgr;

#include "script_mgr.tpp"

} // script
namespace util {

template <>
struct TypeStringTraits<script::TemplatePlaceholder>{
	static util::Str8 type(){ return "T"; }
};

template <>
struct TypeStringTraits<script::TemplateClassPlaceholder>{
	static util::Str8 type(){ return "class T"; }
};

} // util
} // clover

#endif // CLOVER_SCRIPT_SCRIPT_MGR_HPP