#include "box2d.hpp"
#include "chunk_util.hpp"
#include "debug/debugdraw.hpp"
#include "draw.hpp"
#include "fluid_mgr.hpp"
#include "global/cfg_mgr.hpp"
#include "ghostlydynamics.hpp"
#include "material.hpp"
#include "object_rigid.hpp"
#include "collision/baseshape_circle.hpp"
#include "collision/baseshape_polygon.hpp"
#include "collision/query.hpp"
#include "util/profiling.hpp"
#include "world.hpp"

namespace clover {
namespace physics {

static constexpr real64 breakImprecision= 0.2;

static util::DynArray<RigidObject*> objectsByRadialField(
		util::Vec2d center, real64 mag, real64 hvd){
	constexpr real64 min_impulse= 0.1;
	real64 rad= hvd*log(mag/min_impulse)/log(2.0);
	
	constexpr real64 max_rad= 20.0;
	rad= util::limited(rad, 0.0, max_rad);
	
	collision::CircleBaseShape shape(center, rad);
	
	util::DynArray<RigidObject*> objects;
	collision::Query::rigidFixture.potentialRect(
			center,
			util::Vec2d(rad),
			[&](RigidFixture& f) -> bool {
		if (f.overlaps(shape))
			objects.pushBack(&f.getObject());
		return true;
	});
	
	return util::duplicatesRemoved(objects);
}

World* gWorld= 0;

World::World(GridDef grid_def, util::Vec2d gravity, FluidMgr* fluid_mgr)
		: grid(grid_def)
		, gravity(gravity)
		, fluidMgr(fluid_mgr){
	box2dWorld= new b2World(toB2(gravity));
	box2dWorld->SetContactListener(&contactListener);
}

World::~World(){
	delete staticRigidObject; staticRigidObject= nullptr;
	delete box2dWorld; box2dWorld= nullptr;
}

void World::init(){
	RigidObjectDef def;
	def.setStatic(true);
	staticRigidObject= new RigidObject(def);

	box2dWorld->SetDebugDraw(&debug::gDebugDraw->getPhysicsDraw());
}

void World::step(real64 dt, int32 vel_iterations, int32 pos_iterations){
	PROFILE();
	processGhostlyInteractions(contactListener.getGhostlyContacts(), dt);

	RigidObject::preStepUpdateForAll();
	{ PROFILE();
		box2dWorld->Step(dt, vel_iterations, pos_iterations);
	}
	RigidObject::postStepUpdateForAll();
	
	processBreakingContacts();
}

void World::applyRadialImpulseField(
		util::Vec2d center, real64 mag, real64 hvd){
	// Impulse formula: I = M*2^(-r/halfvaluedistance)

	// Apply field to objects
	/// @todo Impulse/damage calculation to take object/field shape into account
	for (RigidObject* obj : objectsByRadialField(center, mag, hvd)){
		util::Vec2d r= obj->getPosition() - center;
		real64 r_len= r.length();
		
		obj->applyImpulse(r.normalized()*mag*pow(2, -r_len/hvd));
	}
}

void World::applyRadialStressField(
		util::Vec2d center, real64 mag, real64 hvd){
	// Stress formula: S = |d/dr I| = M/halfvaluedistance*2^(-r/halfvaluedistance)*log(2)

	// Apply field to objects
	/// @todo Impulse/damage calculation to take object shape into account
	for (RigidObject* obj : objectsByRadialField(center, mag, hvd)){
		util::Vec2d r= obj->getPosition() - center;
		real64 r_len= r.length();
		
		real64 obj_breakstress= obj->getBreakStress();
		if (obj_breakstress < 0.0)
			continue;
		
		if (obj->isPartiallyBreakable()){
			real64 stress_rad=
				hvd*(1 - log(hvd*obj_breakstress/mag)/log(2));
			if (stress_rad > 0.01){
				collision::Shape break_shp;
				break_shp.add(
						collision::CircleBaseShape(center, stress_rad));
				
				obj->breakSome(break_shp);
			}
		}
		else {
			real64 stress= mag*pow(2, -r_len/hvd)*log(2)/hvd;
			
			if (stress >= obj_breakstress)
				obj->setFullyBroken();
		}
	}

}

void World::addChunk(util::Vec2i chunk_pos, uint32 chunk_width){
	ensure(chunk_width == grid.getChunkWidth());
	grid.addChunk(chunk_pos);

	if (fluidMgr){
		util::Vec2i fluid_ch_pos= chunkToFluidChunkPos(chunk_pos);
		auto& counter= fluidChunkCounters[fluid_ch_pos];
		if (counter == 0)
			fluidMgr->addChunk(fluid_ch_pos);
		++counter;
	}
}

void World::removeChunk(util::Vec2i chunk_pos, uint32 chunk_width){
	ensure(chunk_width == grid.getChunkWidth());
	grid.removeChunk(chunk_pos);

	if (fluidMgr){
		util::Vec2i fluid_ch_pos= chunkToFluidChunkPos(chunk_pos);
		auto& counter= fluidChunkCounters[fluid_ch_pos];
		--counter;
		ensure(counter >= 0);
		if (counter == 0){
			fluidChunkCounters.erase(fluidChunkCounters.find(fluid_ch_pos));
			fluidMgr->removeChunk(fluid_ch_pos);
		}
	}
}

util::DynArray<World::BreakingCollision> World::getBreakingCollisions(
			const util::DynArray<PostSolveContact>& contacts){
	PROFILE();
	/// @todo Solution which works when both are partially breaking
	
	const SizeType max_collisions_for_object= global::gCfgMgr->get("physics::maxBreakingCollisionsForObject", 1);
	util::Map<Object*, SizeType> collisions_for_object;
	
	util::DynArray<BreakingCollision> breaking_collisions;
	for (const auto& c : contacts){
		for (SizeType i= 0; i < 2; ++i){
			auto& side= c.getSide(i);
			RigidObject& ob= *static_cast<RigidObject*>(NONULL(side.object));
			Fixture& fix= *NONULL(side.fixture);
			Fixture& other_fix= *NONULL(c.getSide(!i).fixture);
			ensure(&fix != &other_fix);

			if (ob.isPartiallyBreakable())
				continue;

			const collision::Shape* shp=
				static_cast<RigidFixture*>(&fix)->
				getShape();
			if (!shp)
				continue;

			// Could also check for impulse if this is not enough
			if (!ob.isAwake())
				continue;

			SizeType& collisions= collisions_for_object[&ob];
			if (collisions > max_collisions_for_object)
				continue;

			++collisions;

			const ContactManifold& mfold= c.getManifold(i);
			
			util::DynArray<util::Polygon> polys=
				shp->asUnionPolygons(breakImprecision);
			real64 max_poly_rad= 0.0;
			for (auto& p : polys){
				p.transform(fix.getTransform());
				util::Vec2d rad= p.getBoundingRadius();
				if (rad.x > max_poly_rad)
					max_poly_rad= rad.x;
				if (rad.y > max_poly_rad)
					max_poly_rad= rad.y;
			}


			// Obtusion is measured by calculating area inside circle near
			// collision point
			real64 collision_a= 0.0;
			real64 circle_a= 0.0;
			real64 circle_rad= 0.0;
			{
				util::Vec2d center= mfold.averagePoint;
				circle_rad= util::limited(max_poly_rad, 0.05, 1.0);
				circle_a= util::pi*circle_rad*circle_rad;

				util::PolyClipper circle_clip;
				circle_clip.addCircleClipper(
						center, circle_rad, 1.0/breakImprecision);
				circle_clip.addSubjects(polys);
				auto clipped_polys= circle_clip.execute(
						util::PolyClipper::Intersection);
				for (auto& p : clipped_polys)
					collision_a += p.getArea();
			}
			
			real64 collider_obtusion= collision_a/circle_a;
			collider_obtusion= util::limited(collider_obtusion, 0.01, 1.0);

			BreakingCollision collision;
			collision.collider= &ob;
			collision.material= other_fix.getMaterial();
			collision.normal= mfold.normal;
			// std::min of object impulse and contact impulse solves
			// the following problems:
			//	- Omnisolve-like behavior when stacking objects
			//		(object impulse ~ zero)
			//	- ghostly collisions when momentum is changed by e.g. joint
			//		(contact impulse ~ zero)
			/// @todo Take angular impulse into account
			collision.impulse=
				std::min(	ob.getImpulse().translation.length(),
							side.totalImpulse.length());
			collision.hitPoint= mfold.averagePoint;
			collision.obtusionCircleRadius= circle_rad;
			collision.colliderObtusion= collider_obtusion;
			collision.colliderPolys= std::move(polys);
			collision.maxPolyRadius= max_poly_rad;
			breaking_collisions.pushBack(collision);

			break;
		}
	}
	return breaking_collisions;
}

void World::processBreakingContacts(){
	PROFILE();
	SizeType max_breaking_collisions= global::gCfgMgr->get<SizeType>("physics::maxBreakingCollisions", 10);
	util::DynArray<BreakingCollision> cols=
			getBreakingCollisions(contactListener.popBreakingContacts());
	
	if (cols.size() > max_breaking_collisions){
		print(debug::Ch::Phys, debug::Vb::Moderate,
				"Simultaneous breaking collisions exceeded maximum: %i > %i", 
				(int32)cols.size(),
				(int32)max_breaking_collisions);
	}

	SizeType i= 0;	
	for (auto& col : cols){
		breakByCollision(col);
		++i;
		if (i >= max_breaking_collisions)
			break;
	}
}

void World::breakByCollision(const BreakingCollision& col){
	PROFILE();
	ensure_msg(	!NONULL(col.collider)->isPartiallyBreakable(),
				"@todo partially breaking stuff colliding together");

	/// @todo Early return if impulse is too small

	// Find neighbours with the same material (thinking blocks here)
	util::DynArray<RigidFixture*> fixtures;
	util::DynArray<RigidObject*> objects;
	{ PROFILE();
		collision::Query::rigidFixture.potentialRect(
				col.hitPoint,
				util::Vec2d(col.maxPolyRadius*2.0),
				[&](RigidFixture& f) -> bool {
			// Can only break fixture if object is ok with that
			if (	f.getObject().getBreakStress() <=
						col.material->getToughness() &&
					f.getMaterial() == col.material){
				fixtures.pushBack(&f);
				objects.pushBack(&f.getObject());
			}
			return true;
		});
		
		fixtures= util::duplicatesRemoved(fixtures);
		objects= util::duplicatesRemoved(objects);
		
		if (objects.empty() || fixtures.empty())
			return;
	}

	// Calculate obtusion from total area covered near hitPoint
	real64 obtusion= 0.0;
	real64 circle_a= util::pi*pow(col.obtusionCircleRadius, 2);
	{ PROFILE();
		util::DynArray<util::Polygon> material_polys;
		for (const auto& fix : fixtures){
			PROFILE();
			auto shp_polys= NONULL(fix->getShape())->
				asUnionPolygons(breakImprecision);
			for (auto& poly : shp_polys){
				poly.transform(fix->getTransform());
				material_polys.pushBack(std::move(poly));
			}
		}
		util::PolyClipper clipper;
		util::Polygon circle=
			collision::CircleBaseShape(
				col.hitPoint, col.obtusionCircleRadius).
			asPolygon(breakImprecision);
		clipper.addClipper(circle);
		clipper.addSubjects(material_polys);

		real64 material_area= 0.0;
		auto clipped_polys= clipper.execute(util::PolyClipper::Intersection);
		for (auto& p : clipped_polys)
			material_area += p.getArea();
		real64 material_obtusion= material_area/circle_a;
		obtusion= std::min(material_obtusion, col.colliderObtusion);
		obtusion= util::limited(obtusion*2.0, 0.01, 1.0);

		if (material_obtusion <= util::epsilon)
			return;
	}
	
	// Calculate penetration of the collider
	real64 penetration= 0.0;
	{ PROFILE();
		/// @todo Add other properties like density and restitution to the mix
		// util::Map obstusion [0, 1] -> col_area [0, inf) 
		real64 col_area= obtusion*sqrt(circle_a)/(1.0 - obtusion);
		real64 impulse_grad= col.impulse/col_area;
		if (impulse_grad < col.material->getToughness())
			return;
		penetration= pow(impulse_grad/col.material->getToughness(), 0.2) - 1.0;
		penetration= std::min(penetration, col.obtusionCircleRadius*0.5);
	}

	// Cut from breaking material
	{ PROFILE();
		// Cut hole
		collision::Shape cut_shp;
		util::DynArray<util::Polygon> cutters= col.colliderPolys;
		for (auto& p : cutters){
			p.translate(col.normal*penetration);
			cut_shp.add(collision::PolygonBaseShape(p.getVertices()));
		}
		for (auto& obj : objects){
			obj->breakSome(cut_shp);
		}

		// Smooth surroundings
		/// @todo Should depend on toughness
		real64 smooth_amount= 0.7;
		real64 simplify_amount= 0.2;
		real64 smooth_radius= col.obtusionCircleRadius*1.5;
		debug::gDebugDraw->addFilledCircle(col.hitPoint, smooth_radius);

		util::PolyClipper clipper;
		for (auto& obj : objects){
			for (auto& fix : obj->getRigidFixtures()){
				const collision::Shape* shp= fix->getShape();
				if (!shp)
					continue;
				auto polys= shp->asPolygons(breakImprecision);
				for (auto& p : polys){
					p.transform(obj->getTransform());
					// Make sure that neighbouring blocks really overlap
					p.offset(0.01);
				}
				clipper.addSubjects(polys);
			}
		}
		auto material_polys= clipper.execute(util::PolyClipper::Union);

		for (auto& p : material_polys){
			p.simplify(col.hitPoint, smooth_radius, simplify_amount*simplify_amount);
			p.smooth(col.hitPoint, smooth_radius, smooth_amount);
		}

		collision::Shape smoothed;
		for (auto& p : material_polys){
			smoothed.add(collision::PolygonBaseShape(p.getVertices()));
		}
		for (auto& obj : objects){
			obj->breakSome(smoothed, true);
		}
	}

	/// @todo Collider should have some of its momentum back
}

util::Vec2i World::chunkToFluidChunkPos(util::Vec2i chunk_pos) const {
	if (!fluidMgr)
		return util::Vec2i{};
	
	util::Vec2d world_pos= (chunk_pos*grid.getChunkWidth()).casted<util::Vec2d>();
	return chunkVec(world_pos, fluidMgr->getChunkSize());
}

} // physics
} // clover
