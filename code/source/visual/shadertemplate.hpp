#ifndef CLOVER_VISUAL_SHADERTEMPLATE_HPP
#define CLOVER_VISUAL_SHADERTEMPLATE_HPP

#include "build.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace visual {

class ShaderTemplate;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::ShaderTemplate> {
	DECLARE_RESOURCE_TRAITS(visual::ShaderTemplate, String)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Path("vertSrc"),
							AttributeDef::Path("fragSrc"),
							AttributeDef::Path("geomSrc"),
							AttributeDef::String("vertexType"))

	typedef SubCache<visual::ShaderTemplate> SubCacheType;

	static util::Str8 typeName(){ return "ShaderTemplate"; }
	static util::Str8 identifierKey(){ return "name"; }

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::ShaderTemplate& shd) { return false; }
};

} // resources
namespace visual {

/// A set of #defines for a shader template
struct ShaderOptions {
	util::Set<util::Str8> defines;
	util::Map<util::Str8, int32> values;
};

/// Representation of a shader program source code
class ShaderTemplate : public resources::Resource {
public:
	DECLARE_RESOURCE(ShaderTemplate)

	ShaderTemplate();

	virtual void resourceUpdate(bool load, bool force= false) override;
	virtual void createErrorResource() override;

	const util::Str8& getName() const { return name.get(); }

	const util::Str8& getVertSrc() const { return vertCode; }
	const util::Str8& getFragSrc() const { return fragCode; }
	const util::Str8& getGeomSrc() const { return geomCode; }

	const util::Str8& getVertexType() const { return vertexType.get(); }

private:
	RESOURCE_ATTRIBUTE(String, name)
	RESOURCE_ATTRIBUTE(Path, vertSrc)
	RESOURCE_ATTRIBUTE(Path, fragSrc)
	RESOURCE_ATTRIBUTE(Path, geomSrc)
	RESOURCE_ATTRIBUTE(String, vertexType)
	util::Str8 vertCode, fragCode, geomCode;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTEMPLATE_HPP
