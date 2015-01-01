#include "collision/baseshape_polygon.hpp"
#include "game/physics.hpp"
#include "game/world_mgr.hpp"
#include "game/worldgrid.hpp"
#include "game/world_gen/chunk_gen.hpp"
#include "game/world_gen/world_gen.hpp"
#include "physics/grid.hpp"
#include "util/dynamic.hpp"
#include "util/math.hpp"
#include "util/vector.hpp"
#include "world_gen.hpp"

#define DLL_EXPORT extern "C" __attribute__((visibility ("default")))

__attribute__((visibility ("default"))) void temp_hack_to_make_symbols_exported()
{ }

namespace world_gen= clover::game::world_gen;
using namespace clover;

const real64 g_beginningOfTime= -1000.0;
const uint32 chunkWidthInBlocks= game::WorldGrid::chunkWidthInBlocks;

real64 randReal(real64 min, real64 max)
{ return util::Rand::continuous(min, max); }

int32 randInt(int32 min, int32 max)
{ return util::Rand::discrete(min, max); }

util::Vec2d randRotatedVec2d(real64 length)
{
	real64 rot= randReal(0, util::tau);
	return util::Vec2d{std::cos(rot)*length, std::sin(rot)}*length;
}

util::Vec3d randRotatedVec3d(real64 length)
{
	real64 theta= randReal(0, util::pi);
	real64 phi= randReal(0, util::tau);
	return util::Vec3d{
		std::sin(theta)*std::cos(phi),
		std::sin(theta)*std::sin(phi),
		std::cos(phi)}*length;
}

util::Vec3d randMirroredVec3d()
{
	if (randInt(0, 1) == 1)
		return util::Vec3d{1.0, 1.0, 1.0};
	else
		return util::Vec3d{-1.0, 1.0, 1.0};
}

real64 groundSurfaceY(real64 x)
{
	using std::sin;
	return	sin(x*0.02 + 2.0)*(sin(x*0.075 + 0.3)*0.8 +
			sin(x*0.07)*2.0) + sin(x*0.0037 + 2.0)*(sin(x*0.02 + 0.1) + 1.0)*10.0 +
			0.3*sin(x*0.5)*sin(x*0.011 + 0.4) + sin(x*0.0001)*50 + 3.0;
}

// Test biome distribution
real64 forestness(real64 x)
{
	const real64 min= -40.0, max= -00.0;

	if (x < min)
		return 0.0;
	else if (x > max)
		return 1.0;
	else
		return (x - min)/(max - min); // Lerp
}

/// Global initialization of world generation
DLL_EXPORT void initWorld(world_gen::WorldGen& gen)
{
	debug::print("Global world init");
	
	gen.createWorker(	"playerSpawn",
						util::Vec2d{1.0, groundSurfaceY(0.0) + 5.0},
						0.0,
						0.0);
}

DLL_EXPORT void createGroundWorkers(world_gen::ChunkGen& gen)
{
	util::Vec2d corner_pos= game::WorldGrid::chunkCornerToWorldVec(gen.getPosition()) + util::Vec2d{0.5, 0.5};

	for (uint32 x= 0; x < chunkWidthInBlocks; ++x){
		for (uint32 y= 0; y < chunkWidthInBlocks; ++y){
			util::Vec2d pos= corner_pos + util::Vec2d{(real64)x, (real64)y};
			real64 left_g_y= groundSurfaceY(pos.x - 0.5);
			real64 right_g_y= groundSurfaceY(pos.x + 0.5);
			
			if (pos.y - 0.5 > left_g_y ||
				pos.y - 0.5 > right_g_y)
				continue; // Too high
			
			if (randInt(0, 300) == 0)
				continue;
			
			gen.createWorker(	"ground",
								pos,
								0.0,
								g_beginningOfTime);
		}
	}
}

DLL_EXPORT void createGrowWorkers(world_gen::ChunkGen& gen)
{
	util::Vec2d pos= game::WorldGrid::chunkCornerToWorldVec(gen.getPosition()) + util::Vec2d(1);
	gen.createWorker("grow", pos, 0.0, randReal(0.0, 1.0));
}

