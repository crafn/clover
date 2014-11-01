#ifndef CLOVER_GAME_EDITOR_EC_PHYSICSTOOLS_HPP
#define CLOVER_GAME_EDITOR_EC_PHYSICSTOOLS_HPP

#include "build.hpp"
#include "../editorcomponent.hpp"
#include "physics/object_rigid.hpp"
#include "physics/joint_target.hpp"
#include "physics/draw.hpp"
#include "ui/hid/actionlistener.hpp"
#include "util/handle.hpp"

namespace clover {
namespace resources {

class Resource;

} // resources
namespace game { namespace editor {

class PhysicsToolsEc;

template <>
struct EditorComponentTraits<PhysicsToolsEc> {
	static const util::Str8 name(){ return "PhysicsTools"; }
};

class PhysicsToolsEc : public EditorComponent {
public:
	DECLARE_EDITORCOMPONENT(PhysicsToolsEc);

	enum class JointType {
		Distance,
		HardWeld,
		Prismatic,
		Revolute,
		Rope,
		Weld,
		Wheel,
		Detach,
		Mirror,
		Last
	};

	PhysicsToolsEc();
	virtual ~PhysicsToolsEc(){}
	
	void update();
	
	void setDrawingActive(bool b);
	bool isDrawingActive() const;
	
	void setDrawingFlag(physics::Draw::Flag f, bool b);
	bool getDrawingFlag(physics::Draw::Flag f) const;
	
	void setDrawingAlpha(real32 a);
	real32 getDrawingAlpha() const;
	
	physics::Draw::Flag getFlagEnum(int32 i) const;
	
	JointType getCurrentJointType() const { return currentJointType; }

private:
	void onGrabAction(bool start);
	void onAddJointAction();
	void onChangeJointTypeAction(int32 dir);

	util::Vec2d cursorOnWorld() const;

	template <typename T>
	T* createJoint(){
		joints.pushBack(util::UniquePtr<physics::Joint>(new T()));
		return static_cast<T*>(joints.back().get());
	}

	ui::hid::ActionListener<>	startDragL,
								stopDragL,
								addJointL,
								prevJointTypeL,
								nextJointTypeL;


	physics::TargetJoint grabJoint;
	JointType currentJointType= JointType::Distance;
	util::DynArray<util::UniquePtr<physics::Joint>> joints;
	util::Vec2d storedPosition;
	util::DynArray<util::Handle<physics::Object>> storedObjects;
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EC_PHYSICSTOOLS_HPP
