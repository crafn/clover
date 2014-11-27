#include "hardware/glstate.hpp"
#include "global/cfg_mgr.hpp"
#include "util/string.hpp"

// Use this when wondering where error occurs
#define USE_SUPER_OGL_DEBUG 0

#if USE_SUPER_OGL_DEBUG
#define SUPER_OGL_DEBUG(str) \
	gGlState->errorCheck(str);
#else
#define SUPER_OGL_DEBUG(str)
#endif

namespace clover {
namespace hardware {

GlState *gGlState;

#if 0
static void debugCallback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		void* custom){

	if (	severity == GL_DEBUG_SEVERITY_MEDIUM ||
			severity == GL_DEBUG_SEVERITY_HIGH){
		print(debug::Ch::OpenGL, debug::Vb::Critical, "OpenGL msg: %s", message);
	}
}
#endif

GlState::GlState()
		: boundFbo((FboDId)-1)
		, boundVao((VaoDId)-1)
		, boundVbo((VboDId)-1)
		, boundIbo((IboDId)-1)
		, activeProgram(0){

	activeTexUnit=0;
	polygonMode= GL_FILL;

	for (int32 i=0; i<texUnitCount; i++){
		boundTex[i]= 0;
		boundTexTargets[i]= TexTarget::Tex2d;
	}
	blendSource= 0;
	blendDestination= 0;
	
	/// @todo OGL debug messages
#if 0
	if (global::gCfgMgr->get("hardware::checkGlErrors", false)){
		print(debug::Ch::General, debug::Vb::Trivial, "CHECK");
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallbackARB(debugCallback, nullptr);	
	}
#endif
}

GlState::TexDId GlState::genTex(){
	SUPER_OGL_DEBUG("genTex begin");

	uint32 texture;
	glGenTextures(1, &texture);

	SUPER_OGL_DEBUG("genTex end");
	return texture;
}

void GlState::setTextureUnit(int32 unit){
	SUPER_OGL_DEBUG("setTextureUnit begin");

	ensure(unit < texUnitCount);
	if (unit != activeTexUnit){
		activeTexUnit= unit;

		glActiveTexture(unit + GL_TEXTURE0);
	}

	SUPER_OGL_DEBUG("setTextureUnit end");
}

void GlState::bindTex(TexTarget target, TexDId tex, int32 unit){
	SUPER_OGL_DEBUG("bindTex begin");

	if (unit == -1){
		unit= activeTexUnit;
		ensure(unit < texUnitCount);
	}
	
	if (tex != boundTex[unit]){
		if (unit != activeTexUnit)
			setTextureUnit(unit);
		
		boundTexTargets[unit]= target;
		glBindTexture((GLenum)target, tex);
		boundTex[unit]= tex;
	}

	SUPER_OGL_DEBUG("bindTex end");
}

void GlState::setPolygonMode(GLenum mode){
	SUPER_OGL_DEBUG("setPolygonMode begin");

	if (mode != polygonMode){

		polygonMode= mode;

		glPolygonMode(GL_FRONT_AND_BACK, mode);

	}

	SUPER_OGL_DEBUG("setPolygonMode end");
}

void GlState::setBlendFunc(BlendFunc func){
	SUPER_OGL_DEBUG("setPolygonMode begin");

	if (func.srcFactor != blendSource || func.dstFactor != blendDestination){
		glBlendFunc(func.srcFactor, func.dstFactor);

		blendSource= func.srcFactor;
		blendDestination= func.dstFactor;
	}

	SUPER_OGL_DEBUG("setPolygonMode end");
}

void GlState::setViewport(util::Vec2i pos, util::Vec2i size){
	SUPER_OGL_DEBUG("setViewport begin");

	if (pos != viewportPosition || size != viewportSize){
		glViewport(pos.x, pos.y, size.x, size.y);
		viewportPosition= pos;
		viewportSize= size;
	}

	SUPER_OGL_DEBUG("setViewport end");
}

template <>
void GlState::setTexParam<int32>(GLenum e, const int32& v){
	SUPER_OGL_DEBUG("setTexParam<int32> begin");

	glTexParameteri(activeGlTexTarget(), e, v);

	SUPER_OGL_DEBUG("setTexParam<int32> end");
}

template <>
void GlState::setTexParam<real32>(GLenum e, const real32& v){
	SUPER_OGL_DEBUG("setTexParam<real32> begin");

	glTexParameterf(activeGlTexTarget(), e, v);

	SUPER_OGL_DEBUG("setTexParam<real32> end");
}

template <>
void GlState::setTexParam<real64>(GLenum e, const real64& v){
	SUPER_OGL_DEBUG("setTexParam<real64> begin");

	glTexParameterf(activeGlTexTarget(), e, v);

	SUPER_OGL_DEBUG("setTexParam<real64> end");
}

void GlState::setDefaultTexParams(){
	SUPER_OGL_DEBUG("setDefaultTexParams begin");

	setTexParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	setTexParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	setTexParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
	setTexParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	setTexParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SUPER_OGL_DEBUG("setDefaultTexParams end");
}

void GlState::submitTexData(TexFormat format,
							Type datatype,
							util::Vec2i size,
							const void* data,
							int32 miplevel){
	submitTexData(	format,
					datatype,
					util::Vec3i{size.x, size.y, 1},
					data,
					miplevel);
}

void GlState::submitTexData(TexFormat format,
							Type datatype,
							util::Vec3i size,
							const void* data,
							int miplevel){
	SUPER_OGL_DEBUG("submitTexData begin");

	if (boundTexTargets[activeTexUnit] == TexTarget::Tex2d){
		glTexImage2D(	activeGlTexTarget(),
						miplevel,
						(GLint)format,
						size.x,
						size.y,
						0,
						defaultGlLayout(format),
						(GLenum)datatype,
						(const GLvoid*) data);
	}
	else {
		glTexImage3D(	activeGlTexTarget(),
						miplevel,
						(GLint)format,
						size.x,
						size.y,
						size.z,
						0,
						defaultGlLayout(format),
						(GLenum)datatype,
						(const GLvoid*) data);
	}

	SUPER_OGL_DEBUG("submitTexData end");
}

void GlState::generateMipmap(){
	SUPER_OGL_DEBUG("generateMipmap begin");

	glGenerateMipmap(activeGlTexTarget());

	SUPER_OGL_DEBUG("generateMipmap end");
}


void GlState::deleteTex(uint32 tex){
	SUPER_OGL_DEBUG("deleteTexture begin");

	ensure_msg(boundTex[activeTexUnit] != tex, "todo automatic unbind");
	glDeleteTextures(1, &tex);

	SUPER_OGL_DEBUG("deleteTexture end");
}

GlState::FboDId GlState::genFbo(){
	SUPER_OGL_DEBUG("genFbo begin");

	uint32 fbo;
	glGenFramebuffers(1, &fbo);

	SUPER_OGL_DEBUG("genFbo end");
	return fbo;
}

void GlState::bindFbo(FboDId fbo){
	SUPER_OGL_DEBUG("bindFramebuffer begin");

	if (fbo == boundFbo)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	boundFbo= fbo;

	SUPER_OGL_DEBUG("bindFramebuffer end");
}

void GlState::attachTexToFbo(GLenum attachment, TexDId tex, int32 miplevel){
	SUPER_OGL_DEBUG("attachTexToFbo begin");

	glFramebufferTexture(	GL_FRAMEBUFFER,
							attachment,
							tex,
							miplevel);

	SUPER_OGL_DEBUG("attachTexToFbo end");
}

GLenum GlState::getFboStatus(){
	SUPER_OGL_DEBUG("getFboStatus begin");

	return glCheckFramebufferStatus(GL_FRAMEBUFFER);	
}

void GlState::deleteFbo(FboDId fbo){
	SUPER_OGL_DEBUG("deleteFbo begin");

	glDeleteFramebuffers(1, &fbo);

	SUPER_OGL_DEBUG("deleteFbo end");
}

void GlState::setClearColor(const util::Color& c){
	SUPER_OGL_DEBUG("setClearColor begin");

	glClearColor(c.r, c.g, c.b, c.a);

	SUPER_OGL_DEBUG("clearColor end");
}

void GlState::clear(GLbitfield mask){
	SUPER_OGL_DEBUG("clear begin");

	glClear(mask);

	SUPER_OGL_DEBUG("clear end");
}

void GlState::errorCheck(const util::Str8& where){
	if (!global::gCfgMgr->get<bool>("hardware::checkGlErrors", true))
		return;

	uint32 err= glGetError();


	if (err != GL_NO_ERROR){
		util::Str8 msg= "OpenGL error: " + where;
		msg += ": ";


		std::stringstream tc;
		tc << err;



		util::Str8 error= tc.str().c_str();
		error += " ";
		error += (const char*)gluErrorString(err);

		msg +=	error;

		print(debug::Ch::OpenGL, debug::Vb::Moderate, msg.cStr());

	}
}

void GlState::genVertexArrays(uint32 count, VaoDId* vao){
	SUPER_OGL_DEBUG("genVertexArrays begin");

	debug_ensure(glGenVertexArrays != nullptr);
	glGenVertexArrays(count, vao);

	SUPER_OGL_DEBUG("genVertexArrays end");
}

void GlState::genVertexBuffers(uint32 count, VboDId* vbo){
	SUPER_OGL_DEBUG("genVertexBuffers begin");

	glGenBuffers(count, vbo);

	SUPER_OGL_DEBUG("genVertexBuffers end");
}

void GlState::genIndexBuffers(uint32 count, IboDId* ibo){
	SUPER_OGL_DEBUG("genIndexBuffers begin");

	glGenBuffers(count, ibo);

	SUPER_OGL_DEBUG("genIndexBuffers end");
}

void GlState::bindVertexArray(VaoDId id){
	SUPER_OGL_DEBUG("bindVertexArray begin");

	if (id == boundVao)
		return;

	glBindVertexArray(id);
	boundVao= id;

	SUPER_OGL_DEBUG("bindVertexArray end");
}

void GlState::bindVertexBuffer(VboDId id){
	SUPER_OGL_DEBUG("bindVertexBuffer begin");

	if (id == boundVbo)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, id);
	boundVbo= id;

