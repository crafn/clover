#include "animation/script.hpp"
#include "script_mgr.hpp"
#include "debug/debugprint.hpp"
#include "exception.hpp"
#include "game/script.hpp"
#include "game/world_gen/script.hpp"
#include "global/cfg_mgr.hpp"
#include "global/cvar.hpp"
#include "global/event.hpp"
#include "nodes/script.hpp"
#include "nodes/signaltypetraits.hpp"
#include "physics/script.hpp"
#include "resources/script.hpp"
#include "script/reference.hpp"
#include "ui/script.hpp"
#include "util/cb_listener.hpp"
#include "util/shared_ptr.hpp"
#include "util/unique_ptr.hpp"
#include "util.hpp"
#include "util/ensure.hpp"
#include "util/string.hpp"
#include "util/time.hpp"

#include <scriptmath/scriptmath.h>
#include <scriptarray/scriptarray.h>

namespace clover {
namespace script {

ScriptMgr* gScriptMgr= nullptr;

void scriptEnsure(bool x){
	release_ensure_msg(x, "Script ensure");
}

real64 getDeltaTime(){
	return util::gGameClock->getDeltaTime();
}

void printStr8(const util::Str8& str){
	print(debug::Ch::Script, debug::Vb::Trivial, str.cStr());
}

void printCVar(global::CVar& cvar){ // Can't be const CVar&, because then Angelscript requires default constructor o_O
	printStr8(cvar.generateString());
}

void criticalPrintStr8(const util::Str8& str){
	print(debug::Ch::Script, debug::Vb::Critical, str.cStr());
}

void criticalPrintCVar(global::CVar& cvar){ // Can't be const CVar&, because then Angelscript requires default constructor o_O
	criticalPrintStr8(cvar.generateString());
}

util::Str8 stringFactory(unsigned int length, const char* str){
	return (util::Str8(std::string(str, length)));
}

real64 min(const real64& a, const real64& b){
	return std::min(a,b);
}

real64 max(const real64& a, const real64& b){
	return std::max(a,b);
}


int64 randInt(int64 min, int64 max){
	return util::Rand::discrete(min, max);
}

real64 randReal(real64 min, real64 max){
	return util::Rand::continuous(min, max);
}

void messageCallback(const asSMessageInfo* msg, void* param){
	debug::Vb vb= debug::Vb::Trivial;
	if (msg->type == asMSGTYPE_ERROR)
		vb= debug::Vb::Critical;
	else if (msg->type == asMSGTYPE_WARNING)
		vb= debug::Vb::Moderate;
		
	print(debug::Ch::Script, vb, "AngelScript message:\n	file %s, row %i\n	 %s",
		msg->section, msg->row, msg->message);
}

void stringRealConstruct(const real64& f, void* memory){
	new (memory) util::Str8(util::Str8::format("%f", f));
}

void stringIntConstruct(const int64& i, void* memory){
	/// @todo lld not supported in windows
	new (memory) util::Str8(util::Str8::format("%ld", (long)i));
}

void stringUIntConstruct(const uint64& i, void* memory){
	/// @todo llu not supported in windows
	new (memory) util::Str8(util::Str8::format("%lu", (unsigned long)i));
}

void constructVec2d(real64 x, real64 y, void* memory){
	new (memory) util::Vec2d({x, y});
}

void constructVec2d2(real64 a, void* memory){
	new (memory) util::Vec2d(a);
}

void constructVec2f(real32 x, real32 y, void* memory){
	new (memory) util::Vec2f({x, y});
}

void constructVec2f2(real32 a, void* memory){
	new (memory) util::Vec2f(a);
}
void constructVec2i(int32 x, int32 y, void* memory){
	new (memory) util::Vec2i({x, y});
}

void constructVec3d(real64 x, real64 y, real64 z, void* memory){
	new (memory) util::Vec3d({x, y, z});
}

void constructVec3d2(real64 a, void* memory){
	new (memory) util::Vec3d(a);
}

void constructVec4d(real64 x, real64 y, real64 z, real64 w, void* memory){
	new (memory) util::Vec4d({x, y, z, w});
}

void constructVec4d2(real64 a, void* memory){
	new (memory) util::Vec4d(a);
}

void constructRtTransform2d(real64 rot, const util::Vec2d& pos, void* memory){
	new (memory) util::RtTransform2d(rot, pos);
}

void constructSrtTransform2d(	const util::SrtTransform2d::Scale& scale,
								const util::SrtTransform2d::Rotation& rot,
								const util::SrtTransform2d::Translation& pos,
								void* memory){
	new (memory) util::SrtTransform2d(scale, rot, pos);
}

void constructSrtTransform3d(	const util::SrtTransform3d::Scale& scale,
								const util::SrtTransform3d::Rotation& rot,
								const util::SrtTransform3d::Translation& pos,
								void* memory){
	new (memory) util::SrtTransform3d(scale, rot, pos);
}

template <typename T>
void registerCommonVectorMethods(){
	gScriptMgr->registerMethod<T>(&T::length, "length");
	gScriptMgr->registerMethod<T>(&T::lengthSqr, "lengthSqr");
	gScriptMgr->registerMethod<T>(&T::dot, "dot");
	gScriptMgr->registerMethod<T>(&T::operator+, "opAdd");
	gScriptMgr->registerMethod<T, T (T::*)(const T&) const>(&T::operator-, "opSub");
	gScriptMgr->registerMethod<T, T (T::*)() const>(&T::operator-, "opNeg");
	gScriptMgr->registerMethod<T>(&T::operator+=, "opAddAssign");
	gScriptMgr->registerMethod<T>(&T::operator-=, "opSubAssign");
	gScriptMgr->registerMethod<T, T& (T::*)(const T&)>(&T::operator*=, "opMulAssign");
	gScriptMgr->registerMethod<T, T& (T::*)(const typename T::Type&)>(&T::operator*=, "opMulAssign");
	gScriptMgr->registerMethod<T, T (T::*)(const typename T::Type&) const>(&T::operator*, "opMul");
	gScriptMgr->registerMethod<T, T (T::*)(const T&) const>(&T::operator*, "opMul");
	gScriptMgr->registerMethod<T>(&T::operator/, "opDiv");
	gScriptMgr->registerMethod<T>(&T::inversed, "inversed");
	gScriptMgr->registerMethod<T>(&T::normalized, "normalized");
	gScriptMgr->registerMethod<T>(&T::normal, "normal");
}

template <typename T>
void registerCommonTransformMethods(){
	gScriptMgr->registerMethod<T>(&T::inversed, "inversed");
	gScriptMgr->registerMethod<T, T (T::*)(const T&) const>(&T::operator*, "opMul");
}

ScriptMgr::ScriptMgr(){
	gScriptMgr= this;
	
	print(debug::Ch::Script, debug::Vb::Trivial, "AngelScript version: %s", ANGELSCRIPT_VERSION_STRING);
	
	engine= asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->AddRef();

	int32 ret=0;
	
	ret= engine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);
	Utils::devErrorCheck("", ret);
	
	engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, true);
	
	registerEssentials();
	
	global::Event e(global::Event::OnScriptMgrCreate);
	e(global::Event::Object)= this;
	e.send();
}

ScriptMgr::~ScriptMgr(){
	engine->Release();
	script::gScriptMgr= nullptr;
}

Context& ScriptMgr::getFreeContext(){
	for (auto& c : contextPool){
		if (c.isFree())
			return c;
	}
	print(debug::Ch::Script, debug::Vb::Trivial, "New script::Context created");
	contextPool.pushBack(Context());
	return contextPool.back();
}

asIScriptEngine& ScriptMgr::getAsEngine() const {
	debug_ensure(engine);
	return *engine;
}

util::Str8 ScriptMgr::namespacesRemoved(const util::Str8& name) const {
	util::Str8 processed= name;
	util::Str8 further_processed;

	while ((further_processed= Utils::leadingNamespaceRemoved(processed))
			!= processed)
		processed= further_processed;

	return processed; 
}

util::Str8 ScriptMgr::asTemplatePostfix(const util::Str8& name_) const {
	util::Str8 name= name_;
	name= name.splitted(':').back(); // Remove leading namespaces
	
	// Turn first character to upper case
	name.insert(0, name.upperCased()[0]);
	name.erase(1);
	
	return name;
}

void ScriptMgr::setCurrentNamespaceFromName(const util::Str8& name){
	SizeType length_without_namespace= namespacesRemoved(name).length();
	
	util::Str8 nspace= name;
	nspace.erase(name.length() - length_without_namespace, length_without_namespace);
	
	// Remove possible leading ::
	if (!nspace.empty() && nspace[0] == ':')
		nspace= Utils::leadingNamespaceRemoved(nspace);

	getAsEngine().SetDefaultNamespace(nspace.cStr());
}

void ScriptMgr::resetCurrentNamespace(){
	getAsEngine().SetDefaultNamespace("");
}

