#include "draw.hpp"
#include "global/env.hpp"
#include "hardware/glstate.hpp"
#include "util/time.hpp"
#include "util/polygon.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/entitylogic_model.hpp"
#include "visual/model_text.hpp"
#include "physics/draw.hpp"

namespace clover {
namespace debug {

Draw::Primitive::Primitive():
			type(Line),
			rotation(0),
			timeLeft(0.0),
			entity(entityDef, visual::Entity::Layer::Debug),
			model(0),
			fade(false),
			filled(false){
}

Draw::Primitive::Primitive(Primitive&& other)
		: type(other.type)
		, rotation(other.rotation)
		, timeLeft(other.timeLeft)
		, entityDef(std::move(other.entityDef))
		, entity(entityDef, visual::Entity::Layer::Debug)
		, model(other.model)
		, fade(other.fade)
		, filled(other.filled){
	entity.apply(other.entity);
}
		
Draw::Primitive::~Primitive(){
	delete model;
}

void Draw::Primitive::updateGeometry(){
	visual::TriMesh m;

	visual::TextModel* text=0;
	if (type == Text){
		if (!text)
			text= new visual::TextModel();
		entityDef.setModel(*text);
		entityDef.setSnappingToPixels();
		model= text;
	}
	else {

		model= new visual::Model();
		entityDef.setModel(*model);
		
	}

	entityDef.setEnvLight(1.0);
	//entityDef.setLayer(visual::ModelEntityDef::Layer::Debug);

	util::Vec2f		p1_vec= this->p1.getValue().casted<util::Vec2f>(),
			p2_vec= this->p2.getValue().casted<util::Vec2f>();
	//re->setPosition(util::Coord(util::Vec2d(0), p1.getType()));

	if (type == Line){
		/// @todo Coordinate scaling
		entityDef.setFilled(false);
		m.addTriangle(p1_vec,p2_vec,p2_vec);
		//entity.setScale(util::Coord(util::Vec2d(1), p1.getType()));
	}
	else if (type == Cross){
		entityDef.setFilled(false);
		
		util::Vec2f a= {1.f, 0.f}, b= {0.f, 1.f};
		
		m.addTriangle(a-b, a+b, a+b);
		b.y= -b.y;
		m.addTriangle(a-b, a+b, a+b);

		entity.setPosition(p1.getValue().xyz());
		entity.setScale(p2.getValue().xyz());
		entity.setCoordinateSpace(p2.getType());
	}
	else if (type == Rect){
		entityDef.setFilled(filled);
		if (filled)
			m.addRectByCenter(util::Vec2f(0), util::Vec2f(1));
		else {

			util::Vec2f a= {1.f, 0.f}, b= {0.f, 1.f};
			m.addTriangle(a+b, -a+b, -a+b);
			m.addTriangle(-a+b, -a-b, -a-b);
			m.addTriangle(-a-b, a-b, a-b);
			m.addTriangle(a-b, a+b, a+b);
		}

		entity.setPosition(p1.getValue().xyz());
		entity.setScale(p2.getValue().xyz());
		entity.setRotation(util::Quatd::byRotationAxis({0, 0, 1}, rotation));
		entity.setCoordinateSpace(p1.getType());
		entity.setScaleCoordinateSpace(p2.getType());
	}
	else if (type == Circle){
		entityDef.setFilled(filled);
		
		int32 reso= 30;
		real32 r= p2.getValue().x;
		real32 angle=0;
		util::Vec2f a,b;
		int32 count= reso*r+10;
		real32 step= util::tau/count;

		if (filled){
			for (int32 i=0; i<count; i++){
				a= util::Vec2f{(real32)cos(angle), (real32)sin(angle)};
				b= util::Vec2f{(real32)cos(angle+step), (real32)sin(angle+step)};

				m.addTriangle(util::Vec2f(0),a,b);

				angle += step;
			}
		}
		else {
			for (int32 i=0; i<count; i++){
				a= util::Vec2f{(real32)cos(angle), (real32)sin(angle)};
				b= util::Vec2f{(real32)cos(angle+step), (real32)sin(angle+step)};

				m.addTriangle(a,b,b);

				angle += step;
			}
		}
		
		entity.setPosition(p1.getValue().xyz());
		entity.setScale(p2.getValue().xyz());
		entity.setCoordinateSpace(p1.getType());
		entity.setScaleCoordinateSpace(p2.getType());
	}
	else if (type == Polygon){
		entityDef.setFilled(filled);
		
		util::DynArray<util::Vec2f> verts;
		m.add(mesh);

		entity.setScale(util::Vec2d(1.0).xyz());
		entity.setCoordinateSpace(p2.getType());
	}
	else if (type == Text){
		text->setActiveFont("Debug", visual::Font::Size::Small);
		
		entityDef.setColorMul(color);
		text->setAlignment(p2.getValue());
		entity.setPosition(p1.getValue().xyz());
		entity.setCoordinateSpace(p1.getType());
		entity.setScale(util::Vec2d(1.0).xyz());
		entity.setScaleCoordinateSpace(util::Coord::View_Pixels);
		text->setText(textStr);
	}

	if (type != Text)
	{
		m.setColor(color);
		mesh.clear();
		mesh.add(m);
		mesh.flush();
		model->setMesh(mesh);
	}
}

Draw::Draw()
{
	if (!global::g_env.debugDraw)
		global::g_env.debugDraw= this;

	physicsDraw= new physics::Draw();
}

Draw::~Draw(){
	delete physicsDraw;
	physicsDraw= nullptr;

	if (global::g_env.debugDraw == this)
		global::g_env.debugDraw= nullptr;
}

void Draw::setEnabled(DrawFlag f, bool enable){
	drawFlags[flagToIndex(f)]= enable;
	
	if (f == DrawFlag::Physics){
		physicsDraw->setActive(enable);
	}
}
bool Draw::isEnabled(DrawFlag f){
	return drawFlags[flagToIndex(f)];
}
void Draw::toggle(DrawFlag f){
	drawFlags[flagToIndex(f)]= !isEnabled(f);
}

void Draw::addLine(util::Coord p1, util::Coord p2, util::Color color, real32 time, bool fade){
	if (!isEnabled(DrawFlag::Common)) return;

	primitives.pushBack(Primitive());
	Primitive &p=  primitives.back();

	p.type= Primitive::Line;
	p.p1= p1;
	p.p2= p2;
	p.color= color;
	p.timeLeft= time;
	p.fade= fade;
	
	p.updateGeometry();
}

void Draw::addCross(util::Coord p1, util::Color color, real32 size, real32 time, bool fade){
	if (!isEnabled(DrawFlag::Common)) return;

	primitives.pushBack(Primitive());
	Primitive &p=  primitives.back();

	p.type= Primitive::Cross;
	p.p1= p1;
	p.p2.setRelative();
	p.p2= util::Vec2d(size);
	p.color= color;
	p.timeLeft= time;
	p.fade= fade;

	p.updateGeometry();
}

void Draw::addRect(util::Coord center, util::Coord radius, util::Color color, real32 rotation, real32 time, bool fade, bool filled){
	if (!isEnabled(DrawFlag::Common)) return;

	primitives.pushBack(Primitive());
	Primitive &p=  primitives.back();

	p.type= Primitive::Rect;
	p.p1= center;
	radius.setRelative();
	p.p2= radius;

	p.rotation= rotation;
	p.color= color;
	p.timeLeft= time;
	p.fade= fade;
	p.filled= filled;

	p.updateGeometry();
}

void Draw::addFilledRect(util::Coord center, util::Coord radius, util::Color color, real32 rotation, real32 time, bool fade){
	addRect(center,radius,color,rotation,time,fade,true);
}

void Draw::addCircle(util::Coord center, util::Coord radius, util::Color color, real32 time, bool fade, bool filled){
	if (!isEnabled(DrawFlag::Common)) return;

	primitives.pushBack(Primitive());
	Primitive &p=  primitives.back();

	p.type= Primitive::Circle;
	p.p1= center;
	radius.setRelative();
	p.p2= radius;
	p.color= color;
	p.timeLeft= time;
	p.fade= fade;
	p.filled= filled;

	p.updateGeometry();
}

void Draw::addFilledCircle(util::Coord center, util::Coord radius,	 util::Color color, real32 time, bool fade){
	addCircle(center,radius,color,time,fade,true);
}

void Draw::addText(util::Coord pos, util::Str8 str, util::Vec2d alignment, util::Color color, real32 time, bool fade){
	if (!isEnabled(DrawFlag::Common)) return;

	primitives.pushBack(Primitive());
	Primitive &p=  primitives.back();

	p.type= Primitive::Text;
	p.p1= pos;
	p.p2.setValue(alignment);
	p.color= color;
	p.timeLeft= time;
	p.fade= fade;
	p.textStr= str;

	p.updateGeometry();
}

void Draw::addFilledPolygon(util::DynArray<util::Coord>& vertices, util::Color color, real32 time, bool fade){
	addPolygon(vertices, color, time, fade, true);
}

void Draw::addPolygon(util::DynArray<util::Coord>& vertices,
				util::Color color,
				real32 time,
				bool fade,
				bool filled){
	if (!isEnabled(DrawFlag::Common)) return;
	
	primitives.pushBack(Primitive());
	Primitive &p= primitives.back();
	
	p.type= Primitive::Polygon;
	
	ensure(!vertices.empty());
	util::Coord::Type t= vertices.front().getType();
	
	util::Polygon poly;
	for (auto &m : vertices)
		poly.append(m.getValue());
		
	auto mesh= poly.triangulated();
	
	p.mesh.add(mesh.converted<decltype(p.mesh)>());

	p.color= color;
	p.timeLeft= time;
	p.fade= fade;
	p.filled= filled;
	
	p.updateGeometry();
}

void Draw::update(){
	if (isEnabled(DrawFlag::Physics)){
		
		physicsDraw->draw();
		physicsDraw->flush();
	}

	util::LinkedList<Primitive>::Iter it;

	for (it= primitives.begin(); it!= primitives.end(); it++){
		if (it->timeLeft < 0.0){
			if (it->fade && it->color.a > 0.0){
				it->color.a = util::limited(it->color.a-2.0*global::g_env.realClock->getDeltaTime(), 0.0, 1.0);
				it->updateGeometry();
			}
			else {
				it= primitives.erase(it);
				if (primitives.empty() )break;
				it--;

				//if (primitives.size() == 0 || it == primitives.end()) break;
				continue;
			}
		}
		it->timeLeft -= global::g_env.realClock->getDeltaTime();
	}
}

} // debug
} // clover