	SUPER_OGL_DEBUG("bindVertexBuffer end");
}

void GlState::bindIndexBuffer(IboDId id){
	SUPER_OGL_DEBUG("bindIndexBuffer begin");

	if (id == boundIbo)
		return;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	boundIbo= id;

	SUPER_OGL_DEBUG("bindIndexBuffer end");
}

void GlState::vertexAttribPointer(uint32 i, uint32 count, Type type, bool normalized, uint32 vertexSize, uint32 offset_in_bytes){
	SUPER_OGL_DEBUG("vertexAttribPointer begin");

	if (type == Type::Real32 || normalized){
		glVertexAttribPointer(
				i,
				count,
				(GLenum)type,
				normalized,
				vertexSize,
				BUFFER_OFFSET(offset_in_bytes));
	}
	else {
		// Integer value
		glVertexAttribIPointer(
				i,
				count,
				(GLenum)type,
				vertexSize,
				BUFFER_OFFSET(offset_in_bytes));
	}

	SUPER_OGL_DEBUG("vertexAttribPointer end");
}

void GlState::enableVertexAttribArray(uint32 i, bool enable){
	SUPER_OGL_DEBUG("enableVertexAttribArray begin");

	if (enable)
		glEnableVertexAttribArray(i);
	else
		glDisableVertexAttribArray(i);

	SUPER_OGL_DEBUG("enableVertexAttribArray end");
}

