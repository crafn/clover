#include "shader.hpp"
#include "debug/debugprint.hpp"
#include "resources/exception.hpp"
#include "hardware/device.hpp"
#include "hardware/glstate.hpp"
#include "global/exception.hpp"
#include "global/file.hpp"
#include "util/math.hpp"
#include "visual/mesh.hpp"
#include "visual/particlemanifold.hpp"

#include <utility>
#include <cstring>

namespace clover {
namespace visual {

Shader::Shader()
		: vertShd(0)
		, fragShd(0)
		, geomShd(0)
		, program(0)
		, versionStr("150 core")
		, compiled(false){
}

Shader::~Shader() {
	clear();	
}

void Shader::setSources(util::Str8 vertex, util::Str8 geometry, util::Str8 fragment){
	vertSource= vertex;
	geomSource= geometry;
	fragSource= fragment;
}

void Shader::setOutputVaryings(util::DynArray<util::Str8> varyings){
	outputVaryings= std::move(varyings);
}

void Shader::setDefine(const char* def, util::Str8 value){
	util::Str8 append= "#define ";
	append += def;
	append += " ";
	append += value;
	append += '\n';

	print(debug::Ch::General, debug::Vb::Trivial, "DEFINE %s", append.cStr());

	vertSource= append + vertSource;
	if (!geomSource.empty())
		geomSource= append + geomSource;
	if (!fragSource.empty())
		fragSource= append + fragSource;

	defines.pushBack(std::pair<util::Str8, util::Str8>(def, value));
}

void Shader::setDefine(const char* def, int32 value){
	setDefine(def, util::Str8::format("%i", value).cStr());
}

void Shader::compile(const util::DynArray<VertexAttribute>& attribs){
	hardware::gGlState->errorCheck("Shader::compile(..): start");

	if (program)
		throw global::Exception("Shader::compile(..): Already compiled");

	util::Str8 prepend= "#version " + versionStr + "\n";

	vertSource= prepend + vertSource;
	if (!geomSource.empty())
		geomSource= prepend + geomSource;
	if (!fragSource.empty())
		fragSource= prepend + fragSource;

	program= glCreateProgram();
	hardware::gGlState->errorCheck("Shader::compile(..): after glCreateProgram");

	vertShd= createShader(vertSource, GL_VERTEX_SHADER);
	if (!geomSource.empty())
		geomShd= createShader(geomSource, GL_GEOMETRY_SHADER);
	if (!fragSource.empty())
		fragShd= createShader(fragSource, GL_FRAGMENT_SHADER);
	hardware::gGlState->errorCheck("Shader::compile(..): after createShader");

	bindAttributes(attribs);

	if (!outputVaryings.empty()){
		// Transform feedback
		util::DynArray<const char*> varying_cstrs;
		for (auto& v : outputVaryings)
			varying_cstrs.pushBack(v.cStr());

		glTransformFeedbackVaryings(
				program,
				varying_cstrs.size(),
				varying_cstrs.data(),
				GL_INTERLEAVED_ATTRIBS);
	}

	glLinkProgram(program);
	GLint link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (!link_status){
		printLinkInfoLog();
		throw global::Exception("Shader::link(): link failed");
	}

	glBindFragDataLocation(program, 0, "f_color");
	compiled= true;
}

void Shader::use(){
	hardware::gGlState->useProgram(program);
}

void Shader::setUniform(const char* name, const uint8& value, int32 count){
	ensure(count == 1);
	glUniform1ui(uniformLoc(name), value);
}

void Shader::setUniform(const char* name, const uint16& value, int32 count){
	ensure(count == 1);
	glUniform1ui(uniformLoc(name), value);
}

void Shader::setUniform(const char* name, const uint32& value, int32 count){
	glUniform1uiv(uniformLoc(name), count, &value);
}

void Shader::setUniform(const char* name, const int& value, int32 count){
	glUniform1iv(uniformLoc(name), count, &value);
}

void Shader::setUniform(const char* name, const real32& value, int32 count, int32 dim){
	if (dim == 1)
		glUniform1fv(uniformLoc(name), count, &value);
	else if(dim == 2)
		glUniform2fv(uniformLoc(name), count, &value);
	else if(dim == 3)
		glUniform3fv(uniformLoc(name), count, &value);
	else if (dim == 4)
		glUniform4fv(uniformLoc(name), count, &value);
	else throw global::Exception("Shader::setUniform(..): invalid dimensions: %i", dim);
}

void Shader::setUniform(const char* name, const real64& value, int32 count){
	ensure(count == 1);
	real32 v= value;
	glUniform1fv(uniformLoc(name), count, &v);
}

void Shader::setUniform(const char* name, const util::Vec2d& value, int32 count){
	real32 values[count*2];

	for (int32 i=0; i<count; i++){
		values[i*2]= value.x;
		values[i*2+1]= value.y;
	}

	glUniform2fv(uniformLoc(name), count, values);
}

void Shader::setUniform(const char* name, const util::Vec2f& value, int32 count){
	real32 values[count*2];

	for (int32 i=0; i<count; i++){
		values[i*2]= value.x;
		values[i*2+1]= value.y;
	}

	glUniform2fv(uniformLoc(name), count, values);
}

void Shader::setUniform(const char* name, const util::Vec3f& value, int32 count){
	real32 values[count*3];

	for (int32 i=0; i<count; i++){
		values[i*3]= value.x;
		values[i*3+1]= value.y;
		values[i*3+2]= value.z;
	}
	
	glUniform3fv(uniformLoc(name), count, values);
}


void Shader::setUniform(const char* name, const util::Vec4f& value, int32 count){
	real32 values[count*4];

	for (int32 i=0; i<count; i++){
		values[i*4]= value.x;
		values[i*4+1]= value.y;
		values[i*4+2]= value.z;
		values[i*4+3]= value.w;
	}

	glUniform4fv(uniformLoc(name), count, values);
}

void Shader::setUniform(const char* name, const util::Color& value, int32 count){
	real32 values[count*util::Color::size()];

	for (int32 i=0; i<count; i++){
		values[i*util::Color::size()]= value.r;
		values[i*util::Color::size()+1]= value.g;
		values[i*util::Color::size()+2]= value.b;
		values[i*util::Color::size()+3]= value.a;
	}

	glUniform4fv(uniformLoc(name), count, values);
}

void Shader::setUniform(const char* name, const util::Mat33f& value, int32 count){
	glUniformMatrix3fv(uniformLoc(name), count, GL_FALSE, value.data());
}

void Shader::setTexture(hardware::GlState::TexTarget target,
						const char* name,
						uint32 tex,
						int32 slot){
	hardware::gGlState->bindTex(target, tex, slot);
	setUniform(name, slot);
}


void Shader::clear(){
	defines.clear();

	if (program)
		glDeleteProgram(program);

	if (vertShd)
		glDeleteShader(vertShd);

	if (fragShd)
		glDeleteShader(fragShd);

	if (geomShd)
		glDeleteShader(geomShd);

	program= 0;
	vertShd= 0;
	fragShd= 0;
	geomShd= 0;
}

uint32 Shader::createShader(util::Str8 &source, uint32 type){
	uint32 shd= glCreateShader(type);

	const GLchar* source_ch= source.cStr();

	GLint length= -1;
	glShaderSource(shd, 1, &source_ch, &length);

	glCompileShader(shd);

	GLint status;
	glGetShaderiv(shd, GL_COMPILE_STATUS, &status);

	if (!status){
		printInfoLog(shd);

		std::string type_str;
		if (type == GL_VERTEX_SHADER)			type_str= "vertex";
		else if (type == GL_GEOMETRY_SHADER)	type_str= "geometry";
		else									type_str= "fragment";
		
		throw resources::ResourceException("Shader::compile(): %s shader didn't compile", type_str.c_str());
	}

	glAttachShader(program, shd);

	return shd;
}

void Shader::bindAttributes(const util::DynArray<VertexAttribute>& attribs){
	hardware::gGlState->errorCheck("Shader::bindAttributes(..): before binds");

	uint32 id_count= 0;
	for(auto& m : attribs){
		glBindAttribLocation(program, id_count, m.name.cStr());
		//print(debug::Ch::General, debug::Vb::Trivial, "Bind %i %s", id_count, m.name.cStr());
		hardware::gGlState->errorCheck("Shader::bindAttributes(..):");

		ensure_msg(m.arraySize == 1,
				"Last time arrays didn't work as expected. "
				"Be warned");

		id_count += m.arraySize;
	}
}

uint32 Shader::uniformLoc(const char* name){
	auto it= nameToUniform.find(name);
	if (it != nameToUniform.end())
		return it->second;

	ensure(nameToUniform.size() < 1000 &&
			"Don't pass dynamically allocated string to Shader::setUniform!");

	return nameToUniform[name]= glGetUniformLocation(program, name);
}

void Shader::printInfoLog(uint32 shader) const {
	const GLsizei maxLength= 512;
	GLchar infoLog[maxLength];

	glGetShaderInfoLog(shader, maxLength, nullptr, infoLog);
	print(debug::Ch::OpenGL, debug::Vb::Critical, "Shader build log:\n%s", infoLog);
}

void Shader::printLinkInfoLog() const {
	const GLsizei size= 512;
	GLchar info_log[size];

	glGetProgramInfoLog(program, size, nullptr, info_log);
	print(debug::Ch::OpenGL, debug::Vb::Critical, "Shader link log:\n%s", info_log);
}

} // visual
} // clover