DLL_EXPORT void growWork(world_gen::WorldGen& gen, const world_gen::Worker& w)
{
	const real64 check_interval= 5.0;
	gen.createWorker("grow", w.getLocation().getPosition(), 0.0, gen.getWorldMgr().getTime() + check_interval);

	physics::Grid& grid= gen.getWorldMgr().getPropertyGrid().getPhysicsGrid();
	util::Vec2i ch_pos= game::WorldGrid::worldToChunkVec(w.getLocation().getPosition());
	if (!grid.hasChunk(ch_pos))
		return;

	/// @todo ArrayView
	const util::DynArray<physics::Grid::Cell>& cells= grid.getChunkCells(ch_pos);

	uint32 width= grid.getChunkWidth();
	uint32 width_c= grid.getChunkWidth()*grid.getCellsInUnit();
	real32 half_cell= 0.5f/grid.getCellsInUnit();

	for (SizeType i= 0; i < cells.size(); ++i) {
		if (cells[i].worldEdge)
			continue;

		if (cells[i].lastStaticEdit >= w.getCreationTime()) {
			util::Vec2i cell_p{(int32)(i%width_c),
								(int32)(i/width_c)};
			util::Vec2d pos=	util::Vec2d{(real64)ch_pos.x, (real64)ch_pos.y}*width + 
						util::Vec2d{(real64)cell_p.x, (real64)cell_p.y}/width_c*width;

			util::Vec2d offset=
				util::Vec2d{cells[i].staticNormal.x, cells[i].staticNormal.y}*
				(cells[i].staticPortion - 1.0)*
				1.2*2.0*half_cell;
			util::Vec2d anchor_p= pos + util::Vec2d(half_cell) + offset;

			const physics::Grid::Cell& anchor_cell= grid.getCell(anchor_p);
			if (	anchor_cell.staticPortion < 0.0001 ||
					(!anchor_cell.staticEdge && !cells[i].staticEdge))
				continue;

			//
			// Grass
			//

			util::RtTransform2d grass_t;
			grass_t.translation=
				anchor_p + util::Vec2d{cells[i].staticNormal.x, cells[i].staticNormal.y}*0.2;
			grass_t.rotation=
				anchor_cell.staticNormal.rotationZ() - util::tau/4.0;

			const physics::Grid::Cell& grass_cell= grid.getCell(grass_t.translation);
			SizeType grass_count= 0;
			for (SizeType obj_i= 0; obj_i < physics::Grid::Cell::maxObjects; ++obj_i) {
				physics::Object* obj= grass_cell.objects[obj_i];
				if (!obj)
					continue;
				game::WorldEntity* we= game::getOwnerWe(*obj);
				if (!we)
					continue;
				if (we->getTypeName() == "grassClump")
					++grass_count;
			}

			if (anchor_cell.staticNormal.y > 0.3 && grass_count == 0) {
				util::Vec4d color{1.0 - forestness(anchor_p.x)*0.3 + randReal(-0.2, 0.2),
							1.0 - forestness(anchor_p.x)*0.1 + randReal(-0.1, 0.2),
							1.0, 1.0};

				game::WeHandle grass= gen.createEntity("grassClump", grass_t.translation);
				grass.ref().setAttribute("transform", grass_t);
				grass.ref().setAttribute("anchor", anchor_p);
				grass.ref().setAttribute("colorMul", color); 
			}
		}
	}
}

// Create ground block
DLL_EXPORT void groundWork(world_gen::WorldGen& gen, const world_gen::Worker& w){
	util::Vec2d pos= w.getLocation().getPosition();
	
	real64 left_g_y= groundSurfaceY(pos.x - 0.5);
	real64 middle_g_y= groundSurfaceY(pos.x);
	real64 right_g_y= groundSurfaceY(pos.x + 0.5);

	game::WeHandle we= gen.createEntity("block_dirt", pos);
	resources::ResourceRef<collision::Shape> shape;

	if (pos.y + 0.5 < left_g_y &&
		pos.y + 0.5 < middle_g_y &&
		pos.y + 0.5 < right_g_y){
		// Under surface
		shape= resources::ResourceRef<collision::Shape>("block");
	}
	else {
		// Near surface, extend block
		real64 left_y= left_g_y - pos.y;
		real64 middle_y= middle_g_y - pos.y;
		real64 right_y= right_g_y - pos.y;

		util::Vec2d upper_left{-0.5, left_y};
		util::Vec2d upper_middle{0.0, middle_y};
		util::Vec2d upper_right{0.5, right_y};

		util::DynArray<util::Vec2d> vertices;
		vertices.pushBack(util::Vec2d{-0.5, -0.5});
		vertices.pushBack(util::Vec2d{0.5, -0.5});
		if (right_y > -0.5)
			vertices.pushBack(upper_right);
		if (middle_y > -0.5)
			vertices.pushBack(upper_middle);
		if (left_y > -0.5)
			vertices.pushBack(upper_left);

		util::Dynamic<collision::PolygonBaseShape> poly;
		poly.get().setVertices(vertices);
		shape.ref().add(poly.get());
	}
	
	we.ref().setAttribute("shape", shape);
}

