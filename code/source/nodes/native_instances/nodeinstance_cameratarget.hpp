#ifndef CLOVER_NODES_NODEINSTANCE_CAMERATARGET_HPP
#define CLOVER_NODES_NODEINSTANCE_CAMERATARGET_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

/// @todo Replace with util
#include <boost/optional.hpp>

namespace clover {
namespace nodes {

class CameraTargetNodeInstance : public NodeInstance {
public:
	static CompNode* compNode();

	CameraTargetNodeInstance()= default;
	CameraTargetNodeInstance(CameraTargetNodeInstance&&)= delete;
	virtual ~CameraTargetNodeInstance();
	
	virtual void create();
	const util::Str8& getName() const { return nameIn->get(); }
	const util::SrtTransform2d& getTransform() const { return transformIn->get(); }
	
	static const CameraTargetNodeInstance* findTarget(const util::Str8& name);

private:
	InputSlot<SignalType::String>* nameIn;
	InputSlot<SignalType::SrtTransform2>* transformIn;
	
	util::LinkedList<CameraTargetNodeInstance*>::Iter iterator;
	static util::LinkedList<CameraTargetNodeInstance*> targets;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::CameraTargetNodeInstance> {
	static util::Str8 type(){ return "::CameraTargetNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_CAMERATARGET_HPP
