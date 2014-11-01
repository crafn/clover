#ifndef CLOVER_VISUAL_MATERIAL_HPP
#define CLOVER_VISUAL_MATERIAL_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "visual/texture.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace visual {

class Material;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::Material> {
	DECLARE_RESOURCE_TRAITS(visual::Material, String)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Resource("colorMap", "Texture"),
							AttributeDef::Resource("normalMap", "Texture"),
							AttributeDef::Resource("envShadowMap", "Texture"),
							AttributeDef::Color("color", true));

	typedef SubCache<visual::Material> SubCacheType;


	static util::Str8 typeName(){ return "Material"; }
	static util::Str8 identifierKey(){ return "name"; }

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::Material&){ return false; }
};

} // resources
namespace visual {

/// Combines several textures and properties
class Material : public resources::Resource {
public:
	DECLARE_RESOURCE(Material)

	enum TexType {
		TexType_Color,
		TexType_Normal,
		TexType_EnvShadow,
		//TexType_Specular,
		TexType_Last

	};

	using BlendFunc= hardware::GlState::BlendFunc;
	static constexpr BlendFunc defaultBlendFunc{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};

	Material();
	Material(const Material&)= default;
	Material(Material&&)= default;

	void setTexture(TexType type, const Texture* tex);

	const Texture* getTexture(TexType type) const;
	bool hasTexture(TexType type) const { return getTexture(type) != nullptr; }

	void setColor(const util::Color& c){ colorAttribute.set(c); }
	const util::Color& getColor() const { return colorAttribute.get(); }

	BlendFunc getBlendFunc() const { return blendFunc; }

	virtual void resourceUpdate(bool load, bool force=false);
	virtual void createErrorResource();

	uint32 getContentHash() const;

private:
	void changeTexture(TexType type, const util::Str8& resname);
	void cacheBlendFunc();

	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Resource, colorMapAttribute)
	RESOURCE_ATTRIBUTE(Resource, normalMapAttribute)
	RESOURCE_ATTRIBUTE(Resource, envShadowMapAttribute)
	RESOURCE_ATTRIBUTE(Color, colorAttribute)
	RESOURCE_ATTRIBUTE(StringArray, blendFuncAttribute)

	util::DynArray<const Texture*> textures;
	BlendFunc blendFunc;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_MATERIAL_HPP