// Spawn player
DLL_EXPORT void playerSpawnWork(world_gen::WorldGen& gen, const world_gen::Worker& w){
	gen.createEntity("playerMgr");

	game::WeHandle we;
	we= gen.createEntity("testCharacter", w.getLocation().getPosition());
	we->setAttribute("player", "player0");

	// Coop test
	//we= gen.createEntity("testCharacter", w.getLocation().getPosition() + Vec2d(0, 3));
	//we.ref().setAttribute("player", "player1");
}

DLL_EXPORT void createBg(world_gen::WorldGen& gen){
	// Bg test
	
	// Limits of test background
	const real64 min_x= -300, max_x= 300;
	
	//
	// Meadows
	//
	
	for (real64 x= min_x; x < max_x; x += 3.0){
		if (forestness(x) > 0.5)
			continue;
		
		util::SrtTransform3d t{	randMirroredVec3d(),
								util::Quatd(),
								util::Vec3d{x,
											groundSurfaceY(x) - 10.0,
											-310.0 + sin(x*x*332.56)*300.0}};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_meadow0");
	}
	
	// Mountain
	{
		util::SrtTransform3d t;
		t.translation= util::Vec3d{0.0, 0.0, -1000.0};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_distantMountain0");
	}
	
	// Clouds
	for (real64 x= min_x; x < max_x; x += 20.0){
		if (forestness(x) > 0.1)
			continue;
		
		util::SrtTransform3d t;
		t.translation= util::Vec3d{	x,
									groundSurfaceY(x) + 50.0,
									-110.0 + sin(x*331.583)*100.0};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_cloud0");
	}
	
	//
	// Forest bg
	//
	
	for (real64 x= min_x; x < max_x; x += 8.0){
		if (forestness(x) < 0.95)
			continue;
	
		util::SrtTransform3d t;
		t.translation= util::Vec3d{	x,
									groundSurfaceY(x) + randReal(-1.0, 1.0)*30.0,
									-60.0};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_forest_color");
		
		real64 brightness= randReal(0.7, 1.0);
		we.ref().setAttribute("colorMul", util::Vec4d{brightness, brightness, brightness, 1.0});
	}

	// Light halos
	for (real64 x= min_x; x < max_x; x += 7.0){
		if (forestness(x) < 0.6)
			continue;
		
		util::SrtTransform3d t{	util::Vec3d(1.0)*randReal(0.5, 1.2),
								util::Quatd(),
								util::Vec3d{x,
											groundSurfaceY(x) + randReal(-1.0, 1.0)*10.0 + 20.0,
											randReal(-60.0, -40.0)}};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_forest_halo");
		
		real64 alpha= randReal(0.05, 1.0)*randReal(0.7, 1.0);
		we.ref().setAttribute("colorMul", util::Vec4d{1.0, 1.0, 1.0, alpha});
	}
	
	// Trees
	for (real64 x= min_x; x < max_x; x += 20.0){
		if (forestness(x) < 0.1)
			continue;
		
		util::SrtTransform3d t{	randMirroredVec3d(),
								util::Quatd(),
								util::Vec3d{x,
											groundSurfaceY(x) + randReal(-3.0, 10.0),
											randReal(-25.0, -7.0)}};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_forest_tree0");
	}
	
	// Silhouettes
	for (real64 x= min_x; x < max_x; x += 11.0){
		if (forestness(x) < 0.2)
			continue;
		
		real64 min_z= -45.0, max_z= -25.0;
		real64 z= randReal(min_z, max_z);
		
		util::SrtTransform3d t{	randMirroredVec3d()*util::Vec3d(randReal(0.7, 1.3)),
								util::Quatd(),
								util::Vec3d{x,
											groundSurfaceY(x) + randReal(10.0, 30.0),
											z}};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_forest_silhouette0");
		
		real64 alpha= (z - min_z)/(max_z - min_z);
		we.ref().setAttribute("colorMul", util::Vec4d{1.0, 1.0, 1.0, alpha});
	}
	
	// Leaves
	for (real64 x= min_x; x < max_x; x += 0.3){
		if (forestness(x) < 0.3)
			continue;
		
		real64 min_z= -30.0, max_z= -20.0;
		real64 z= randReal(min_z, max_z) + sin(x*0.1)*3.0;
		util::SrtTransform3d t{	randMirroredVec3d(),
								util::Quatd::byRotationAxis(randRotatedVec3d(1.0), randReal(0, 6.3)),
								util::Vec3d{x,
											groundSurfaceY(x) + randReal(15.0, 40.0),
											z}};
		game::WeHandle we= gen.createEntity("bg_staticVisual", t);
		
		we.ref().setAttribute("entityDef", "bg_forest_leaf0");
		
		real64 alpha= (z - min_z)/(max_z - min_z);
		we.ref().setAttribute("colorMul", util::Vec4d{1.0, 1.0, 1.0, alpha});
	}
	
	// Light beams
	for (real64 x= min_x; x < max_x; x += 2.0){
		if (forestness(x) < 0.5)
			continue;
		
		real64 min_z= -45.0, max_z= 1.0;
		real64 z= randReal(min_z, max_z);
		util::SrtTransform3d t{	randMirroredVec3d(),
								util::Quatd(),
								util::Vec3d{	x,
										groundSurfaceY(x) + randReal(30.0, 40.0),
										z}};
		game::WeHandle we= gen.createEntity("bg_lightBeam", t);
	}
}

