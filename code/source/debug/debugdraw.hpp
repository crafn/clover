#ifndef CLOVER_DEBUGDRAW_HPP
#define CLOVER_DEBUGDRAW_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "util/string.hpp"
#include "util/coord.hpp"
#include "util/linkedlist.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/model.hpp"
#include "visual/mesh.hpp"

#include <bitset>

namespace clover {
namespace visual {

class TextModel;
class ModelEntity;

} // visual
namespace physics {

class Draw;

} // physics
namespace debug {

class DebugDraw {
    struct Primitive {
        enum Type {

            Line,
			Polygon,
            Cross,
            Rect,
            Circle,
            Text
        };

        Primitive();
        ~Primitive();
		Primitive(const Primitive&) = delete;
		Primitive& operator=(const Primitive&) = delete;
		Primitive(Primitive&&);
		Primitive& operator=(Primitive&&);

        void updateGeometry();

        Type type;

        util::Coord p1, p2;
        real32 rotation;
        util::Color color;

        real32 timeLeft;

		visual::ModelEntityDef entityDef;
        visual::Entity entity;
        visual::Model* model;
		visual::TriMesh	mesh;

        util::Str8 textStr;

        bool fade;
        bool filled;
    };

public:
	enum class DrawFlag {
		Common, // Are methods in this class enabled
		Physics, // Is physics::Draw enabled (it has its own sub-flags)
		Last // Flag count
	};

    DebugDraw();
	virtual ~DebugDraw();

    void setEnabled(DrawFlag f, bool enable=true);
	bool isEnabled(DrawFlag f);
	void toggle(DrawFlag f);

    void addLine(util::Coord p1,
                 util::Coord p2,
                 util::Color color=util::Color{0,0,0,0.5},
                 real32 time= 0.0,
                 bool fade=false);

    void addCross(util::Coord p1,
				  util::Color color=util::Color{0,0,0,0.5},
                  real32 size=0.1,
                  real32 time= 0.0,
                  bool fade=false);

    void addRect(util::Coord center,
                 util::Coord radius,
                 util::Color color=util::Color{0,0,0,0.5f},
                 real32 rotation=0,
                 real32 time= 0.0,
                 bool fade= false,
                 bool filled= false);

    void addFilledRect(util::Coord center,
                 util::Coord radius,
                 util::Color color= util::Color{0,0,0,0.5f},
                 real32 rotation=0,
                 real32 time= 0.0,
                 bool fade= false);

    void addCircle(util::Coord center,
                   util::Coord radius,
                   util::Color color= util::Color{0,0,0,0.5f},
                   real32 time= 0.0,
                   bool fade= false,
                   bool filled= false);
				   
    void addFilledCircle(util::Coord center,
                   util::Coord radius,
                   util::Color color= util::Color{0,0,0,0.5f},
                   real32 time= 0.0,
                   bool fade= false);

    void addText(	util::Coord pos, 
					util::Str8,
					util::Vec2d alignment=util::Vec2d{0,1},
					util::Color color= util::Color{0,0,0,0.7f},
					real32 time=0.0,
					bool fade=false);


	void addFilledPolygon(util::DynArray<util::Coord>& vertices,
					util::Color color= util::Color{0,0,0,0.5f},
					real32 time= 0.0,
					bool fade= false);


	void addPolygon(util::DynArray<util::Coord>& vertices,
					util::Color color= util::Color{0,0,0,0.5f},
					real32 time= 0.0,
					bool fade= false,
					bool filled= false);

    void update();

	physics::Draw& getPhysicsDraw(){ ensure(physicsDraw); return *physicsDraw; }

private:
    util::LinkedList<Primitive> primitives;

	uint32 flagToIndex(DrawFlag f) const { return static_cast<uint32>(f); }
	std::bitset<static_cast<uint32>(DrawFlag::Last)> drawFlags;
	
	physics::Draw* physicsDraw;
};

extern DebugDraw *gDebugDraw;

} // debug
} // clover

#endif // CLOVER_DEBUGDRAW_HPP
