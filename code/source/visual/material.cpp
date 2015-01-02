#include "global/env.hpp"
#include "material.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace visual {

Material::Material()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(colorMapAttribute, "colorMap", "")
		, INIT_RESOURCE_ATTRIBUTE(normalMapAttribute,"normalMap", "")
		, INIT_RESOURCE_ATTRIBUTE(envShadowMapAttribute, "envShadowMap", "")
		, INIT_RESOURCE_ATTRIBUTE(colorAttribute, "color", util::Color(1.0))
		, INIT_RESOURCE_ATTRIBUTE(blendFuncAttribute, "blendFunc", util::DynArray<util::Str8>({"", ""}))
		, blendFunc(defaultBlendFunc){
	textures.resize(TexType_Last);
	for (auto &m : textures){
		m= nullptr;
	}

	colorMapAttribute.setOnChangeCallback([=] () {
		if (getResourceState() != State::Uninit)
			changeTexture(TexType_Color, colorMapAttribute.get());
	});
	
	normalMapAttribute.setOnChangeCallback([=] () {
		if (getResourceState() != State::Uninit)
			changeTexture(TexType_Normal, normalMapAttribute.get());
	});
	
	envShadowMapAttribute.setOnChangeCallback([=] () {
		if (getResourceState() != State::Uninit)
			changeTexture(TexType_EnvShadow, envShadowMapAttribute.get());
	});
	
	blendFuncAttribute.setOnChangeCallback([=] () {
		cacheBlendFunc();
	});

}

void Material::setTexture(TexType type, const Texture* tex){
	ensure(type < textures.size());
	textures[type]= tex;

	bool good= true;
	for (const auto& t : textures){
		if (t == nullptr)
			continue;
		if (t->getResourceState() == resources::Resource::State::Error ||
			t->isResourceObsolete()){
			good= false;
			break;
		}
	}
	if (good)
		setResourceState(State::Loaded);
	else
		setResourceState(State::Error);
}

const Texture* Material::getTexture(TexType type) const {
	ensure(type < textures.size());
	//print(debug::Ch::General, debug::Vb::Trivial, "type: %i, size: %i, this: %i", type, textures.size(), this);
	return textures[type];
}

void Material::resourceUpdate(bool load, bool force){
	
	if (load || getResourceState() == State::Uninit){
		
		util::DynArray<util::Str8> tex_names= { colorMapAttribute.get(),
									normalMapAttribute.get(),
									envShadowMapAttribute.get() };


		for (uint32 i=0; i<tex_names.size(); ++i){

			if (tex_names[i].empty()){
				textures[i]= nullptr;
			}
			else {
				textures[i]= &global::g_env.resCache->getResource<Texture>(tex_names[i]);
			}
		}

		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void Material::createErrorResource(){
	setResourceState(State::Error);

	for (auto& m : textures){
		m= nullptr;
	}
	
	textures[TexType_Color]= &global::g_env.resCache->getErrorResource<Texture>();
}

uint32 Material::getContentHash() const {
	uint32 hash= util::hash32(getColor());
	for (SizeType i= 0; i < textures.size(); ++i){
		const Texture* tex= textures[i];
		if (tex)
			hash += tex->getContentHash()*(i + 1);
	}
	hash += blendFunc.srcFactor*(blendFunc.dstFactor + 100);
	return hash;
}

void Material::changeTexture(TexType type, const util::Str8& resname){
	if (resname.empty())
		setTexture(type, nullptr);
	else
		setTexture(type, &global::g_env.resCache->getResource<Texture>(resname));
}

void Material::cacheBlendFunc(){
	auto& factors= blendFuncAttribute.get();
	if (factors.size() == 2){
		blendFunc.srcFactor= hardware::gGlState->asConstant(factors[0], defaultBlendFunc.srcFactor);
		blendFunc.dstFactor= hardware::gGlState->asConstant(factors[1], defaultBlendFunc.dstFactor);
	}
	else {
		print(debug::Ch::Resources, debug::Vb::Moderate, "invalid blendFunc attribute for: %s", getIdentifier().cStr());
	}
}

} // visual
} // clover
