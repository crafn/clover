#include "friction.hpp"
#include "object.hpp"

namespace clover {
namespace physics {

void applyFriction(	Object& a, Object& b,
					real64 friction_mul, real64 dt){	

	const real64 expmul= 1.5;
	const real64 dampmul= 0.99;

	real64 a_rot_vel= a.getAngularVelocity();
	real64 b_rot_vel= b.getAngularVelocity();
	real64 a_inertia= a.getInertia();
	real64 b_inertia= b.getInertia();
	real64 m_a= a.getMass();
	real64 m_b= b.getMass();
	util::Vec2d cm_a= a.getCenterOfMass();
	util::Vec2d cm_b= b.getCenterOfMass();
	util::Vec2d cm= (cm_a*m_a + cm_b*m_b)/(m_a + m_b);
	util::Vec2d dist= cm_b - cm_a;

	util::Vec2d cm_v_a= a.getVelocity(cm);
	util::Vec2d cm_v_b= b.getVelocity(cm);
	util::Vec2d dv= (cm_v_b - cm_v_a);
	
	util::Vec2d dir= dv.normalized();
	real64 magnitude= pow(dv.lengthSqr(), expmul/2.0);
	util::Vec2d impulse= dir*friction_mul*dt*magnitude;

	util::Vec2d cm_dv= b.getVelocity(cm_b) - a.getVelocity(cm_a);
	real64 max_impulse_a_sqr= cm_dv.lengthSqr()*pow(a.getMass()*dampmul, 2);
	real64 max_impulse_b_sqr= cm_dv.lengthSqr()*pow(b.getMass()*dampmul, 2);
	real64 max_impulse_sqr= std::min(max_impulse_a_sqr, max_impulse_b_sqr);
	if (impulse.lengthSqr() > max_impulse_sqr)
		impulse= dir*sqrt(max_impulse_sqr);

	a.applyImpulse(impulse);
	b.applyImpulse(-impulse);

	real64 rot_dv= b.getAngularVelocity() - a.getAngularVelocity();
	real64 rot_dir= rot_dv < 0 ? -1 : 1;
	real64 rot_magnitude= pow(util::abs(rot_dv)*dist.length(), expmul);
	real64 rot_impulse= rot_dir*friction_mul*dt*rot_magnitude;

	real64 max_rot_impulse_a= dampmul*util::abs(rot_dv)*a_inertia;
	real64 max_rot_impulse_b= dampmul*util::abs(rot_dv)*b_inertia;
	real64 max_rot_impulse= std::min(max_rot_impulse_a, max_rot_impulse_b);
	if (util::abs(rot_impulse) > max_rot_impulse)
		rot_impulse= rot_dir*max_rot_impulse;

	a.applyAngularImpulse(rot_impulse);
	b.applyAngularImpulse(-rot_impulse);

}

} // physics
} // clover