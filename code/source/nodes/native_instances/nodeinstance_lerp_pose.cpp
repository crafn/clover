#include "nodeinstance_lerp_pose.hpp"
#include "animation/armaturepose.hpp"

namespace clover {
namespace nodes {

CompNode* PoseLerpNodeInstance::compNode()
{
	auto n= new CompNode{};
	n->addInputSlot("input1", SignalType::ArmaturePose);
	n->addInputSlot("input2", SignalType::ArmaturePose);
	n->addInputSlot("factor", SignalType::Real, 0.0);
	n->addInputSlot("limitFactor", SignalType::Boolean, true);
	n->addOutputSlot("result", SignalType::ArmaturePose);
	return n;
}

void PoseLerpNodeInstance::create(){
	input1= addInputSlot<SignalType::ArmaturePose>("input1");
	input2= addInputSlot<SignalType::ArmaturePose>("input2");
	factorIn= addInputSlot<SignalType::Real>("factor");
	limitFactorIn= addInputSlot<SignalType::Boolean>("limitFactor");
	
	resultOut= addOutputSlot<SignalType::ArmaturePose>("result");
	
	auto cb= [&] (){ setUpdateNeeded(); };
	input1->setOnReceiveCallback(cb);
	input2->setOnReceiveCallback(cb);
	factorIn->setOnReceiveCallback(cb);
	
	setUpdateNeeded(false);
}

void PoseLerpNodeInstance::update(){
	try {
		real64 blend_factor= factorIn->get();
		
		if (limitFactorIn->get())
			blend_factor= util::limited(blend_factor, 0.0, 1.0);
		
		auto pose1= input1->get().getLocalInBindPose();
		auto pose2= input2->get().getLocalInBindPose();

		animation::ArmaturePose::Pose result_pose;

		if (!pose1.empty() && !pose2.empty()){
			result_pose= lerp(pose1, pose2, blend_factor);
		}
		else { // Poses not delivered properly to inputs
			if (!pose1.empty())
				result_pose= pose1;
			else if (!pose2.empty())
				result_pose= pose2;
		}

		if (!result_pose.empty()){	
			resultOut->send(
				animation::ArmaturePose(input1->get().getArmature(), std::move(result_pose))
			);
		}
	}
	catch (global::Exception& e){
		print(debug::Ch::Nodes, debug::Vb::Moderate, "ArmaturePose lerp failed: %s", e.what());
	}
	
	setUpdateNeeded(false);
}

} // nodes
} // clover
