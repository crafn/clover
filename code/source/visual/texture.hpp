#ifndef CLOVER_VISUAL_TEXTURE_HPP
#define CLOVER_VISUAL_TEXTURE_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "util/string.hpp"
#include "resources/resource.hpp"
#include "hardware/glstate.hpp"
#include "visual/image.hpp"

namespace clover {
namespace visual {

class Texture;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::Texture> {
	DECLARE_RESOURCE_TRAITS(visual::Texture, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Path("file"));
	
	typedef SubCache<visual::Texture> SubCacheType;
	
	static util::Str8 typeName(){ return "Texture"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::Texture& tex);
};

} // resources
namespace visual {

/// Drawable texture
class Texture : public resources::Resource {
public:
	DECLARE_RESOURCE(Texture)
	
	Texture();
	Texture(const Texture&)= delete;
	Texture(Texture&&);

	Texture& operator=(const Texture&)= delete;
	Texture& operator=(Texture&&);

	virtual ~Texture();
	
	virtual void resourceUpdate(bool load, bool force=false);
	virtual void createErrorResource();
	
	void bind();
	
	util::Vec2i getDimensions() const { return dimensions; }
	
	void setExternal(hardware::GlState::TexDId id, util::Vec2i size);

	/// @return OpenGL texture id
	hardware::GlState::TexDId getDId() const { onResourceUse(); return id; }
	
	const util::Str8& getName() const { return nameAttribute.get(); }
	
	uint32 getContentHash() const;
	
private:
	void submit(bool only_mipmaps=false);
	void unload(bool unload_mipmaps_also=true);
	
	/// Called when fileAttribute or the pointed file is changed
	void onFileChange();

	hardware::GlState::TexDId id;
	bool external= false;
	bool mipMapsLoaded;
	util::Vec2i dimensions;

	Image image;
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Path, fileAttribute)
};

} // visual
} // clover

#endif // CLOVER_VISUAL_TEXTURE_HPP