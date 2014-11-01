#ifndef CLOVER_PHYSICS_DRAW_HPP
#define CLOVER_PHYSICS_DRAW_HPP

#include "build.hpp"
#include "grid.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace visual {

class Entity;
class ModelEntityDef;
class Model;
class TriMesh;

} // visual
namespace physics {

/// Internal class
/// Not using debug::gDebugDraw, since it's too slow for drawing lots of things
class Draw : public b2Draw {
public:
	enum class Flag {
		BaseShape = 		b2Draw::e_shapeBit,
		Joint = 			b2Draw::e_jointBit,
		BoundingBox = 		b2Draw::e_aabbBit,
		BroadPhasePair =	b2Draw::e_pairBit,
		CenterOfMass = 		b2Draw::e_centerOfMassBit,
		FluidParticles,
		Grid
	};
	static constexpr uint32 flagCount= 7;

	Draw();
	virtual ~Draw();

	void setActive(bool b= true);
	bool isActive() const;
	
	void setAlpha(real32 a){ alpha= a; }
	real32 getAlpha() const { return alpha; }
	
	void setFlag(Flag f, bool enabled=true);
	bool getFlag(Flag f) const;
	
	Flag getFlagEnum(int32 i) const;
	
	void draw();
	void flush();

	/// Box2d requirements
	void DrawPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color);
	void DrawCircle(const b2Vec2& center, real64 radius, const b2Color& color);
	void DrawSolidCircle(const b2Vec2& center, real64 radius, const b2Vec2& axis, const b2Color& color);
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawTransform(const b2Transform& xf);
	

private:
	real32 alpha;
	bool fluidParticleFlag;
	bool gridFlag;
	
	/// 0: outline, 1: filled
	visual::ModelEntityDef* modelEntityDef[2];
	visual::Entity* modelEntity[2];
	visual::Model* model[2];
	visual::TriMesh* mesh[2];

	bool isVisible(util::Vec2d pos, real64 radius);
	
	void drawCircleImpl(const b2Vec2& center, real64 radius, const b2Color& color, bool filled);
	void drawPolygonImpl(const b2Vec2* vertices, int32 vertex_count, const b2Color& color, bool filled);
	void drawChunk(	util::Vec2i pos,
					uint32 width,
					util::ArrayView<const Grid::Cell> cells);
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_DRAW_HPP
