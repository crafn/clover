#ifndef CLOVER_UI_EDITOR_EC_UI_RESOURCELIST_HPP
#define CLOVER_UI_EDITOR_EC_UI_RESOURCELIST_HPP

#include "../editorcomponent_ui.hpp"
#include "build.hpp"
#include "game/editor/components/ec_resourcelist.hpp"
#include "gui/element_textfield.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_listbox.hpp"
#include "util/cb_listener.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

class ResourceListEcUi : public EditorComponentUi<ResourceListEc> {
public:
	typedef EditorComponentUi<ResourceListEc> BaseType;
	
	ResourceListEcUi(ResourceListEc& comp);
	virtual ~ResourceListEcUi();

	virtual void update() override;

protected:
	virtual void onResize() override;

private:
	void updateSearch(const util::Str8& keywords);
	void updateSearchGui();

	gui::LinearLayoutElement contentLayout;
	gui::TextFieldElement searchField;
	gui::ListBoxElement searchResultsList;
	
	bool searchGuiUpdateNeeded; // Updates gui within a frame if true
	
	int32 maxResultCount;
	util::DynArray<resources::Resource*> searchResults;
	util::CbMultiListener<util::OnChangeCb> resourceChangeListener;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_RESOURCELIST_HPP