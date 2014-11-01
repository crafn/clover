#ifndef CLOVER_UI_EDITOR_EC_UI_RESOURCE_HPP
#define CLOVER_UI_EDITOR_EC_UI_RESOURCE_HPP

#include "../editorcomponent_ui.hpp"
#include "build.hpp"
#include "game/editor/components/ec_resource.hpp"
#include "global/event.hpp"
#include "gui/element_contextmenu.hpp"
#include "gui/element_combobox.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_button.hpp"
#include "gui/element_attributefield.hpp"
#include "gui/attributefieldelementfactory.hpp"
#include "resources/attribute_def.hpp"
#include "resources/baseattribute.hpp"
#include "util/cb_listener.hpp"
#include "util/string.hpp"

#define ATTRIBUTE_FIELD_HEADERS
#include "resources/attributetypes.def"
#undef ATTRIBUTE_FIELD_HEADERS

namespace clover {
namespace ui { namespace game { namespace editor {

/// Useful tools for resourceUis
template <typename T> // ResourceEc<R> T
class BaseResourceEcUi : public EditorComponentUi<T> {
public:
	typedef EditorComponentUi<T> Base;
	typedef typename Base::ComponentType ComponentType;
	typedef typename ComponentType::ResourceType ResourceType;
	
	BaseResourceEcUi(T& comp, bool show_attributes= true);
	virtual ~BaseResourceEcUi();

protected:
	ResourceType* getSelectedResource(){ return Base::getComponent().getSelectedResource(); }

	void createAttributeFields(gui::Element* super);
	void clearAttributeFields();
	
	/// @todo Some of this stuff should be private
	
	gui::ContextMenuElement contextMenu;
	gui::ContextMenuElement::NodeId createResourceNodeId;
	gui::ContextMenuElement::NodeId destroyResourceNodeId;
	
	// Create new -dialog
	using IdentifierAttributeType= typename resources::ResourceTraits<ResourceType>::IdentifierAttributeType;
	using IdentifierElement= gui::AttributeFieldElement<IdentifierAttributeType>;
	
	gui::FloatingElement createDialogFloating;
	gui::PanelElement createDialogBgPanel;
	gui::LinearLayoutElement createDialogLayout;
	gui::ComboBoxElement createDialogResFileCombo;
	resources::AttributeDef createDialogIdentifierAttributeDef;
	IdentifierElement createDialogIdentifier;
	gui::ButtonElement createDialogCreateButton;
	
private:
	void clear();
	void onResourceSelect(ResourceType& res);
	void onResourceChange(); // When *resource changes or new resource is selected
	void onCreateButtonPress();

	util::CbListener<util::OnChangeCb> resourceChangeListener;

	using FieldPtr= std::shared_ptr<gui::BaseAttributeFieldElement>;
	void initAttributeField(FieldPtr& field, const resources::AttributeDef& def, gui::Element* super);
	util::DynArray<FieldPtr> attributeFields;

	util::DynArray<resources::AttributeDef> attributeDefs;
};

/// Default ResourceEcUi
template <typename T>
class ResourceEcUi : public BaseResourceEcUi<T> {
public:
	typedef BaseResourceEcUi<T> Base;
	
	ResourceEcUi(T& comp);
	virtual ~ResourceEcUi();
	
protected:
	virtual void onResize();
	
private:
	gui::LinearLayoutElement contentLayout;
};

#include "ec_ui_resource.tpp"

template <typename T>
struct EditorComponentUiTraits<ResourceEc<T>> {

	static util::Coord defaultRadius(){
		if (resources::ResourceTraits<T>::getAttributeDefs().size() < 2)
			return util::Coord::P(25); // Empty (has only idenfitier attributes)

		return util::Coord::P({150, 150});
	}

};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_RESOURCE_HPP