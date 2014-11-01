#include "nodeinstance_cameratarget.hpp"

namespace clover {
namespace nodes {

util::LinkedList<CameraTargetNodeInstance*> CameraTargetNodeInstance::targets;

CameraTargetNodeInstance::~CameraTargetNodeInstance(){
	targets.erase(iterator);
}

void CameraTargetNodeInstance::create(){
	targets.pushBack(this);
	iterator= std::prev(targets.end());
	
	nameIn= addInputSlot<SignalType::String>("name");
	transformIn= addInputSlot<SignalType::SrtTransform2>("transform");
}

const CameraTargetNodeInstance* CameraTargetNodeInstance::findTarget(const util::Str8& name){
	for (const auto& t : targets){
		if (t->getName() == name)
			return t;
	}
	return nullptr;
}

} // nodes
} // clover