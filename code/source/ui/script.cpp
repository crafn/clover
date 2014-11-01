#include "script.hpp"
#include "script/script_mgr.hpp"
#include "ui/hid/taglistentry.hpp"

namespace clover {
namespace ui {

void registerToScript(){
	script::gScriptMgr->registerObjectType<ui::hid::TagListEntry>();	
	script::gScriptMgr->registerConstructor<ui::hid::TagListEntry, const util::Str8&, const util::Str8&>();
	script::gScriptMgr->registerConstructor<ui::hid::TagListEntry, const util::Str8&, const util::Str8&, const util::Str8&>();
	script::gScriptMgr->registerMethod(&ui::hid::TagListEntry::isSet, "isSet");
}

} // ui
} // clover