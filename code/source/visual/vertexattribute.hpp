#ifndef CLOVER_VISUAL_VERTEXATTRIBUTE_HPP
#define CLOVER_VISUAL_VERTEXATTRIBUTE_HPP

#include "build.hpp"
#include "hardware/glstate.hpp"
#include "util/string.hpp"

namespace clover {
namespace visual {

struct VertexAttribute {
	/// Offset from beginning of the struct (in bytes)
	uint32 offset= 0;
	
	/// Type of a single element (e.g. hardware::GlState::Type::Real32 for util::Vec2f)
	hardware::GlState::Type elemType= hardware::GlState::Type::Real32;
	
	/// Numer of elements (e.g. 2 for util::Vec2f)
	uint32 elemCount= 0;
	
	/// Number of values in an array (e.g. 1 for util::Vec2f)
	uint32 arraySize= 1;

	/// @see OpenGL spec for glVertexAttribPointer
	bool normalized= false;

	/// Name in shader
	util::Str8 name;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_VERTEXATTRIBUTE_HPP
