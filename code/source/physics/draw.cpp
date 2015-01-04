#include "box2d.hpp"
#include "draw.hpp"
#include "global/env.hpp"
#include "phys_mgr.hpp"
#include "physics/world.hpp"
#include "visual/entity.hpp"
#include "visual/mesh.hpp"
#include "visual/model.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/visual_mgr.hpp"
#include "visual/camera_mgr.hpp"
#include "visual/camera.hpp"

namespace clover {
namespace physics {

Draw::Draw()
		: alpha(0.5)
		, fluidParticleFlag(false)
		, gridFlag(false){
	for (int32 i=0; i<2; ++i){
		model[i]= new visual::Model();
		modelEntityDef[i]= new visual::ModelEntityDef();
		mesh[i]= new visual::TriMesh();

		modelEntityDef[i]->setModel(*model[i]);
		modelEntityDef[i]->setEnvLight(1.0);

		model[i]->setMesh(*mesh[i]);

		modelEntity[i]= new visual::Entity(*modelEntityDef[i], visual::Entity::Layer::PhysicsDebug);

		if (i == 0)
			modelEntityDef[i]->setFilled(false);
	}
}

Draw::~Draw(){
	for (int32 i=0; i<2; ++i){
		delete modelEntity[i]; modelEntity[i]= 0;
		delete modelEntityDef[i]; modelEntityDef[i]= 0;
		delete model[i]; model[i]= 0;
		delete mesh[i]; mesh[i]= 0;
	}
}

void Draw::setActive(bool b){
	for (int32 i=0; i<2; ++i){
		modelEntity[i]->setActive(b);
	}
}

bool Draw::isActive() const {
	return modelEntity[0]->isActive();
}

void Draw::setFlag(Flag f, bool enabled){
	switch (f){
		case Flag::FluidParticles:
			fluidParticleFlag= enabled;
		break;
		case Flag::Grid:
			gridFlag= enabled;
		break;
		default:
			if (enabled)
				SetFlags(GetFlags() | (uint32)f);
			else
				SetFlags(GetFlags() & ~(uint32)f);
	}
}

bool Draw::getFlag(Flag f) const {
	switch (f){
		case Flag::FluidParticles:
			return fluidParticleFlag;
		case Flag::Grid:
			return gridFlag;
		default:
			return GetFlags() & (uint32)f;
	}
}

Draw::Flag Draw::getFlagEnum(int32 i) const {
	Flag f;
	switch(i){
		case 0: f= physics::Draw::Flag::BaseShape; break;
		case 1: f= physics::Draw::Flag::Joint; break;
		case 2: f= physics::Draw::Flag::BoundingBox; break;
		case 3: f= physics::Draw::Flag::BroadPhasePair; break;
		case 4: f= physics::Draw::Flag::CenterOfMass; break;
		case 5: f= physics::Draw::Flag::FluidParticles; break;
		case 6: f= physics::Draw::Flag::Grid; break;
		default: release_ensure_msg(0, "Invalid index for enum");
	}
	return f;
}

void Draw::draw(){
	for (int32 i=0; i<2; ++i)
		mesh[i]->clear();

	global::g_env->physMgr->getWorld().getB2World().DrawDebugData();

	if (gridFlag){
		Grid& grid= global::g_env->physMgr->getWorld().getGrid();
		uint32 chunk_width= grid.getChunkWidth();
		real64 radius= chunk_width/std::sqrt(2.0);
		for (auto&& vec : grid.getChunkPositions()){
			auto center=
				(vec.casted<util::Vec2d>() + util::Vec2d(0.5))*chunk_width;
			if (!isVisible(center, radius))
				continue;

			drawChunk(	vec,
						grid.getChunkWidth(),
						grid.getChunkCells(vec));
		}
	}

	for (int32 i= 0; i < 2; ++i){
		// Hack to inform about mesh change
		/// @todo	Spatial cache update for entities should be automatic in
		///			case of a mesh change
		modelEntity[i]->setActive(!modelEntity[i]->isActive());
		modelEntity[i]->setActive(!modelEntity[i]->isActive());
	}
}

void Draw::flush(){
	for (int32 i=0; i<2; ++i){
		mesh[i]->flush();
	}
}

void Draw::DrawPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color){
	// Maybe 10 is enough for radius
	if (!isVisible(fromB2(vertices[0]), 10)) return;

	drawPolygonImpl(vertices, vertex_count, color, false);
}

void Draw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color){
	// Maybe 10 is enough for radius
	if (!isVisible(fromB2(vertices[0]), 10)) return;

	drawPolygonImpl(vertices, vertex_count, color, true);
}

void Draw::DrawCircle(const b2Vec2& center, real64 radius, const b2Color& color){
	if (!isVisible(fromB2(center), radius)) return;

	drawCircleImpl(center, radius, color, false);
}