// For prototyping and testing purposes
DLL_EXPORT void protoChunkInit(world_gen::ChunkGen& gen){
	if (gen.getPosition() == util::Vec2i{0, 0})
		createBg(gen.getWorldGen());

	util::Vec2d corner_pos= game::WorldGrid::chunkCornerToWorldVec(gen.getPosition());
	
	util::Vec2d center;
	center.x= corner_pos.x + chunkWidthInBlocks*0.5;
	
	if (corner_pos.y < groundSurfaceY(center.x) &&
		corner_pos.y + chunkWidthInBlocks > groundSurfaceY(center.x)){
		
		gen.createWorker(	"proto",
							util::Vec2d{center.x, groundSurfaceY(center.x)},
							4.0,
							g_beginningOfTime);
	}
	
}

DLL_EXPORT void protoWork(world_gen::WorldGen& gen, const world_gen::Worker& w){
	// Create some stones
	util::Vec2d pos= w.getLocation().getPosition();
	for (SizeType i= 0; i < 2; ++i){
		gen.createEntity("stone", pos + util::Vec2d{0.0, 0.5});
		pos.y += 0.2;

	}
	if (abs(pos.x) > 10)
		gen.createEntity("minion", pos + util::Vec2d{0.0, 0.3});
		
	// Occasional rollbot
	if (randInt(0, 5) == 0)
		gen.createEntity("rollBot", w.getLocation().getPosition() + util::Vec2d{0.0, 1.0});
		
	// Occasional woodenBarrel
	if (randInt(0, 5) == 0)
		gen.createEntity("woodenBarrel", w.getLocation().getPosition() + util::Vec2d{0.0, 2.0});

}

void* tempfake_dlsym(void*, const char* name)
{
#define FUNC(x) if (!std::strcmp(name, #x)) return (void*)x;
	FUNC(createGroundWorkers);
	FUNC(groundWork);
	FUNC(initWorld);
	FUNC(playerSpawnWork);
	FUNC(protoChunkInit);
	FUNC(protoWork);
	FUNC(createGrowWorkers);
	FUNC(growWork);
	return nullptr;
}
