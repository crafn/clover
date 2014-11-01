#include "texture.hpp"
#include "hardware/glstate.hpp"
#include "resources/exception.hpp"

#include <GL/glew.h>

namespace clover {
namespace resources {

bool ResourceTraits<visual::Texture>::expired(const visual::Texture& tex){
	return tex.getStaleness() > 60*15;
} // About 15 seconds

} // resources
namespace visual {

Texture::Texture():
	mipMapsLoaded(false),
	INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", ""),
	INIT_RESOURCE_ATTRIBUTE(fileAttribute, "file", ""){
	
		
	fileAttribute.setOnChangeCallback([&] () {
		if (getResourceState() != State::Uninit)
			onFileChange();
	});
		
	fileAttribute.get().setOnFileChangeCallback([&] (resources::PathAttributeValue::FileEvent f){
		if (getResourceState() != State::Uninit){
			onFileChange();
		}
	});
		
	id= hardware::gGlState->genTex();

}

Texture::Texture(Texture&& other):
		Resource(std::move(other)),
		id(0),
		mipMapsLoaded(false),
		dimensions(other.dimensions),
		nameAttribute(std::move(other.nameAttribute)),
		fileAttribute(std::move(other.fileAttribute)){
			
	id= other.id;
	external= other.external;
	mipMapsLoaded= other.mipMapsLoaded;
	other.id= 0;
	other.mipMapsLoaded= false;
}

Texture& Texture::operator=(Texture&& other){
	Resource::operator=(std::move(other));
	id= other.id;
	external= other.external;
	mipMapsLoaded= other.mipMapsLoaded;
	dimensions= other.dimensions;

	other.id= 0;
	other.mipMapsLoaded= false;

	return *this;
}

Texture::~Texture(){
	unload();
}

void Texture::resourceUpdate(bool load, bool force){
	util::Str8 path= fileAttribute.get().whole();

	try {
		if (getResourceState() == State::Uninit){
			// Blocking load when loaded first time
			image.load(path);
			this->submit();
			image.clear();

			setResourceState(State::Loaded);
		}
		else {
			if (load){
				// Load
				if (getResourceState() != State::Loading){
					// Start loading
					image.loadAsync(path); // Could maybe use std::future
					setResourceState(State::Loading);
				}
				else {
					try {
						if (image.isLoaded()){
							// Image is loaded, submitting
							this->submit();
							
							// Free the image
							image.clear();
							
							setResourceState(State::Loaded);
						}
					}
					catch (...){
						createErrorResource();
					}
				}
			}
			else {
				// Unload
				this->unload(false);
				setResourceState(State::Unloaded);
			}
		}
	}
	catch (const resources::ResourceException& e){
		createErrorResource();
	}
}

void Texture::createErrorResource(){
	image.clear();
	
	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, id);

	hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
	hardware::gGlState->setTexParam(GL_TEXTURE_MAX_LOD, 0);
	hardware::gGlState->setTexParam(GL_TEXTURE_MIN_LOD, 0);
	hardware::gGlState->setTexParam(GL_TEXTURE_LOD_BIAS, 0);
	hardware::gGlState->setTexParam(GL_TEXTURE_BASE_LEVEL, 0);

	const uint32 width= 8, height= 8;

	uint8 image_data[3*width*height];

	// Generate checkerboard
	uint32 pixelcounter= 0;
	bool rowflip= false;
	for (uint32 i=0; i<width*height*3; i+=3){
		if ((i%2 && rowflip) || (i%2 == 0 && !rowflip)){
			image_data[i]= 0;
			image_data[i+1]= 0;
			image_data[i+2]= 0;
		}
		else {
			image_data[i]= 255;
			image_data[i+1]= 0;
			image_data[i+2]= 255;
		}

		++pixelcounter;
		if (pixelcounter == width){
			pixelcounter=0;
			rowflip= !rowflip;
		}
	}

	hardware::gGlState->submitTexData(
			hardware::GlState::TexFormat::Rgb8,
			hardware::GlState::Type::Uint8,
			util::Vec2i{width, height},
			image_data);
	
	setResourceState(State::Error);

}
void Texture::bind(){
	ensure(id);
	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, id);
	onResourceUse();
}

void Texture::setExternal(hardware::GlState::TexDId id_, util::Vec2i size){
	id= id_;
	dimensions= size;
	external= true;
}

uint32 Texture::getContentHash() const {
	return id + util::hash32(nameAttribute.get());
}

void Texture::submit(bool only_mipmaps){
	ensure(image.isLoaded());
	ensure(!external);
	hardware::gGlState->errorCheck("Tex load start");

	// Now generate the OpenGL texture object
	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, id);

	hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
	hardware::gGlState->setTexParam(GL_TEXTURE_MAX_LOD, 1000);
	hardware::gGlState->setTexParam(GL_TEXTURE_MIN_LOD, -1000);
	hardware::gGlState->setTexParam(GL_TEXTURE_LOD_BIAS, -0.6); // Käyttää isompaa mipmappia ku mitä normisti käyttäis => terävät tekstuurit

	dimensions= image.getDimensions();

	if (!only_mipmaps){
		hardware::gGlState->submitTexData(
				image.getColorFormat(),
				image.getDataTypeFormat(),
				image.getDimensions(),
				image.getRawData(),
				0);
		hardware::gGlState->setTexParam(GL_TEXTURE_BASE_LEVEL, 0);
	}
	else {
		hardware::gGlState->submitTexData(
				image.getColorFormat(),
				image.getDataTypeFormat(),
				util::Vec2i(1),
				nullptr,
				0);
		hardware::gGlState->setTexParam(GL_TEXTURE_BASE_LEVEL, 1);
	}

	Image mip;
	hardware::gGlState->errorCheck("Before mipmaps");
	int32 miplevel=1;
	while (miplevel == 1 || mip.getDimensions().x > 1 || mip.getDimensions().y > 1){
		if (mipMapsLoaded) break;

		if (miplevel == 1)
			mip= std::move(image.cheapShrinkedToHalf());
		else
			mip= std::move(mip.cheapShrinkedToHalf());

		hardware::gGlState->submitTexData(
				mip.getColorFormat(),
				mip.getDataTypeFormat(),
				mip.getDimensions(),
				mip.getRawData(),
				miplevel);

		++miplevel;
	}
	hardware::gGlState->errorCheck("After mipmaps");

	mipMapsLoaded= true;
}

void Texture::unload(bool unload_mipmaps_also){
	if (external)
		return;

	if (unload_mipmaps_also){
		hardware::gGlState->deleteTex(id);
		id= hardware::gGlState->genTex();
		mipMapsLoaded= false;
	}
	else {
		ensure(mipMapsLoaded);
		hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, id);

		// Don't free mipmaps
		hardware::gGlState->setTexParam(GL_TEXTURE_BASE_LEVEL, 1);
		hardware::gGlState->submitTexData(hardware::GlState::TexFormat::Rgb8,
								hardware::GlState::Type::Uint8,
								util::Vec2i(1),
								nullptr);

	}
	image.clear();

}

void Texture::onFileChange(){
	this->unload();
	onResourceUse(); // Reset staleness
	setResourceState(State::Unloaded);
}

} // visual
} // clover