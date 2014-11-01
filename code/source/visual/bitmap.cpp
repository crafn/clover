#include "visual/bitmap.hpp"
#include "debug/debugprint.hpp"
#include "hardware/glstate.hpp"

namespace clover {
namespace visual {

Bitmap::Bitmap(util::Vec2i size):
	buffer(0),
	bufLen(0){

	if (size.x != 0 && size.y != 0){
		create(size);
	}
}

Bitmap::~Bitmap(){
	if (buffer)
		destroy();
}

void Bitmap::create(util::Vec2i size){
	dim.x= size.x;
	dim.y= size.y;

	bufLen= 4*size.x*size.y;
	buffer= new uint8[bufLen];

	for (int32 i=0; i<bufLen; i++){
		if (i%4 == 3)
			buffer[i]=0;
		else
			buffer[i]= 255;
	}
}

void Bitmap::blitBW(const uint8 * rect, util::Vec2i size, util::Vec2i pos, bool font_conversion){
	for (int32 y=0; y<size.y; y++){
		for (int32 x=0; x<size.x; x++){
			if (x + pos.x < 0 || x + pos.x >= dim.x ||
				y + pos.y < 0 || y + pos.y >= dim.y) continue;

			int32 bufIndex = 4*((y+pos.y)*dim.x + pos.x + x);
			int32 rectIndex= (size.y-y-1)*size.x + x;

			if (!font_conversion){
				// RGB
				buffer[bufIndex]= buffer[bufIndex+1]= buffer[bufIndex+2]= rect[rectIndex];
				// Alpha
				buffer[bufIndex+3]= 255;
			}
			else {

				buffer[bufIndex]= buffer[bufIndex+1]= buffer[bufIndex+2]= 255;
				buffer[bufIndex+3]= rect[rectIndex];

			}
		}
	}
}

Texture Bitmap::createTexture(bool font_mode){
	Texture tex;
	tex.bind();

	if (font_mode){
		hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_S, GL_CLAMP);
		hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_T, GL_CLAMP);

		hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		hardware::gGlState->submitTexData(	hardware::GlState::TexFormat::Rgba8,
									hardware::GlState::Type::Uint8,
									dim,
									buffer);
	}
	else {
		hardware::gGlState->setDefaultTexParams();
		hardware::gGlState->submitTexData(	hardware::GlState::TexFormat::Rgba8,
									hardware::GlState::Type::Uint8,
									dim,
									buffer);
		hardware::gGlState->generateMipmap();
	}

	return (tex);
}

void Bitmap::destroy(){
	if (buffer == 0)return;

	delete [] buffer;
	bufLen=0;
	buffer=0;
}

} // visual
} // clover