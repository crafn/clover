#include "fixture_rigid.hpp"
#include "util/vector.hpp"
#include "physics/object_rigid.hpp"
#include "physics/material.hpp"
#include "resources/cache.hpp"
// For setMaterial shortcut
#include "game/physicalmaterial.hpp"

namespace clover {
namespace physics {

RigidFixtureDef::RigidFixtureDef(real32 d, real32 f, real32 r)
		: shape(nullptr){
			
	contactFilter.setB2Filter(box2dDef.filter);

	box2dDef.density= d;
	box2dDef.friction= f;
	box2dDef.restitution= r;
}

RigidFixtureDef::RigidFixtureDef(const physics::Material& mat){
	contactFilter.setB2Filter(box2dDef.filter);
	
	setMaterial(mat);
}

RigidFixtureDef::RigidFixtureDef(const RigidFixtureDef& other):
	FixtureDef(other),
	box2dDef(other.box2dDef),
	shape(other.shape){

	contactFilter.setB2Filter(box2dDef.filter);
}

RigidFixtureDef::RigidFixtureDef(RigidFixtureDef&& other):
	FixtureDef(std::move(other)),
	box2dDef(other.box2dDef),
	shape(other.shape){

	contactFilter.setB2Filter(box2dDef.filter);

	other.shape= 0;

}

RigidFixtureDef::~RigidFixtureDef(){
}

void RigidFixtureDef::onEvent(global::Event& e){
	switch(e.getType()){
		
		case global::Event::OnPhysMaterialChange: {
			
			Material* mat= e(global::Event::Material).getPtr<Material>();
			
			ensure(mat == getMaterial());
			setDensity(mat->getDensity());
			setFriction(mat->getFriction());
			setRestitution(mat->getRestitution());

		}
		break;
		
		default: break;
	}
}

void RigidFixtureDef::setShape(const collision::Shape& shape_){
	shape= resources::ResourceRef<collision::Shape>(&shape_);
}

void RigidFixtureDef::setShape(resources::ResourceRef<collision::Shape> shape_){
	shape= shape_;
}

void RigidFixtureDef::setShape(const util::Str8& name){
	shape= resources::ResourceRef<collision::Shape>(name);
}

void RigidFixtureDef::setDensity(real32 f){
	box2dDef.density= f;
}


void RigidFixtureDef::setRestitution(real32 f){
	box2dDef.restitution= f;
}


void RigidFixtureDef::setFriction(real32 f){
	box2dDef.friction= f;
}

void RigidFixtureDef::setMaterial(const physics::Material& mat){
	FixtureDef::setMaterial(mat);

	setDensity(mat.getDensity());
	setFriction(mat.getFriction());
	setRestitution(mat.getRestitution());

}

void RigidFixtureDef::setMaterial(const util::Str8& name){
	setMaterial(global::g_env->resCache->getResource<game::PhysicalMaterial>(name));
}

void RigidFixtureDef::setAsSensor(bool b){
	box2dDef.isSensor= b;
}

bool RigidFixtureDef::isSensor() const {
	return box2dDef.isSensor;
}

//
// RigidFixture
//

RigidFixture::RigidFixture(RigidObject& owner_, RigidObject& object_, const RigidFixtureDef& def_)
		: Fixture(def_)
		, owner(&owner_)
		, object(&object_)
		, def(def_){
	addToTraceableTypeTree(collision::Traceable::TypeId::RigidFixture);
			
	ensure(owner);
	ensure(owner->getB2Body());

	recreateB2Fixtures();

	userData.owner= this;
	refreshListeners();
}

RigidFixture::~RigidFixture(){
	destroyB2Fixtures();
	userData.owner= nullptr;
}

RigidFixture::Transform RigidFixture::getTransform() const {
	return object->getTransform();
}

bool RigidFixture::overlaps(const util::Vec2d& point) const {
	if (!def.getShape())
		return false;

	return def.getShape()->overlaps(point, getTransform());
}

bool RigidFixture::overlaps(const Traceable& other) const {
	if (!def.getShape())
		return false;

	/// @todo Better handling for other types of collision::Traceable :p
	const RigidFixture* other_fix= dynamic_cast<const RigidFixture*>(&other);
	if (!other_fix || !other_fix->getShape()) return false;

	return other_fix->getShape()->overlaps(*def.getShape(), getTransform(), other.getTransform());
}

bool RigidFixture::overlaps(const collision::Ray& ray, collision::RayCastResult* output) const {
	if (!def.getShape())
		return false;

	return def.getShape()->overlaps(ray, getTransform(), output);
}

bool RigidFixture::overlaps(const collision::BaseShape& shp, const Transform& shp_transform) const {
	if (!def.getShape())
		return false;
	
	for (const auto& baseshape : def.getShape()->getBaseShapes()){
		if (shp.overlaps(*baseshape, getTransform(), shp_transform))
			return true;
	}
	
	return false;
}

bool RigidFixture::isSensor() const {
	return def.isSensor();
}

void RigidFixture::setShape(resources::ResourceRef<collision::Shape> shape){
	def.setShape(shape);
	recreateB2Fixtures();
	refreshListeners();
}

void RigidFixture::changeOwner(RigidObject& new_owner){
	if (&new_owner == owner)
		return;

	destroyB2Fixtures();
	owner= &new_owner;
	recreateB2Fixtures();
}

void RigidFixture::recreateB2Fixtures(){
	destroyB2Fixtures();
	
	if (!owner || !def.getShape())
		return;

	const collision::Shape* shape= def.getShape();
	util::UniquePtr<collision::Shape> modified_shape;
	if (object->isMirrored() || object != owner){
		modified_shape= util::makeUniquePtr<collision::Shape>(*def.getShape());
		auto t= object->getTransform()*owner->getTransform().inversed();

		if (object->isMirrored()){
			modified_shape->mirror();
		}
		if (object != owner){
			// 'object' doesn't own the b2 body, so shape needs to be transformed
			modified_shape->transform(t);
		}
		shape= modified_shape.get();
	}

	const auto& base_shapes= shape->getBaseShapes();
	for (const auto& bshape : base_shapes){
		for (SizeType i= 0; i < bshape->getB2ShapeCount(); ++i){
			b2FixtureDef b2_def= def.getB2Def();
			b2_def.shape= &bshape->getB2Shape(i);

			ensure(owner->getB2Body());
			ensure(b2_def.shape);
			box2dFixtures.pushBack(owner->getB2Body()->CreateFixture(&b2_def));
			box2dFixtures.back()->SetUserData(&userData);
		}
	}
}

void RigidFixture::destroyB2Fixtures(){
	if (owner){
		auto b2_body= owner->getB2Body();

		if (b2_body)
		{
			for (auto& m : box2dFixtures){

				ensure(b2_body);
				b2_body->DestroyFixture(m);
			}

		}
	}
	
	box2dFixtures.clear();
}


void RigidFixture::refreshListeners(){
	shapeChangeListener.clear();
	if (def.getShape()){
		shapeChangeListener.listen(*def.getShape(), [this]{
			recreateB2Fixtures();
		});
	}
	
	materialChangeListener.clear();
	if (def.getMaterial()){
		debug_ensure(dynamic_cast<const game::PhysicalMaterial*>(def.getMaterial()));
		const game::PhysicalMaterial* mat= static_cast<const game::PhysicalMaterial*>(def.getMaterial());
		
		materialChangeListener.listen(*mat, [this]{
			// Update box2d fixture materials
			for (auto& m : box2dFixtures){
				m->SetDensity(def.getMaterial()->getDensity());
				m->SetFriction(def.getMaterial()->getFriction());
				m->SetRestitution(def.getMaterial()->getRestitution());
				
				ensure(owner);
				if (owner->getB2Body())
					owner->getB2Body()->ResetMassData();
			}
		});
	}
}

} // physics
} // clover