#ifndef CLOVER_GAME_WORLDENTITY_MACROS_HPP
#define CLOVER_GAME_WORLDENTITY_MACROS_HPP

#include "build.hpp"

namespace clover {
namespace game {

/// @todo Remove whole legacy junk file

class WEInfo;

#define DECLARE_WE(typename_, basename_)		\
protected:										\
	typedef basename_ ## WE BaseClass;			\
	typedef typename_ ## WE ThisClass;			\
	virtual void onCreate(const WEInfo& info);	\
	void create();								\
	void destroy();								\
public:											\
	static WEInfo generateWEInfo();				\
	typename_ ## WE();							\
	virtual ~typename_ ## WE();					\
	void initDataDesc();						\
	
	
	
#define IMPLEMENT_WE(typename_)							\
	typename_ ## WE::typename_ ## WE(){					\
		weType= typename_;								\
	}													\
	typename_ ## WE::~typename_ ## WE(){				\
		destroy();										\
	}													\
	void typename_ ## WE::onCreate(const WEInfo& i){	\
		BaseClass::onCreate(i);							\
		ensure(info);									\
		initDataDesc();									\
		create();										\
	}


#define BEGIN_WE_INFO(typename_)								\
	WEInfo typename_##WE::generateWEInfo(){						\
		WEInfo info= BaseClass::generateWEInfo();

#define DEF_WE_DEFAULT_ICON(model)								\
	info.icon.defaultModel= &global::g_env.resCache->getResource<visual::Model>(model);

#define DEF_WE_PICKABLE(is_pickable)							\
	info.pickable= is_pickable;

#define END_WE_INFO()											\
	return info; }


#define BEGIN_WE_DATADESC(typename_)			\
	void typename_##WE::initDataDesc(){			\
		
#define DEF_WE_SAVE_FIELD_DYNARRAY( array_name, array_max_size )		\
		saveFields.add(array_name, util::Str8(#array_name), array_max_size);
		
#define DEF_WE_SAVE_FIELD( variable_name )					\
		saveFields.add(variable_name, util::Str8(#variable_name));

#define DEF_WE_SET_FUNC( variable_name, method )			\
		saveFields.getFieldByName( util::Str8(#variable_name) )->	\
			setChangeFunc<decltype(variable_name)>(boost::bind(&ThisClass::method, this, _1));

#define DEF_WE_STRICT_DEPENDENCY_DYNARRAY( handle, array_max_size )	\
		for (auto &m : handle) m.setStrict(*this);					\
		saveFields.add( handle,										\
						util::Str8(#handle), array_max_size);		
		
			
			
#define DEF_WE_STRICT_DEPENDENCY( handle )							\
		handle.setStrict(*this);									\
		saveFields.add( handle,										\
						util::Str8(#handle));
			
#define DEF_WE_LOOSE_DEPENDENCY_DYNARRAY( handle, array_max_size )	\
		saveFields.add( handle,										\
						util::Str8(#handle), array_max_size);				\
		
			
#define DEF_WE_LOOSE_DEPENDENCY( handle )							\
		saveFields.add( handle,										\
						util::Str8(#handle));								\


#define DEF_WE_SAVE_FIELD_SETM( variable_name, set_method )			\
		DEF_WE_SAVE_FIELD( variable_name ) DEF_WE_SET_FUNC( variable_name, set_method )

#define DEF_WE_SAVE_FIELD_DYNARRAY_SETM( array_name, array_size, set_method )		\
		DEF_WE_SAVE_FIELD_DYNARRAY( array_name, array_size ) DEF_WE_SET_FUNC( array_name, set_method )



#define END_WE_DATADESC() }

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_MACROS_HPP