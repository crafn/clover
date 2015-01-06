#ifndef CLOVER_COLLISION_SHAPE_HPP
#define CLOVER_COLLISION_SHAPE_HPP

#include "build.hpp"
#include "baseshape.hpp"
#include "util/dyn_array.hpp"
#include "util/mesh.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace collision {

class Shape;

} // collision
namespace resources {

template <>
struct ResourceTraits<collision::Shape> {
	DECLARE_RESOURCE_TRAITS(collision::Shape, String)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"));

	typedef SubCache<collision::Shape> SubCacheType;

	static util::Str8 typeName(){ return "Shape"; }
	static util::Str8 identifierKey(){ return "name"; }

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const collision::Shape& type){ return false; }
};

} // resources
namespace collision {

class ENGINE_API Shape : public resources::Resource {
public:
	typedef uint32 SubShapeId;
	typedef util::GenericMesh<util::Vec2d, uint16> MeshType;
	typedef std::shared_ptr<BaseShape> BaseShapePtr;
	typedef BaseShape::Transform Transform;

	DECLARE_RESOURCE(Shape)

	Shape();
	Shape(Shape&&)= default;
	Shape(const Shape&); /// Deep copy
	virtual ~Shape();

	Shape& operator=(Shape&&)= default;
	Shape& operator=(const Shape&); /// Deep copy


	/// Clones shape and adds it
	/// @return Sub-shape id
	SubShapeId add(const BaseShape& shape);

	/// Creates sub-shapes from a mesh
	void addMesh(const MeshType& m);

	/// Transforms every BaseShape by t
	void transform(const Transform& t);

	/// Mirrors along x-axis
	void mirror();

	/// Removes BaseShapes
	void clear(){ baseShapesAttribute.get().clear(); }
	bool empty() const { return baseShapesAttribute.get().empty(); }

	const util::DynArray<BaseShapePtr>& getBaseShapes() const { return baseShapesAttribute.get(); }

	util::DynArray<util::Polygon> asPolygons(real64 imprecision) const;
	util::DynArray<util::Polygon> asConvexPolygons(real64 imprecision) const;
	util::DynArray<util::Polygon> asUnionPolygons(real64 imprecision) const;

	bool overlaps(	const collision::Ray& ray,
					const Transform& transform,
					collision::RayCastResult* output= nullptr) const;
	bool overlaps(	const util::Vec2d& pos,
					const Transform& transform) const;
	bool overlaps(	const Shape&,
					const Transform& shape_transform,
					const Transform& transform) const;

	real64 distance(const Shape& shape,
					const Transform& shape_transform,
					const Transform& transform) const;

	Shape erasedBy(const Shape& other, const Transform& other_transform, const Transform& this_transform, real64 imprecision, bool boolean_and= false) const;

	void clean(real64 min_area);

	virtual void resourceUpdate(bool load, bool force=false);
	virtual void createErrorResource();
	util::Str8 getName() const { return nameAttribute.get(); }

private:
	void deepCopy(const Shape& other);

	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(BaseShapeArray, baseShapesAttribute)
};

} // collision
namespace util {

template <>
struct TypeStringTraits<collision::Shape> {
	static util::Str8 type(){ return "collision::Shape"; }
};

} // util
} // clover

#endif // CLOVER_COLLISION_SHAPE_HPP
