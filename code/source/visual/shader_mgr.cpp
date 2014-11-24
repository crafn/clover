#include "particletype.hpp"
#include "resources/cache.hpp"
#include "shader_mgr.hpp"
#include "shadertemplate.hpp"
#include "util/pair.hpp"
#include "util/tuple.hpp"
#include "vertex.hpp"

namespace clover {
namespace visual {

Shader& ShaderMgr::getShader(const util::Str8& name, const ShaderOptions& options)
{
	ShaderId id= util::hash32(util::makeTuple(name, options.defines, options.values));
	auto it= shaders.find(id);
	if (it != shaders.end())
		return it->second;

	// Create shader

	auto& shd_tpl= resources::gCache->getResource<ShaderTemplate>(name);
	Shader shd;
	shd.setSources(shd_tpl.getVertSrc(), shd_tpl.getFragSrc(), shd_tpl.getGeomSrc());
	for (auto& str : options.defines)
		shd.setDefine(str);
	for (auto& pair : options.values)
		shd.setDefine(pair.first, pair.second);

	util::DynArray<VertexAttribute> attribs;
	util::Str8 vert_type= shd_tpl.getVertexType();
	if (vert_type == "visualMesh") {
		attribs= Vertex::getAttributes();
	} else if (vert_type == "visualParticle") {
		attribs= ParticleVBOVertexData::getAttributes();
	} else {
		print(debug::Ch::Visual, debug::Vb::Critical,
				"Unknown vertex type in shader %s: %s",
				name.cStr(),
				vert_type.cStr());
	}
	shd.compile(attribs);

	shaders[id]= std::move(shd);
	return getShader(name, options);
}

} // visual
} // clover
