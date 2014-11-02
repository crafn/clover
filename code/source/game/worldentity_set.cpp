#include "worldentity_set.hpp"
#include "worldentity.hpp"

namespace clover {
namespace game {

WESet::WESet(){
}

WESet::~WESet(){

}

WESet::Iter WESet::begin(){
	return handles.begin();
}

WESet::Iter WESet::end(){
	return handles.end();
}

WESet::cIter WESet::begin() const {
	return handles.begin();
}

WESet::cIter WESet::end() const {
	return handles.end();
}

int32 WESet::size(){
	return handles.size();
}

void WESet::clear(){
	handles.clear();
}

void WESet::add(const game::WorldEntity& we){
	handles.pushBack(game::WeHandle(&we));
}

void WESet::add(const game::WeHandle& h){
	handles.pushBack(h);
}

void WESet::add(const WESet& s){
	handles.insert(handles.end(), s.begin(), s.end());
}

void WESet::remove(const game::WeHandle& h){
	handles.remove(h);
}

} // game
} // clover