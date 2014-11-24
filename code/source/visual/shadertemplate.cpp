#include "shadertemplate.hpp"
#include "global/file.hpp"

namespace clover {
namespace visual {

ShaderTemplate::ShaderTemplate()
	: INIT_RESOURCE_ATTRIBUTE(name, "name", "")
	, INIT_RESOURCE_ATTRIBUTE(vertSrc, "vertSrc", "")
	, INIT_RESOURCE_ATTRIBUTE(fragSrc, "fragSrc", "")
	, INIT_RESOURCE_ATTRIBUTE(geomSrc, "geomSrc", "")
	, INIT_RESOURCE_ATTRIBUTE(options, "options", {})
{
	auto on_change= [&] ()
	{
		if (getResourceState() != State::Uninit)
			resourceUpdate(true);
	};

	auto on_content_change= [&] (resources::PathAttributeValue::FileEvent)
	{
		if (getResourceState() != State::Uninit)
			resourceUpdate(true);
	};

	vertSrc.setOnChangeCallback(on_change);
	vertSrc.get().setOnFileChangeCallback(on_content_change);
	fragSrc.setOnChangeCallback(on_change);
	fragSrc.get().setOnFileChangeCallback(on_content_change);
	geomSrc.setOnChangeCallback(on_change);
	geomSrc.get().setOnFileChangeCallback(on_content_change);
}

void ShaderTemplate::resourceUpdate(bool load, bool force)
{
	if (getResourceState() == State::Uninit || load) {
		vertCode= global::File::readText(vertSrc.get().whole());
		fragCode= global::File::readText(fragSrc.get().whole());
		if (geomSrc.get().relative().length() > 0)
			geomCode= global::File::readText(geomSrc.get().whole());

		setResourceState(State::Loaded);
	} else {
		setResourceState(State::Unloaded);
	}
}

void ShaderTemplate::createErrorResource()
{
}

} // visual
} // clover
