#include "material.hpp"
#include "global/event.hpp"

namespace clover {
namespace physics {

Material::Material():
	density(1.0), 
	friction(0.5),
	restitution(0.5),
	toughness(-1.0){
}

Material::Material(real32 d, real32 f, real32 r, real32 t):
	density(d),
	friction(f),
	restitution(r),
	toughness(t){

}

void Material::setDensity(real32 d){
	density= d;
	onChange();
}

void Material::setFriction(real32 f){
	friction= f;
	onChange();
}

void Material::setRestitution(real32 r){
	restitution= r;
	onChange();
}

void Material::setToughness(real32 t){
	toughness= t;
	onChange();
}

void Material::onChange(){
	if (getReferenceCount() > 0){
		global::Event e(global::Event::OnPhysMaterialChange);
		e(global::Event::Material)= this;
		e.send();
	}
}

} // physics
} // clover