#include "cfg_mgr.hpp"

#include <fstream>

namespace clover {
namespace global {

CfgMgr* gCfgMgr= nullptr;

CfgMgr::CfgMgr(){
	load();
}

CfgMgr::~CfgMgr(){
}

void CfgMgr::load(){
	root= util::ObjectNode(util::ObjectNode::Value::Object);

	std::ifstream file(findCfgFilePath().cStr(), std::ios::binary);
	if (!file){
		print(debug::Ch::General, debug::Vb::Moderate, "No config file found!");
		return;
	}
	
	std::string content( 	(std::istreambuf_iterator<char>(file)),
							(std::istreambuf_iterator<char>()));
	
	try {
		root.parseText(content);
	}
	catch (...){
		print(debug::Ch::General, debug::Vb::Critical, "Settings loading failed");
	}
	
	createVars();
}

void CfgMgr::save(){
	util::Str8 path= findCfgFilePath();
	print(debug::Ch::General, debug::Vb::Trivial, "Saving cfg to %s", path.cStr());
	std::ofstream file(path.cStr(), std::ios::binary);
	file << root.generateText().cStr();
}

void CfgMgr::registerVarsToScript(){
	for (auto& pair : vars){
		auto& var = pair.second;
		var.tryRegisterToScript();
	}
}

void CfgMgr::createVars(){
	for (SizeType n_i= 0; n_i < root.size(); ++n_i){
		util::ObjectNode name_space= std::move(root[root.getMemberNames()[n_i]]);
		ensure(!name_space.isNull());
		
		for (SizeType v_i = 0; v_i < name_space.size(); ++v_i){
			util::Str8 field_key= name_space.getMemberNames()[v_i];
			util::Str8 var_name= root.getMemberNames()[n_i] + "::" + field_key;
			
			util::ObjectNode ob_value= std::move(name_space[field_key]);
			
			auto var_pair_it= vars.find(var_name);
			if (var_pair_it == vars.end()){
				// Create new CVar if CVar with same name doesn't exist
				vars.insert(std::make_pair(var_name, CVar(var_name, std::move(ob_value))));
			}
			else {
				// Update the value of CVar if it exists already (happens when reloading)
				var_pair_it->second.setObjectNode(std::move(ob_value));
			}

		}
	}
}

util::Str8 CfgMgr::findCfgFilePath() const {
	for (SizeType i= 0; i < defaultCfgFilePaths().size(); ++i){
		util::Str8 path= defaultCfgFilePaths()[i];
		if (std::ifstream(path.cStr()))
			return path;
	}
	
	// No file found, return path near binary so that creating the file is not a problem
	/// @todo Make installer friendly
	return defaultCfgFilePaths().front();
}

} // global
} // clover