void GlState::submitBufferData(VaoBufferType type, uint32 byte_count, const void* data, VaoUsage usage){
	SUPER_OGL_DEBUG("submitBufferData begin");

	glBufferData((GLenum)type, byte_count, data, (GLenum)usage);

	SUPER_OGL_DEBUG("submitBufferData end");
}


void GlState::overwriteBufferData(VaoBufferType type, uint32 byte_offset, uint32 byte_count, const void* data){
	SUPER_OGL_DEBUG("overwriteBufferData begin");

	glBufferSubData((GLenum)type, byte_offset, byte_count, data);

	SUPER_OGL_DEBUG("overwriteBufferData end");
}

void GlState::drawElements(Primitive p, uint32 draw_index_offset, uint32 draw_index_count, Type index_type, uint32 max_vertex_index){
	SUPER_OGL_DEBUG("drawElements begin");

	glDrawRangeElements((GLenum)p, 0, max_vertex_index, draw_index_count, (GLenum)index_type, BUFFER_OFFSET(draw_index_offset*getSize(index_type)));

	SUPER_OGL_DEBUG("drawElements end");
}

void GlState::drawArrays(Primitive p, uint32 draw_index_offset, uint32 draw_index_count){
	SUPER_OGL_DEBUG("drawArrays begin");

	glDrawArrays((GLenum)p, draw_index_offset, draw_index_count);

	SUPER_OGL_DEBUG("drawArrays end");
}

