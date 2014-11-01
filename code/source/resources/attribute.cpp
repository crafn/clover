#include "attribute.hpp"
#include "resource.hpp"

namespace clover {
namespace resources {

void tryLaunchAttributeOnChangeCallbacks(const BaseAttribute::OnChangeCallback& cb, Resource* res){
	Resource::State pre_state= Resource::State::Uninit;
	if (res)
		pre_state= res->getResourceState();
	
	if (cb)
		cb(); // Launch change callback of the attribute
	
	// Launch callbacks added to resource only if state hasn't changed
	// onChange has already been called by resource if the state has changed
	if (res && pre_state == res->getResourceState())
		((util::OnChangeCb*)res)->trigger();
}

} // resources
} // clover