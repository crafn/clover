#ifndef CLOVER_GUI_MGR_HPP
#define CLOVER_GUI_MGR_HPP

#include "build.hpp"
#include "audio/audiosourcehandle.hpp"
#include "global/eventreceiver.hpp"
#include "cursor.hpp"
#include "callback.hpp"

namespace clover {
namespace gui {

/// @todo Gui system needs to be redesigned

class Element;

class GuiMgr : public global::EventReceiver {
public:
	GuiMgr();
	virtual ~GuiMgr();

	virtual void onEvent(global::Event&);

	void update();
	
	void destroyTree(Element& e);
	void destroySubTree(Element& e);
	
	Cursor& getCursor();
	
	/// GuiElement calls
	void onDestroy(const Element& e);
private:
	util::DynArray<BaseCallback*> accumulatedCallbacks;

	Cursor guiCursor;
	
	// Gui audio source
	audio::AudioSourceHandle audioSource;
};

extern GuiMgr* gGuiMgr;

} // gui
} // clover

#endif // CLOVER_GUI_MGR_HPP