void GlState::deleteVertexArrays(uint32 count, VaoDId* id){
	SUPER_OGL_DEBUG("deleteVertexArrays begin");

	glDeleteVertexArrays(count, id);

	SUPER_OGL_DEBUG("deleteVertexArrays end");
}

void GlState::deleteVertexBuffers(uint32 count, VboDId* id){
	SUPER_OGL_DEBUG("deleteVertexBuffers begin");

	glDeleteBuffers(count, id);

	SUPER_OGL_DEBUG("deleteVertexBuffers end");
}

void GlState::deleteIndexBuffers(uint32 count, IboDId* id){
	SUPER_OGL_DEBUG("deleteIndexBuffers begin");

	glDeleteBuffers(count, id);

	SUPER_OGL_DEBUG("deleteIndexBuffers end");
}

void GlState::useProgram(ProgramDid p){
	SUPER_OGL_DEBUG("useProgram begin");

	if (p != activeProgram){
		glUseProgram(p);
		activeProgram= p;
	}

	SUPER_OGL_DEBUG("useProgram end");
}

void GlState::flush(){
	SUPER_OGL_DEBUG("flush begin");

	glFlush();

	SUPER_OGL_DEBUG("flush end");
}

void GlState::finish(){
	SUPER_OGL_DEBUG("finish begin");

	glFinish();

	SUPER_OGL_DEBUG("finish end");
}

uint32 GlState::asConstant(const util::Str8& constant_str, uint32 error_value) const {
	if (constant_str.empty())
		return error_value;

	#define GLCONSTANT(x)\
		if (constant_str == #x){ return x; }
	#include "gl_constants.def"
	#undef GLCONSTANT

	return error_value;
}

GLenum GlState::activeGlTexTarget() const {
	ensure(activeTexUnit >= 0 && activeTexUnit < texUnitCount);
	return (GLenum)boundTexTargets[activeTexUnit];
}

GLenum GlState::defaultGlLayout(TexFormat format) const {
	static util::Map<TexFormat, GLenum> format_to_layout= {
		{ TexFormat::Rgb8, GL_RGB },
		{ TexFormat::Rgba8, GL_RGBA },
		{ TexFormat::Rgb16, GL_RGB },
		{ TexFormat::Rgba16, GL_RGBA },
		{ TexFormat::Depth, GL_DEPTH_COMPONENT }
	};

	auto it= format_to_layout.find(format);
	ensure(it != format_to_layout.end());
	return it->second;
}

} // hardware
} // clover
