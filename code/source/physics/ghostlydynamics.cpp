#include "ghostlydynamics.hpp"
#include "debug/debugdraw.hpp"
#include "fixture_rigid.hpp"
#include "friction.hpp"
#include "object.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace physics {

static real64 collisionRealness(const Object& a, const Object& b){
	return util::limited(
			(1.0 - a.getGhostliness())*(1.0 - b.getGhostliness()),
			0.0, 1.0);
}

bool isGhostlyContact(const Contact& c){
	if (collisionRealness(	*NONULL(c.getSide(0).object),
							*NONULL(c.getSide(1).object)) == 1.0)
		return false;

	RigidFixture& f1= *NONULL(static_cast<RigidFixture*>(c.getSide(0).fixture));
	RigidFixture& f2= *NONULL(static_cast<RigidFixture*>(c.getSide(1).fixture)); 
	if (f1.isSensor() || f2.isSensor())
		return false;

	return true;	
}

PostSolveContact onGhostlyContact(const Contact& c){
	Object& a= *NONULL(c.getSide(0).object);
	Object& b= *NONULL(c.getSide(1).object);

	/// @todo Include restitution to calculations

	const util::Vec2d contact_point= c.getManifold().averagePoint;
	const util::Vec2d normal= c.getManifold().normal;
	const util::Vec2d vel_dif=	b.getVelocity(contact_point) -
							a.getVelocity(contact_point);
	real64 normal_vel= vel_dif.dot(normal);

	PostSolveContact post_c= c.converted<PostSolveContact>();
	if (normal_vel < 0.0){
		// When realness approaches 1.0, collision becomes elastic
		// When realness approaches 0.0, collision becomes nonexistent
		real64 impulse= 2.0*collisionRealness(a, b)*normal_vel;
		impulse /= a.getInvMass() + b.getInvMass();

		a.applyImpulse(normal*impulse, contact_point);
		b.applyImpulse(-normal*impulse, contact_point);

		post_c.getSide(0).totalImpulse= normal*impulse;
		post_c.getSide(1).totalImpulse= -normal*impulse;
	}
	return post_c;
}

void processGhostlyInteractions(const util::DynArray<Contact>& contacts, real64 dt){
	PROFILE();
	for (const Contact& c : contacts){

		Object& a= *NONULL(c.getSide(0).object);
		Object& b= *NONULL(c.getSide(1).object);

		real64 friction_mul= collisionRealness(a, b);
		applyFriction(a, b, friction_mul, dt);
	}
}

} // physics
} // clover