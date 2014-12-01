#include "debug/debugprint.hpp"
#include "object_rigid.hpp"
#include "physics/fixture_rigid.hpp"
#include "physics/joint.hpp"
#include "util/dyn_array.hpp"
#include "util/polygon.hpp"
#include "util/profiling.hpp"
#include "util/misc.hpp"
#include "world.hpp"

namespace clover {
namespace physics {

RigidObjectDef::RigidObjectDef(util::Vec2d pos, real32 rot, util::Vec2d vel, real32 angularvel){
	def.type= b2_dynamicBody;
	def.position= pos.b2();
	def.angle= rot;
	def.linearVelocity= vel.b2();
	def.angularVelocity= angularvel;
	def.fixedRotation= false;
}

void RigidObjectDef::setTransform(Object::Transform t){
	setPosition(t.translation);
	setRotation(t.rotation);
}

void RigidObjectDef::setPosition(util::Vec2d pos){
	def.position= pos.b2();
}

void RigidObjectDef::setRotation(real64 rot){
	def.angle= rot;
}

void RigidObjectDef::setType(RigidObjectType t){
	switch(t){
		case RigidObjectType::Dynamic:
			def.type= b2_dynamicBody;
		break;
		case RigidObjectType::Static:
			def.type= b2_staticBody;
		break;
		case RigidObjectType::Kinematic:
			def.type= b2_kinematicBody;
		break;
	}
}

void RigidObjectDef::setStatic(bool s){
	if (s)
		def.type= b2_staticBody;
	else
		def.type= b2_dynamicBody;
}

bool RigidObjectDef::isStatic() const {
	return def.type == b2_staticBody;
}

void RigidObjectDef::setFixedRotation(bool s){
	def.fixedRotation= s;
}


util::LinkedList<RigidObject*> RigidObject::nonStaticRigidObjects;

///
/// RigidObject
///

RigidObject::RigidObject(RigidObjectDef bdef):
			bodyDef(bdef),
			body(nullptr){
	nsSelfIter= nonStaticRigidObjects.end();

	typeId= TypeId::Rigid;

	bodyData.owner= this;
	bodyDef.def.userData= &bodyData;

	createB2Body();
	addToGrid();
}

RigidObject::~RigidObject(){
	removeFromGrid();
	detachJoints();

	ensure(isNormal());

	if (body)
		destroyB2Body();

	body= nullptr;
}

RigidFixture& RigidObject::add(const RigidFixtureDef& fix){
	removeFromGrid();
	fixtureDefs.pushBack(util::makeUniquePtr<RigidFixtureDef>(fix));
	createFixture(fix);
	addToGrid();
	return *fixtures.back();
}

void RigidObject::set(const RigidObjectDef& def){
	release_ensure_msg(!isProxy() && !isProxyMaster(), "@todo proxy impl");
	removeFromGrid();

	bodyDef= def;
	bodyDef.def.userData= &bodyData;

	if (bodyDef.def.active){
		if (body)
			destroyB2Body();
		createB2Body();
	}
	addToGrid();
}

void RigidObject::setActive(bool a){
	if (a == bodyDef.def.active)
		return;

	removeFromGrid();
	bodyDef.def.active= a;

	if (isNormal()){
		getB2Body()->SetActive(a);
	} else {
		bool body_active= false;

		// Check if someone in weld group is active
		for (auto& obj : getWeldGroupObjects()){
			if (obj->isActive()){
				body_active= true;
				break;
			}
		}
		getB2Body()->SetActive(body_active);
	}
	addToGrid();
}

void RigidObject::setStatic(bool s){
	removeFromGrid();
	/// @todo Proxy impl
	if (body){
		if (s)
			body->SetType(b2_staticBody);
		else
			body->SetType(b2_dynamicBody);
	}
	bodyDef.setStatic(s);

	onStaticnessChange();
	addToGrid();
}

void RigidObject::clear(){
	ensure_msg(getJoints().empty(), "todo clear with joints");
	removeFromGrid();

	fixtureDefs.clear();
	fixtures.clear();

	if (body)
		destroyB2Body();

	body= nullptr;
}

void RigidObject::clearFixtures(){
	removeFromGrid();

	fixtureDefs.clear();
	fixtures.clear();
}

util::Vec2d RigidObject::getCenterOfMass() const {
	if (isProxy())
		return getPosition() + proxyData->positionToCm;

	if (isProxyMaster())
		return getPosition() + proxyMasterData->positionToCm;

	if (!body)
		return getPosition();
	return body->GetWorldCenter();
}

real64 RigidObject::getMass() const {
	if (isProxy())
		return proxyData->mass;

	if (isProxyMaster())
		return proxyMasterData->mass;

	if (!body)
		return 1.0f;

	/// @fixme	Return correct value for static/kinematic bodies
	///			They shouldn't have zero mass!
	///			Staticness should be thought as a constraint which
	///			prevents all changes in velocity, not as an infinite mass
	return body->GetMass();
}

real64 RigidObject::getEffInvMass() const {
	ensure(getB2Body());
	return getB2Body()->GetMass();
}

real64 RigidObject::getInertia() const {
	if (isProxy())
		return proxyData->inertia;

	if (isProxyMaster())
		return proxyMasterData->inertia;

	if (!body)
		return 1.0;
	else if (body->IsFixedRotation())
		return 0.0;
	else
		return body->GetInertia();
}

void RigidObject::applyForce(util::Vec2d force, util::Vec2d point){
	if (bodyDef.def.type != b2_dynamicBody)
		return;

	if (getB2Body())
		getB2Body()->ApplyForce(force.b2(), point.b2(), true);
}

void RigidObject::applyForce(util::Vec2d force){
	if (bodyDef.def.type != b2_dynamicBody)
		return;

	if (getB2Body())
		getB2Body()->ApplyForceToCenter(force.b2(), true);
}

void RigidObject::applyImpulse(util::Vec2d impulse, util::Vec2d point){
	if (bodyDef.def.type != b2_dynamicBody)
		return;

	if (getB2Body())
		getB2Body()->ApplyLinearImpulse(impulse.b2(), point.b2(), true);
}

void RigidObject::applyImpulse(util::Vec2d impulse){
	if (bodyDef.def.type != b2_dynamicBody)
		return;

	if (getB2Body())
		getB2Body()->ApplyLinearImpulse(impulse.b2(), getB2Body()->GetWorldCenter(), true);
}

void RigidObject::applyTorque(real64 momentum){
	if (bodyDef.def.type != b2_dynamicBody)
		return;

	if (getB2Body())
		getB2Body()->ApplyTorque(momentum, true);
}

void RigidObject::applyAngularImpulse(real64 impulse){
	if (bodyDef.def.type != b2_dynamicBody)
		return;

	if (getB2Body())
		getB2Body()->ApplyAngularImpulse(impulse, true);
}

void RigidObject::setVelocity(const util::Vec2d& vel){
	if (getB2Body()){
		getB2Body()->SetLinearVelocity(vel.b2());
		overrideCachedValuesByB2();
	}
}

util::Vec2d RigidObject::getVelocity(util::Vec2d worldpoint) const {
	if (!getB2Body())
		return util::Vec2d(0);

	return getB2Body()->GetLinearVelocityFromWorldPoint(worldpoint.b2());
}

void RigidObject::setAngularVelocity(real64 omega){
	/// @todo Proxy impl
	if (body){
		body->SetAngularVelocity(omega);
		overrideCachedValuesByB2();
	}
}

void RigidObject::setTransform(const Transform& t){
	removeFromGrid();
	if (isProxy()){
		auto offset_t= proxyData->master->getTransform()*
			getTransform().inversed();
		proxyData->master->setTransform(offset_t*t);
	}
	else if (body){
		body->SetTransform(t.translation.b2(), t.rotation);
		body->SetAwake(true);
		overrideCachedValuesByB2();
	}
	addToGrid();
}

void RigidObject::setPosition(util::Vec2d p){
	removeFromGrid();
	if (isProxy()){
		auto offset_t= proxyData->master->getTransform()*
			getTransform().inversed();
		proxyData->master->setPosition(p + offset_t.translation);
	}
	else if (body){
		body->SetTransform(p.b2(), body->GetAngle());
		body->SetAwake(true);
		overrideCachedValuesByB2();
	}
	addToGrid();
}

void RigidObject::setRotation(real32 r){
	removeFromGrid();
	if (isProxy()){
		auto offset_t= proxyData->master->getTransform()*
			getTransform().inversed();
		proxyData->master->setTransform(
				offset_t*Transform(r, getPosition()));
	}
	else if (body){
		body->SetTransform(body->GetPosition(), r);
		body->SetAwake(true);
		overrideCachedValuesByB2();
	}
	addToGrid();
}

void RigidObject::setFixedRotation(bool b){
	/// @todo Proxy impl
	bodyDef.def.fixedRotation= b;
	if (body){
		body->SetFixedRotation(b);
	}
}

bool RigidObject::hasFixedRotation() const {
	return bodyDef.def.fixedRotation;
}

void RigidObject::setGravityScale(real64 scale){
	/// @todo Proxy impl
	bodyDef.def.gravityScale= scale;
	if (body){
		body->SetGravityScale(scale);
	}
}

bool RigidObject::isAwake() const {
	return getB2Body()->IsAwake();
}

real64 RigidObject::getBreakStress() const {
	// Current implementation finds toughest material and returns the toughness.
	// That is safe because then breaking valuable stuff (e.g. something containing diamonds)
	// is never too easy so odds for accidentally destroying something are then minimized

	real64 greatest_toughness= -1.0;

	for (const auto& fix : fixtures){
		const physics::Material* mat= fix->getMaterial();

		if (!mat)
			return -1.0;

		real64 toughness= mat->getToughness();

		if (toughness < 0.0)
			return -1.0; // Nothing can be tougher than infinitely tough material

		if (toughness > greatest_toughness){
			greatest_toughness= toughness;
		}
	}

	return greatest_toughness;
}

void RigidObject::breakSome(
		const collision::Shape& break_area,
		bool boolean_and){
	PROFILE();
	ensure(isPartiallyBreakable());

	removeFromGrid();

	// Detach joints if needed
	/// @note Detahching a joint can trigger events which modify other joints
	bool detached= false;
	do {
		detached= false;
		for (Joint* j : getJoints()){
			SizeType i= 1337;
			if (&j->getObject(0) == this)
				i= 0;
			else if (&j->getObject(1) == this)
				i= 1;

			bool overlaps= break_area.overlaps(j->getAnchor(i), Transform{});
			if (	(overlaps && !boolean_and) ||
					(!overlaps && boolean_and)){
				j->detach();
				detached= true;
				break;
			}
		}
	} while (detached);

	constexpr real64 imprecision= 0.2;
	constexpr real64 min_area= 0.001;
	// Break from fixtures
	for (auto fix_it= fixtures.begin(); fix_it != fixtures.end();){
		RigidFixture& fix= *fix_it->get();

		const collision::Shape* shp= fix.getShape();

		resources::ResourceRef<collision::Shape> new_shp;

		if (shp){
			if (!shp->overlaps(break_area, Transform{}, fix.getTransform())){
				++fix_it;
				continue;
			}

			new_shp.ref()= shp->erasedBy(	break_area,
											util::RtTransform2d{},
											fix.getTransform(),
											imprecision,
											boolean_and);
			new_shp.ref().clean(min_area);
			fix.setShape(new_shp);
		}

		if (	!shp || !new_shp.isSet() || new_shp.get().empty()){
			fix_it= fixtures.erase(fix_it);
		}
		else {
			++fix_it;
		}
	}

	if (fixtures.empty()){
		setFullyBroken();
	}
	else {
		addToGrid();
		OnBreakCb::trigger();
	}
}

void RigidObject::setFullyBroken(){
	if (!fullyBroken){
		removeFromGrid();
		fullyBroken= true;
		detachJoints();

		destroyB2Body();

		OnBreakCb::trigger();
	}
}

b2Body* RigidObject::getB2Body() const {
	if (isProxy())
		return proxyData->master->body;
	else
		return body;
}

util::DynArray<Fixture*> RigidObject::getFixtures() const {
	util::DynArray<Fixture*> ret;
	for (auto& m : fixtures){
		ret.pushBack(m.get());
	}
	return ret;
}

void RigidObject::weld(RigidObject& o){
	if (&o == this)
		return;

	if ((isProxyMaster() || isNormal()) && o.isNormal()){
		// Handles
		// this: normal, o: normal
		// this: master, o: normal

		ProxyData proxy_data;
		proxy_data.master= this;
		proxy_data.offset= o.getTransform()*getTransform().inversed();
		proxy_data.positionToCm= o.getCenterOfMass() - getPosition();
		proxy_data.mass= o.getMass();
		proxy_data.inertia= o.getInertia();

		ProxyMasterData proxy_m_data;
		proxy_m_data.positionToCm= getCenterOfMass() - getPosition();
		proxy_m_data.mass= getMass();
		proxy_m_data.inertia= getInertia();

		// Transfer b2 fixtures to this body
		for (auto& fix : o.getRigidFixtures()){
			fix->changeOwner(*this);
		}

		// Make 'this' a master
		if (!proxyMasterData)
			proxyMasterData= util::makeUniquePtr<ProxyMasterData>(proxy_m_data);
		proxyMasterData->proxies.pushBack(&o);

		// Make 'o' a proxy
		o.proxyData= util::makeUniquePtr<ProxyData>(proxy_data);
		ensure(o.getB2Body() == getB2Body());

		// Transfer b2 joints to this body after making a proxy but before destroying b2 body
		// of the proxy, because joint needs to destroy and create the b2 joint
		for (auto& joint : o.getJoints()){
			joint->recreate();
		}

		if (o.body)
			o.destroyB2Body();

		onStaticnessChange();
		o.onStaticnessChange();
	}
	else if (isProxy() && !o.isProxyMaster()){
		// Handles
		// this: proxy,	o: proxy
		// this: proxy, o: normal

		// Welding to proxy can be reduced to welding to master
		proxyData->master->weld(o);
	}
	else {
		// Handles
		// this: master, o: master
		// this: master, o: proxy
		// this: proxy,	 o: proxy
		// this: normal, o: master
		// this: normal, o: proxy

		if (isProxy()){
			proxyData->master->weld(o);
		}
		else if (o.isProxy()){
			weld(*o.proxyData->master);
		}
		else {
			// Handles
			// this: master, o: master
			// this: normal, o: master

			// Merge two groups
			util::DynArray<RigidObject*> to_be_welded= o.proxyMasterData->proxies;
			to_be_welded.pushBack(&o);

			o.unweldGroup();

			for (auto obj : to_be_welded){
				ensure(obj->isNormal());
				weld(*obj);
			}
		}
	}

	ensure(!(isProxy() && isProxyMaster()));
}

void RigidObject::unweldGroup(){
	ensure(!isNormal());

	if (isProxy()){
		proxyData->master->unweldGroup();
	}
	else {
		while (isProxyMaster()){
			RigidObject& o= *proxyMasterData->proxies.back();

			// Remove proxy from 'this'
			proxyMasterData->proxies.remove(&o);
			if (proxyMasterData->proxies.empty())
				proxyMasterData.reset();

			// Make 'o' normal
			o.proxyData.reset();
			o.createB2Body();
			ensure(o.getB2Body() != getB2Body());

			// Transfer b2 fixtures back to body of o
			for (auto& fix : o.getRigidFixtures()){
				fix->changeOwner(o);
			}

			// Transfer b2 joints back to body of o
			for (auto& joint : o.getJoints()){
				joint->recreate();
			}

			o.onStaticnessChange();
		}

		onStaticnessChange();
	}
}

void RigidObject::setMirrored(bool b){
	if ((int32)isMirrored() == (int32)b)
		return;

	removeFromGrid();
	if (isProxy()){
		Transform prev_t= getTransform();

		proxyData->master->setMirrored(!proxyData->master->isMirrored());

		// Mirroring is done along x-axis of the master body
		// Prevent shifting of the object after flipping around master
		setTransform(prev_t);
	}
	else {
		Object::setMirrored(b);
		recreateFixtures();

		if (isProxyMaster()){
			proxyMasterData->positionToCm.x *= -1.0;
			for (auto& object : proxyMasterData->proxies){
				object->proxyData->offset.translation.x *= -1.0;
				object->proxyData->positionToCm.x *= -1.0;
				object->Object::setMirrored(!object->isMirrored());
			}

			// Update offsets to positions before recreating
			overrideCachedValuesByB2();

			for (auto& object : proxyMasterData->proxies){
				object->recreateFixtures();
			}
		}

		/// @todo Mirror joints
	}
	addToGrid();
}

void RigidObject::setGhostliness(real64 g){
	if (getGhostliness() == g)
		return;

	Object::setGhostliness(g);
	recreateFixtures();
}

void RigidObject::preStepUpdate(){
}

void RigidObject::postStepUpdate(){	
	bool was_awake= isAwake();
	if (was_awake)
		removeFromGrid();

	if (!isProxy())
		updateCachedValues(getB2CachedValues());

	if (isProxyMaster()){
		// Proxies must be updated after master
		for (auto& p : proxyMasterData->proxies){
			p->updateCachedValues(p->getB2CachedValues());
		}
	}

	if (was_awake)
		addToGrid();
}

void RigidObject::preStepUpdateForAll(){
	PROFILE();
	for (auto& obj : nonStaticRigidObjects){
		obj->preStepUpdate();
	}
}

void RigidObject::postStepUpdateForAll(){
	PROFILE();
	for (auto& obj : nonStaticRigidObjects){
		obj->postStepUpdate();
	}
}

void RigidObject::updateEstimationsForAll(real64 relative_time){
	PROFILE();
	for (auto& obj : nonStaticRigidObjects){
		obj->updateEstimation(relative_time);
	}
}

void RigidObject::createB2Body(){
	PROFILE_("physics");
	ensure(!isProxy());
	ensure(!body);

	// Use cached values
	bodyDef.def.position= getPosition().b2();
	bodyDef.def.angle= getRotation();
	bodyDef.def.angularVelocity= getAngularVelocity();
	bodyDef.def.linearVelocity= getVelocity().b2();

	body= gWorld->getB2World().CreateBody(&bodyDef.def);

	onStaticnessChange();
}

void RigidObject::destroyB2Body(){
	PROFILE_("physics");
	ensure(body);
	ensure(getJoints().empty() || isProxy());
	gWorld->getB2World().DestroyBody(body);
	body= nullptr;

	onStaticnessChange();
}

util::Vec2d RigidObject::getB2Position() const {
	if (getB2Body())
		return util::Vec2d(getB2Body()->GetPosition());
	else
		return bodyDef.def.position;
}

real64 RigidObject::getB2Rotation() const {
	if (getB2Body())
		return getB2Body()->GetAngle();
	else
		return bodyDef.def.angle;
}

util::Vec2d RigidObject::getB2Velocity() const {
	if (getB2Body())
		return getB2Body()->GetLinearVelocity();
	else
		return bodyDef.def.linearVelocity;
}

real64 RigidObject::getB2AngularVelocity() const {
	if (getB2Body())
		return getB2Body()->GetAngularVelocity();
	else
		return bodyDef.def.angularVelocity;
}

Object::CachedValues RigidObject::getB2CachedValues() const {
	CachedValues v;
	v.transform.translation= getB2Position();
	v.transform.rotation= getB2Rotation();
	v.velocity.translation= getB2Velocity();
	v.velocity.rotation= getB2AngularVelocity();

	if (isProxy())
		v.transform= proxyData->offset*v.transform;

	return v;
}

void RigidObject::overrideCachedValuesByB2(){
	removeFromGrid();
	if (!isProxy())
		resetCachedValues(getB2CachedValues());

	if (isProxyMaster()){
		// Have proxies updated at the same frame as master
		for (auto& p : proxyMasterData->proxies){
			p->resetCachedValues(p->getB2CachedValues());
		}
	}
	addToGrid();
}

void RigidObject::recreateBody(){
	if (body){
		ensure(getJoints().empty());
		destroyB2Body();
	}

	createB2Body();

	for (int32 i=0; i<(int)fixtureDefs.size(); i++){
		createFixture(*fixtureDefs[i]);
	}
}

void RigidObject::recreateFixtures(){
	for (auto& fix : getRigidFixtures())
		fix->recreate();
}

void RigidObject::createFixture(const RigidFixtureDef& fix_def){
	if (fix_def.getShape() == 0)
		return;

	/// @todo Recalculate mass, inertia and center of mass in case of a proxy
	release_ensure_msg(!isProxy() && !isProxyMaster(), "@todo proxy impl");

	RigidObject* body= this;
	if (isProxy())
		body= proxyData->master;

	fixtures.pushBack(util::makeUniquePtr<RigidFixture>(*body, *this, fix_def));
}

void RigidObject::onStaticnessChange(){
	bool was_nonstatic= nsSelfIter != nonStaticRigidObjects.end();
	bool is_nonstatic= getB2Body() && !bodyDef.isStatic();

	if (was_nonstatic && !is_nonstatic){
		nonStaticRigidObjects.erase(nsSelfIter);
		nsSelfIter= nonStaticRigidObjects.end();
	}
	else if (!was_nonstatic && is_nonstatic){
		nonStaticRigidObjects.pushBack(this);
		nsSelfIter= std::prev(nonStaticRigidObjects.end());
	}
}

util::DynArray<RigidObject*> RigidObject::getWeldGroupObjects() const {
	if (isProxyMaster()){
		auto group= proxyMasterData->proxies;
		group.pushBack(const_cast<RigidObject*>(this));
		return group;
	}
	else if (isProxy()){
		return proxyData->master->getWeldGroupObjects();
	}
	else {
		return {};
	}
}

void RigidObject::removeFromGrid(){
	if (!inGrid)
		return;

	if (fixtures.empty()) {
		physics::remove(gWorld->getGrid().getCell(getPosition()), *this);
	} else {
		for (auto&& fix : fixtures) {
			gWorld->getGrid().remove(fix.ref(), getTransform());
		}
	}
	inGrid= false;
}

void RigidObject::addToGrid(){
	if (inGrid || !isActive())
		return;

	if (fixtures.empty()) {
		physics::add(gWorld->getGrid().getCell(getPosition()), *this);
	} else {
		for (auto&& fix : fixtures) {
			gWorld->getGrid().add(fix.ref(), getTransform());
		}
	}
	inGrid= true;
}

} // physics
} // clover
