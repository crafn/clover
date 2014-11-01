#include "worldentity_table.hpp"
#include "global/exception.hpp"

namespace clover {
namespace game {

WETable::WETable():util::PtrTable<game::WorldEntity>(100000){
    nextUniqueEntityId= 1;
}

game::WorldEntityId WETable::assignUniqueId(){
    nextUniqueEntityId++;
    return nextUniqueEntityId-1;
}

game::WorldEntityId WETable::getNextUniqueId() const {
    return nextUniqueEntityId;
}

void WETable::setNextUniqueId(game::WorldEntityId id){
    nextUniqueEntityId= id;
}

WETable gWETable;

} // game
} // clover