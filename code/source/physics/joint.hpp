#ifndef CLOVER_PHYSICS_JOINT_HPP
#define CLOVER_PHYSICS_JOINT_HPP

#include "build.hpp"
#include "jointtype.hpp"
#include "util/callbacker.hpp"
#include "util/class_preproc.hpp"
#include "util/dyn_array.hpp"
#include "util/ensure.hpp"
#include "util/unique_ptr.hpp"
#include "util/vector.hpp"

namespace clover {
namespace physics {

class Object;
class RigidObject;
class PhysMgr;

struct ENGINE_API OnAttachCb : public util::SingleCallbacker<>{};
struct ENGINE_API OnDetachCb : public util::SingleCallbacker<>{};

using WorldVec= util::Vec2d;

template <typename, JointType>
struct JointTypeCrtp;

/// Constraint between Objects
/// OnAttach called after objects and box2d joint are attached
/// OnDetach called before objects are detached and box2d joint destroyed
class ENGINE_API Joint	: public util::Callbacker<OnAttachCb, OnDetachCb> {
public:
	Joint();
	DELETE_COPY(Joint);
	DELETE_MOVE(Joint);
	virtual ~Joint();

	JointType getType() const { return type; }

	virtual bool isAttached() const = 0;
	explicit operator bool() const { return isAttached(); }

	/// Clears joint from objects
	virtual void detach();

	Object& getObject(SizeType i) const { return *objects.at(i); }
	SizeType getObjectCount() const { return objects.size(); }
	const util::DynArray<Object*>& getObjects() const { return objects; }

	virtual WorldVec getAnchor(SizeType i) const = 0;

	/// @return every object which is connected to the net of joints
	util::DynArray<Object*> getGraphObjects() const;
	util::DynArray<Joint*> getGraphJoints();

	void setFrictionMul(real64 mul);

private:
	void onFrictionChange() const;
	void simulateFriction(real64 dt) const;
	static void simulateFrictions(real64 dt);

protected:
	friend class Object;
	friend class RigidObject;
	template <typename, JointType> friend class JointTypeCrtp;
	void setType(JointType t){ type= t; }

	util::DynArray<Joint*> getNeighborJoints() const;

	/// Calls internally
	void addObject(Object& o);

	/// Object calls
	virtual void recreate()= 0;

private:
	friend class PhysMgr;
	int32 tableIndex;
	JointType type;

	util::DynArray<Object*> objects;

	// Joint friction is not part of box2d simulation, but our own
	real64 frictionMul= 0.0;
	mutable util::LinkedList<const Joint*>::Iter fJointsIt;
};

template <typename T, JointType Type>
struct ENGINE_API JointTypeCrtp {
	JointTypeCrtp(){
		static_cast<T*>(this)->setType(Type);
	}
};

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::OnAttachCb> {
	static util::Str8 type(){ return "physics::OnAttachCb"; }
};

template <>
struct TypeStringTraits<physics::OnDetachCb> {
	static util::Str8 type(){ return "physics::OnDetachCb"; }
};

template <>
struct TypeStringTraits<physics::Joint> {
	static util::Str8 type(){ return "physics::Joint"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_JOINT_HPP
