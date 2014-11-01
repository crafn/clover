#include "worldentity_handleconnection.hpp"
#include "debug/debugprint.hpp"

namespace clover {
namespace game {

util::PtrTable<StrictHandleConnection> StrictHandleConnection::strictHandleTable;

StrictHandleConnection::StrictHandleConnection(	game::WorldEntity& owner_,
												game::WeHandle& handle_)
		: owner(&owner_)
		, handle(&handle_){
	tableIndex= strictHandleTable.insert(*this);

}

StrictHandleConnection::StrictHandleConnection(StrictHandleConnection&& other):
	tableIndex(other.tableIndex),
	owner(other.owner),
	handle(other.handle){

	other.handle= 0;
	other.owner= 0;
	other.tableIndex= -1;
}

StrictHandleConnection::~StrictHandleConnection(){
	//print(debug::Ch::WE, debug::Vb::Trivial, "~StrictHandleConnection");
	if (tableIndex >= 0)
		strictHandleTable.remove(tableIndex);
}

void StrictHandleConnection::resetHandle(game::WeHandle& h){
	handle= &h;
}

uint32 StrictHandleConnection::countConnections(){
	uint32 count= 0;
	for (uint32 i= 0; i < strictHandleTable.size(); ++i){
		if (strictHandleTable[i] != 0)
			++count;
	}
	return count;
}

} // game
} // clover