void ScriptMgr::registerEssentials(){
	RegisterScriptMath(engine);
	
	int32 ret;
	ret= engine->RegisterTypedef("real32", "float"); Utils::devErrorCheck("Register real32", ret);
	ret= engine->RegisterTypedef("real64", "double"); Utils::devErrorCheck("Register real64", ret);
	ret= engine->RegisterTypedef("SizeType", "uint64"); Utils::devErrorCheck("Register SizeType", ret);
	
	//
	// Array
	//
	ret= engine->RegisterObjectType("Array<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE);
	Utils::devErrorCheck("Register array", ret);

	ret= engine->RegisterObjectBehaviour("Array<T>", asBEHAVE_ADDREF, "void f()", asMETHOD(Reference,addRef), asCALL_THISCALL);
	Utils::devErrorCheck("Register array addref", ret);
	ret= engine->RegisterObjectBehaviour("Array<T>", asBEHAVE_RELEASE, "void f()", asMETHOD(Reference,release), asCALL_THISCALL);
	Utils::devErrorCheck("Register array release", ret);

	ret= engine->RegisterGlobalProperty("const real32 epsilonf", const_cast<real32*>(&util::epsilonf));
	ret= engine->RegisterGlobalProperty("const real64 epsilond", const_cast<real64*>(&util::epsilon));
	Utils::devErrorCheck("Register util::epsilon", ret);
	
	registerObjectType<util::ObjectNode>();
	registerObjectType<global::CVar>();
	
	registerTemplateType<util::Dynamic>();
	registerTemplateType<util::UniquePtr>();
	registerTemplateType<util::SharedPtr>();
	registerTemplateType<util::CbListener>();

	registerObjectNodeSupport<bool>();
	registerObjectNodeSupport<int32>();
	registerObjectNodeSupport<int64>();
	registerObjectNodeSupport<real32>();
	registerObjectNodeSupport<real64>();
	
	registerObjectType<util::Str8>();

	ret= engine->RegisterStringFactory("Str8", asFUNCTION(stringFactory), asCALL_CDECL);
	Utils::devErrorCheck("Register string factory", ret);
	
	registerConstructor<util::Str8>(stringRealConstruct);
	registerConstructor<util::Str8>(stringIntConstruct);
	registerConstructor<util::Str8>(stringUIntConstruct);
	registerMethod<util::Str8>(static_cast<util::Str8 (util::Str8::*)(const util::Str8&) const>(&util::Str8::operator+), "opAdd");
	
	registerGlobalFunction(printStr8, "print");
	registerGlobalFunction(printCVar, "print");
	registerGlobalFunction(criticalPrintStr8, "criticalPrint");
	registerGlobalFunction(criticalPrintCVar, "criticalPrint");

	registerGlobalFunction(min, "min");
	registerGlobalFunction(max, "max");
	registerGlobalFunction(util::limited<real64>, "limited");
	registerGlobalFunction(util::limited<int64>, "limited");
	registerGlobalFunction(getDeltaTime, "getDeltaTime");
	registerGlobalFunction(randInt, "randInt");
	registerGlobalFunction(randReal, "randReal");
	registerGlobalProperty(util::pi, "pi");
	registerGlobalProperty(util::tau, "tau");
	registerGlobalProperty(util::euler, "euler");
	registerGlobalFunction(scriptEnsure, "ensure");
	
	registerFuncdef<void()>();

	registerObjectType<global::CfgMgr>();
	registerMethod(&global::CfgMgr::load, "load");
	registerMethod(&global::CfgMgr::save, "save");
	registerGlobalProperty(global::gCfgMgr, "gCfgMgr");
	
	typedef util::Vec2d Vec2Type;
	registerObjectType<Vec2Type>();
	registerConstructor<Vec2Type>(constructVec2d);
	registerConstructor<Vec2Type>(constructVec2d2);
	registerCommonVectorMethods<Vec2Type>();
	registerMethod<Vec2Type>(&Vec2Type::rotationZ, "rotationZ");
	registerMethod<Vec2Type>(&Vec2Type::crossZ, "crossZ");
	registerMember<Vec2Type>(&Vec2Type::x, "x");
	registerMember<Vec2Type>(&Vec2Type::y, "y");
	
	registerObjectType<util::Vec2f>();
	registerConstructor<util::Vec2f>(constructVec2f);
	registerConstructor<util::Vec2f>(constructVec2f2);
	registerCommonVectorMethods<util::Vec2f>();
	registerMethod<util::Vec2f>(&util::Vec2f::rotationZ, "rotationZ");
	registerMethod<util::Vec2f>(&util::Vec2f::crossZ, "crossZ");
	registerMember<util::Vec2f>(&util::Vec2f::x, "x");
	registerMember<util::Vec2f>(&util::Vec2f::y, "y");

	registerObjectType<util::Vec2i>();
	registerConstructor<util::Vec2i>(constructVec2i);
	registerMethod<util::Vec2i>(&util::Vec2i::operator+, "opAdd");
	registerMethod<util::Vec2i>(static_cast<util::Vec2i (util::Vec2i::*)(const util::Vec2i&) const>(&util::Vec2i::operator-), "opSub");
	registerMethod<util::Vec2i>(&util::Vec2i::operator+=, "opAddAssign");
	registerMethod<util::Vec2i>(&util::Vec2i::operator-=, "opSubAssign");
	registerMember<util::Vec2i>(&util::Vec2i::x, "x");
	registerMember<util::Vec2i>(&util::Vec2i::y, "y");
	
	typedef util::Vec3d Vec3Type;
	registerObjectType<Vec3Type>();
	registerConstructor<Vec3Type>(constructVec3d);
	registerConstructor<Vec3Type>(constructVec3d2);
	registerCommonVectorMethods<Vec3Type>();
	registerMember<Vec3Type>(&Vec3Type::x, "x");
	registerMember<Vec3Type>(&Vec3Type::y, "y");
	registerMember<Vec3Type>(&Vec3Type::z, "z");
	
	typedef util::Vec4d Vec4Type;
	registerObjectType<Vec4Type>();
	registerConstructor<Vec4Type>(constructVec4d);
	registerConstructor<Vec4Type>(constructVec4d2);
	registerCommonVectorMethods<Vec4Type>();
	registerMember<Vec4Type>(&Vec4Type::x, "x");
	registerMember<Vec4Type>(&Vec4Type::y, "y");
	registerMember<Vec4Type>(&Vec4Type::z, "z");
	registerMember<Vec4Type>(&Vec4Type::w, "w");
	
	typedef util::Quatd QuatType;
	registerObjectType<QuatType>();
	registerMethod<QuatType>(&QuatType::axis, "axis");
	registerMethod<QuatType>(&QuatType::rotation, "rotation");
	registerMethod<QuatType>(&QuatType::rotationZ, "rotationZ");
	registerMethod<QuatType>(&QuatType::normalized, "normalized");
	registerMethod<QuatType>(static_cast<QuatType (QuatType::*)(const QuatType&) const>(&QuatType::operator*), "opMultiply");
	registerMethod<QuatType>(static_cast<QuatType& (QuatType::*)(const QuatType&)>(&QuatType::operator*=), "opMultiplyAssign");
	registerMember<QuatType>(&QuatType::x, "x");
	registerMember<QuatType>(&QuatType::y, "y");
	registerMember<QuatType>(&QuatType::z, "z");
	registerMember<QuatType>(&QuatType::w, "w");
	registerGlobalFunction(QuatType::byRotationAxis, "QuatdByRotationAxis");
	registerGlobalFunction(QuatType::byRotationFromTo, "QuatdByRotationFromTo");

	typedef util::RtTransform2d RtTransform2Type;
	registerObjectType<RtTransform2Type>();
	registerConstructor<RtTransform2Type>(constructRtTransform2d);
	registerMember<RtTransform2Type>(&RtTransform2Type::translation, "translation");
	registerMember<RtTransform2Type>(&RtTransform2Type::rotation, "rotation");
	registerCommonTransformMethods<RtTransform2Type>();
	
	typedef util::SrtTransform2d SrtTransform2Type;
	registerObjectType<SrtTransform2Type>();
	registerConstructor<SrtTransform2Type>(constructSrtTransform2d);
	registerMember<SrtTransform2Type>(&SrtTransform2Type::translation, "translation");
	registerMember<SrtTransform2Type>(&SrtTransform2Type::rotation, "rotation");
	registerMember<SrtTransform2Type>(&SrtTransform2Type::scale, "scale");
	registerCommonTransformMethods<SrtTransform2Type>();

	typedef util::SrtTransform3d SrtTransform3Type;
	registerObjectType<SrtTransform3Type>();
	registerConstructor<SrtTransform3Type>(constructSrtTransform3d);
	registerMember<SrtTransform3Type>(&SrtTransform3Type::translation, "translation");
	registerMember<SrtTransform3Type>(&SrtTransform3Type::rotation, "rotation");
	registerMember<SrtTransform3Type>(&SrtTransform3Type::scale, "scale");
	registerCommonTransformMethods<SrtTransform3Type>();

	util::Coord::registerToScript();

	animation::registerToScript();
	physics::registerToScript();
	game::registerToScript();
	nodes::registerToScript();
	resources::registerToScript();
	game::world_gen::registerToScript();
	ui::registerToScript();	

	//static_assert(std::is_copy_assignable<std::vector<std::unique_ptr<int>>>::value == false, "std::is_copy_assignable broken");
}

} // script
} // clover
