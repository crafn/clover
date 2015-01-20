#ifndef CLOVER_ANIMATION_CLIP_HPP
#define CLOVER_ANIMATION_CLIP_HPP

#include "armaturepose.hpp"
#include "build.hpp"
#include "resources/resource.hpp"
#include "util/class_preproc.hpp"
#include "util/cb_listener.hpp"

namespace clover {
namespace animation {
	class Clip;
} // animation

namespace resources {

template <>
struct ResourceTraits<animation::Clip> {
	DECLARE_RESOURCE_TRAITS(animation::Clip, String);

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"))

	typedef SubCache<animation::Clip> SubCacheType;

	static util::Str8 typeName(){ return "AnimationClip"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& value){ return value; }
	static util::Str8 identifierKey(){ return "name"; }
	static bool expired(const animation::Clip&){ return false; }
	
};

} // resources

namespace animation {

/// Animation clip
class Clip : public resources::Resource {
public:
	DECLARE_RESOURCE(Clip)

	Clip();
	DELETE_COPY(Clip);
	DELETE_MOVE(Clip);

	/// @param phase 0.0 == first sample, 1.0 == last sample
	ArmaturePose getPose(real32 phase, bool interpolate) const;

	real32 getFps() const { return fps; }
	SizeType getSampleCount() const { return samples.size(); }
	real32 getTime() const { return getSampleCount()/getFps(); }

	const Armature& getArmature() const;
	const util::Str8& getName() const { return nameAttribute.get(); }

	virtual void resourceUpdate(bool load, bool force= true) override;
	virtual void createErrorResource() override;

private:
	void load();
	void unload();

	void loadBvh();

	using LocalPose= ArmaturePose::Pose;

	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Path, fileAttribute)
	RESOURCE_ATTRIBUTE(Resource, armatureAttribute)

	util::CbListener<util::OnChangeCb> armatureChangeListener;

	real32 fps;
	util::DynArray<LocalPose> samples;
};

} // animation
} // clover

#endif // CLOVER_ANIMATION_CLIP_HPP
