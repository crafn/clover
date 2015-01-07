#include "audio/audiosourcehandle.hpp"
#include "debug/draw.hpp"
#include "global/env.hpp"
#include "physics/object.hpp"
#include "physics/fixture.hpp"
#include "physicalmaterial.hpp"
#include "physicalmaterialpair.hpp"
#include "util/string.hpp"
#include "resources/cache.hpp"
#include "worldcontactlistener.hpp"

namespace clover {
namespace game {

void WorldContactListener::onBeginContact(const physics::Contact&){
}

void WorldContactListener::onEndContact(const physics::Contact&){
}

void WorldContactListener::onPreSolveContact(const physics::Contact& c){
}

void WorldContactListener::onPostSolveContact(const physics::PostSolveContact& c){

	// Should this be in
	if (c.isNew()){
		physics::ContactManifold manifold= c.getManifold();

		// Calculate amplitude of collision sound by change of kinetic energy
		
		physics::Object* objects[2] = {
			c.getSide(0).object,
			c.getSide(1).object
		};

		real64 m[2] = {
			objects[0]->isStatic() ? 1.0 : objects[0]->getMass(),
			objects[1]->isStatic() ? 1.0 : objects[1]->getMass()
		};

		real64 J[2] = {
			objects[0]->isStatic() || objects[0]->hasFixedRotation() ? 1.0 : objects[0]->getInertia(),
			objects[1]->isStatic() || objects[1]->hasFixedRotation() ? 1.0 : objects[1]->getInertia()
		};

		// Momentum before and after collision

		util::Vec2d d_p= c.getSide(0).totalImpulse;

		util::Vec2d p_1[2] = {
			objects[0]->getMomentum(),
			objects[1]->getMomentum()
		};

		util::Vec2d p_0[2] = {
			p_1[0] - d_p,
			p_1[1] + d_p
		};

		// Angular momentum before and after collision

		real64 d_L[2]= {
			-d_p.crossZ(c.getManifold().averagePoint - objects[0]->getPosition()),
			d_p.crossZ(c.getManifold().averagePoint - objects[1]->getPosition()),
		};

		real64 L_1[2]= {
			objects[0]->getAngularMomentum(),
			objects[1]->getAngularMomentum(),
		};

		real64 L_0[2]= {
			L_1[0] - d_L[0],
			L_1[1] - d_L[1]
		};

		//print(debug::Ch::Audio, debug::Vb::Moderate, "Angular momenta: %f -> %f, %f -> %f", L_0[0], L_1[0], L_0[1], L_1[1]);

		// E_kin = E_t + E_r = p^2 / 2m + L^2 / 2J
		real64 energy_loss= 0;
		for (SizeType i=0; i<2; ++i){

			//print(debug::Ch::Phys, debug::Vb::Trivial, "Stuff %i: %f, %f, %f", i, L_0[i], d_L[i], J[i]);
			//print(debug::Ch::Phys, debug::Vb::Trivial, "Rotational energy %i before: %f", i, L_0[i]*L_0[i]/(2.0*J[i]));
			//print(debug::Ch::Phys, debug::Vb::Trivial, "Rotational energy %i after: %f", i, L_1[i]*L_1[i]/(2.0*J[i]));

			energy_loss += p_0[i].dot(p_0[i])/(2.0*m[i]) + L_0[i]*L_0[i]/(2.0*J[i]);
			energy_loss -= p_1[i].dot(p_1[i])/(2.0*m[i]) + L_1[i]*L_1[i]/(2.0*J[i]);
		}

		//print(debug::Ch::Audio, debug::Vb::Moderate, "Energy loss: %f J", energy_loss);
		if (energy_loss < 0) energy_loss= 0;

		/*
		if (c.getSide(0).fixture->getMaterial() && c.getSide(1).fixture->getMaterial()){
			// Some of the energy loss is heat, that doesn't make a sound
			real64 heat_fraction=	(c.getSide(0).fixture->getMaterial()->getRestitution()+0.2)*
									(c.getSide(1).fixture->getMaterial()->getRestitution()+0.2)/1.4
									/ (pow(m[0] + m[1], 1.5)) * 2.0;
			heat_fraction= util::limited(heat_fraction, 0.0, 1.0);
			energy_loss *= heat_fraction;
		}*/

		const real64 min_energy= 2.0;
		if (energy_loss> min_energy){
			real64 amplitude= util::limited(0.03*pow(energy_loss, 0.3), 0.0, 1.0);
			//print(debug::Ch::Audio, debug::Vb::Moderate, "Amplitude: %f", amplitude);

			// Collision sounds

			const game::PhysicalMaterial	*	m1= static_cast<const game::PhysicalMaterial*>(c.getSide(0).fixture->getMaterial()),
									*	m2= static_cast<const game::PhysicalMaterial*>(c.getSide(1).fixture->getMaterial());


			ensure(!m1 || dynamic_cast<const game::PhysicalMaterial*>(c.getSide(0).fixture->getMaterial()));
			ensure(!m2 || dynamic_cast<const game::PhysicalMaterial*>(c.getSide(1).fixture->getMaterial()));

			bool skip= false;

			if (m1 && !m1->isAllowingCollisionSound()) skip= true;
			if (m2 && !m2->isAllowingCollisionSound()) skip= true;

			bool mat_pair_found= false;

			if (m1 && m2 && !skip){

				resources::StrResourcePair physpair(m1, m2);
				const game::PhysicalMaterialPair* pair= global::g_env->resCache->findResource<game::PhysicalMaterialPair>(physpair);

				if (pair){
					const audio::Sound* pair_sound= pair->getCollisionSound();

					mat_pair_found= true;

					if (pair_sound && canBePlayed(*pair_sound, amplitude)){
						audio::AudioSourceHandle as;
						as.assignNewSource(audio::AudioSource::Type::Spatial);
						as.setPosition(manifold.averagePoint);
						as.setVolume(amplitude);
						as.playSound(*pair_sound);

						notifyPlaying(*pair_sound, amplitude);
					}
				}
			}

			if (!mat_pair_found && !skip){
				// Try some default collision sounds

				auto try_launch_sound = [&] (const game::PhysicalMaterial* a) {
					if (!a || !a->getDefaultCollisionSound()) return;
					if (!canBePlayed(*a->getDefaultCollisionSound(), amplitude)) return;

					audio::AudioSourceHandle as;
					as.assignNewSource(audio::AudioSource::Type::Spatial);

					as.setPosition(manifold.averagePoint);
					as.setVolume(amplitude);
					as.playSound(*a->getDefaultCollisionSound());

					notifyPlaying(*a->getDefaultCollisionSound(), amplitude);

				};

				try_launch_sound(m1);
				try_launch_sound(m2);
			}

		}
	}
}

bool WorldContactListener::canBePlayed(const audio::Sound& sound, real64 amplitude){
	auto it= recentSounds.find(&sound);
	if (it == recentSounds.end()) return true;
	return	it->second.time < global::g_env->realClock->getTime() - 0.1 ||
			it->second.amplitude < amplitude*0.5;
}

void WorldContactListener::notifyPlaying(const audio::Sound& sound, real64 amplitude){
	recentSounds[&sound].time= global::g_env->realClock->getTime();
	recentSounds[&sound].amplitude= amplitude;
}

} // game
} // clover