void Draw::DrawSolidCircle(const b2Vec2& center, real64 radius, const b2Vec2& axis, const b2Color& color){
	if (!isVisible(fromB2(center), radius)) return;

	drawCircleImpl(center, radius, color, true);
}

void Draw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color){
	if (!isVisible(fromB2(p1), 10)) return;

	b2Vec2 verts[3]= {p1, p2, p1};
	drawPolygonImpl(verts, 3, color, false);
}

void Draw::DrawTransform(const b2Transform& xf){
	if (!isVisible(fromB2(xf.p), 1)) return;

	const real64 rad= 0.3;

	b2Vec2 points[3]= {	xf.p,
						xf.p + rad*b2Vec2(cos(xf.q.GetAngle()),				sin(xf.q.GetAngle())),
						xf.p + rad*b2Vec2(cos(xf.q.GetAngle() + util::tau/4),	sin(xf.q.GetAngle() + util::tau/4))
					  };

	b2Vec2 verts[6]= {	points[0], points[1], points[0],
						points[0], points[2], points[0] };

	b2Color color(0,1,0);

	drawPolygonImpl(verts, 6, color, false);
}

bool Draw::isVisible(util::Vec2d pos, real64 radius){
	return global::g_env->visualMgr->getCameraMgr().getSelectedCamera().
		isWorldBBInViewport(pos, util::Vec2d(radius));
}

void Draw::drawCircleImpl(const b2Vec2& center, real64 radius, const b2Color& color, bool filled){
	int32 vertex_count= radius*3*20;
	b2Vec2 verts[vertex_count];

	for (int32 i=0; i<vertex_count; ++i){
		real64 angle= (real64)i/vertex_count*util::tau;

		verts[i].x= center.x + cos(angle)*radius;
		verts[i].y= center.y + sin(angle)*radius;
	}

	drawPolygonImpl(verts, vertex_count, color, filled);
}

void Draw::drawPolygonImpl(const b2Vec2* vertices, int32 vertex_count, const b2Color& color, bool filled){
	util::DynArray<visual::TriMesh::IndexType> indices;
	visual::TriMesh m;

	util::Color c= {(real32)color.r, (real32)color.g, (real32)color.b, alpha};

	// Concave polygon
	for (int32 i=0; i<vertex_count; ++i){
		m.add(util::Vec2f{(real32)vertices[i].x, (real32)vertices[i].y}, c);

		if (i < vertex_count - 1){
			indices.pushBack(0);
			indices.pushBack(i);
			indices.pushBack(i+1);
		}
	}
	m.setIndices(indices);

	int32 i=0;
	if (filled) i= 1;

	mesh[i]->add(m);
}

void Draw::drawChunk(	util::Vec2i pos,
						uint32 width,
						util::ArrayView<const Grid::Cell> cells){
	uint32 cells_in_width= static_cast<uint32>(std::sqrt(cells.size()));
	real32 cells_in_unit= static_cast<real32>(cells_in_width)/width;
	real32 cell_width= 1.0f/cells_in_unit;

	util::DynArray<visual::TriMesh::IndexType> indices;
	visual::TriMesh m;
	util::Color dyn_c{0.0, 1.0, 0.0, 1.0};
	util::Color static_c{0.0, 0.0, 1.0, 1.0};
	
	SizeType i= 0;
	for (uint32 y= 0; y < cells_in_width; ++y){
		for (uint32 x= 0; x < cells_in_width; ++x){
			ensure(i < cells.size());
			real64 portion= cells[i].dynamicPortion + cells[i].staticPortion;
			if (portion > 0.0){
				real64 color_mul= cells[i].staticPortion/portion;
				util::Color c= util::lerp(dyn_c, static_c, color_mul);
				if (cells[i].staticEdge)
					c.r= 1.0;
				if (cells[i].worldEdge)
					c.r= c.g= c.b= 0.0;
				c.a= portion*alpha;

				util::Vec2f ll{	static_cast<real32>(x)/cells_in_unit,
								static_cast<real32>(y)/cells_in_unit};
				ll += pos.casted<util::Vec2f>()*width;

				SizeType v_index= m.getVertexCount();
				m.add(ll, c);
				m.add(ll + util::Vec2f{cell_width, 0.0f}, c);
				m.add(ll + util::Vec2f{cell_width, cell_width}, c);
				m.add(ll + util::Vec2f{0.0f, cell_width}, c);

				indices.pushBack(v_index);
				indices.pushBack(v_index + 1);
				indices.pushBack(v_index + 2);
				indices.pushBack(v_index);
				indices.pushBack(v_index + 2);
				indices.pushBack(v_index + 3);
			}
			++i;
		}
	}
	m.setIndices(indices);
	mesh[1]->add(m);
}

} // physics
} // clover
