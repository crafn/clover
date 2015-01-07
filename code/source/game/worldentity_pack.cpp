#include "worldentity_pack.hpp"
#include "util/misc.hpp"
#include "global/exception.hpp"
#include "debug/print.hpp"

namespace clover {
namespace game {

WEPack::WEPack()
		: id(0){
}

void WEPack::set(util::RawPack& data_){
	data= data_;
	
	data.setReadMode();
	data.seekg(0);
}

void WEPack::reset(uint64 id_){
	id= id_;
	data.clear();
}

util::RawPack& WEPack::getVarPack(){
	return data;
}

void WEPack::prepareVarBlockReading(){
	data.setReadMode(true);
	data.seekg(0);
}

} // game
} // clover