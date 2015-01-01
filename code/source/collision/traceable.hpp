#ifndef CLOVER_COLLISION_TRACEABLE_HPP
#define CLOVER_COLLISION_TRACEABLE_HPP

#include "build.hpp"
#include "collision/ray.hpp"
#include "util/transform.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace collision {

/// Abstract base class for queriable objects
class Traceable {
public:
	using Transform= util::RtTransform<real64, util::Vec2d>;

	enum TypeId {
		Fixture,
		RigidFixture
		//SoftFixture
	};

	/*	When creating a new Traceable

		To class definition:
			static constexpr collision::Traceable::TypeId staticTraceableTypeId= collision::Traceable::New_Traceable_Type_Id;

		To collision::Query:
			SubQuery<New_Traceable_Type> newTraceableType;

		To call addToTraceableTypeTree in constructors
	*/

	Traceable()= default;
	Traceable(const Traceable&)= default;
	Traceable(Traceable&&)= default;
	virtual ~Traceable(){};

	virtual Transform getTransform() const = 0;

	virtual bool overlaps(const util::Vec2d& pos) const = 0;
	virtual bool overlaps(const Traceable& other) const = 0;
	virtual bool overlaps(const collision::Ray& ray, collision::RayCastResult* output=0) const = 0;

	TypeId getTraceableTypeId() const { return typeTree.back(); }
	bool isConvertibleTo(TypeId id) const { for (auto& m : typeTree) if (m == id) return true; return false; }

protected:
	void addToTraceableTypeTree(TypeId t){ typeTree.pushBack(t); ensure(typeTree.size() < 5); /* failsafe */ }

private:
	util::DynArray<TypeId> typeTree;
};

} // collision
} // clover

#endif // CLOVER_COLLISION_TRACEABLE_HPP
