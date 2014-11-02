#ifndef CLOVER_UI_USERINPUT_HPP
#define CLOVER_UI_USERINPUT_HPP

#include "build.hpp"
#include "hid/actionlistener.hpp"
#include "util/coord.hpp"

namespace clover {

/// @todo Replace with new action-based input mapping
class UserInput {
public:
	enum TriggerType {
		GuiCause, // Esim. napin painaminen
		GuiSecondaryCause, // RMB
		GuiHold, // Esim. hiiren pohjassa pitäminen
		GuiCancel, // Esim. Comboboxin sulkeminen
		GuiStartLeftDragging, // Esim. Sliderin säätö
		GuiStartMiddleDragging,
		GuiStartRightDragging,
		GuiStopDragging,
		GuiConfirm, // Enter
		GuiPrevHistoryEntry, // Up
		GuiNextHistoryEntry, // Down
		
		ToggleQuickSelectRing, // Rinkulat
		ToggleInventory,
		NextLinkedQuickSelectEntity, // Seuraava entity pikanäppäimitetyistä
		PrevLinkedQuickSelectEntity, // Edellinen entity pikanäppäimitetyistä
		PrimaryAction,	// Pelissä entityn ensisijainen toiminto (hiiren vasen klikkaus)
		ThrowAwayCharge, // Pitämällä pohjassa heitto menee pidemmälle
		ThrowAwayLaunch, // Heitto
		RunLeft,
		RunRight,
		LaunchJump,
		MaintainJump,
		Suicide,
		Respawn,
		
		DevCamRight,
		DevCamUp,
		DevCamLeft,
		DevCamDown,
		DevCamZoomIn,
		DevCamZoomOut,
		DevSaveResources,
		
		ToggleEditorView_0,
		ToggleEditorView_1,
		ToggleEditorView_2,
		ToggleEditorView_3,
	
		MouseLeftPress,
		MouseLeftDown,
		MouseLeftRelease,
		
		MouseMiddlePress,
		MouseMiddleDown,
		MouseMiddleRelease,
		
		MouseRightPress,
		MouseRightDown,
		MouseRightRelease,
		
		LastTriggerType
	};
	
	struct TriggerDef {
		TriggerType triggerType;
		const char* channelName;
		const char* contextName;
		const char* actionName;
	};
	
	/// Quick hack to map crappy old system to the new context-based system
	static constexpr TriggerDef triggerList[LastTriggerType] = {
		{GuiCause,					  "host",	 "gui",					"primaryRelease"	   },
		{GuiSecondaryCause,			  "host",	 "gui",					"secondaryRelease"	   },
		{GuiHold,					  "host",	 "gui",					"primaryDown"		   },
		{GuiCancel,					  "host",	 "gui",					"secondaryRelease"	   },
		{GuiStartLeftDragging,		  "host",	 "gui",					"primaryPress"		   },
		{GuiStartMiddleDragging,	  "host",	 "gui",					"middlePress"		   },
		{GuiStartRightDragging,		  "host",	 "gui",					"secondaryPress"	   },
		{GuiStopDragging,			  "host",	 "gui",					"anyRelease"		   },
		{GuiConfirm,				  "host",	 "gui",					"confirm"			   },
		{GuiPrevHistoryEntry,		  "host",	 "gui",					"prevListEntry"		   },
		{GuiNextHistoryEntry,		  "host",	 "gui",					"nextListEntry"		   },

		{ToggleQuickSelectRing,		  "player0", "charCtrl",			"toggleQuickSelect"	   },
		{ToggleInventory,			  "player0", "charCtrl",			"toggleInventory"	   },
		{NextLinkedQuickSelectEntity, "player0", "charCtrl",			"nextItem"			   },
		{PrevLinkedQuickSelectEntity, "player0", "charCtrl",			"prevItem"			   },
		{PrimaryAction,				  "player0", "charCtrl",			"primaryAction"		   },
		{ThrowAwayCharge,			  "player0", "charCtrl",			"chargeThrowAway"	   },
		{ThrowAwayLaunch,			  "player0", "charCtrl",			"launchThrowAway"	   },
		{RunLeft,					  "player0", "charCtrl",			"runLeft"			   },
		{RunRight,					  "player0", "charCtrl",			"runRight"			   },
		{LaunchJump,				  "player0", "charCtrl",			"launchJump"		   },
		{MaintainJump,				  "player0", "charCtrl",			"maintainJump"		   },
		{Suicide,					  "player0", "charCtrl",			"suicide"			   },
		{Respawn,					  "player0", "charCtrl",			"respawn"			   },

		{DevCamRight,				  "host",	 "dev",					"camRight"			   },
		{DevCamUp,					  "host",	 "dev",					"camUp"				   },
		{DevCamLeft,				  "host",	 "dev",					"camLeft"			   },
		{DevCamDown,				  "host",	 "dev",					"camDown"			   },
		{DevCamZoomIn,				  "host",	 "dev",					"camZoomIn"			   },
		{DevCamZoomOut,				  "host",	 "dev",					"camZoomOut"		   },
		{DevSaveResources,			  "host",	 "dev",					"saveResources"		   },

		{ToggleEditorView_0,		  "host",	 "dev",					"toggleEditorView0"	   },
		{ToggleEditorView_1,		  "host",	 "dev",					"toggleEditorView1"	   },
		{ToggleEditorView_2,		  "host",	 "dev",					"toggleEditorView2"	   },
		{ToggleEditorView_3,		  "host",	 "dev",					"toggleEditorView3"	   },

		{MouseLeftPress,			  "host",	 "gui",					"primaryPress"		   },
		{MouseLeftDown,				  "host",	 "gui",					"primaryDown"		   },
		{MouseLeftRelease,			  "host",	 "gui",					"primaryRelease"	   },

		{MouseMiddlePress,			  "host",	 "gui",					"middlePress"		   },
		{MouseMiddleDown,			  "host",	 "gui",					"middleDown"		   },
		{MouseMiddleRelease,		  "host",	 "gui",					"middleRelease"		   },

		{MouseRightPress,			  "host",	 "gui",					"secondaryPress"	   },
		{MouseRightDown,			  "host",	 "gui",					"secondaryDown"		   },
		{MouseRightRelease,			  "host",	 "gui",					"secondaryRelease"	   },
		
	};
	
	struct TriggerState {
		TriggerState():actionTriggered(false), curState(false), prevState(false){}
		bool actionTriggered;
		bool curState, prevState;
	};
	
	
	UserInput();
	
	/// Is user doing something
	bool isTriggered(TriggerType t) const { return triggerStates[t].curState; }
	
	/// Is this the first frame of trigger on
	bool hasTriggeringStarted(TriggerType t) const { return triggerStates[t].curState && !triggerStates[t].prevState; }
	bool hasTriggeringStopped(TriggerType t) const { return !triggerStates[t].curState && triggerStates[t].prevState; }
	
	void update();
	
	util::Coord getCursorPosition();
	
	/// Difference from previous frame to current one (also in lock mode)
	util::Coord getCursorDifference();


	/// Aloittaa lukitusmoodin
	void pushCursorLock();
	
	/// Lopettaa lukitusmoodin
	void popCursorLock();
	
	bool isCursorLocked(){ return cursorLocked; }
	
	/// Ignore all keypress-triggers except some of the Gui* (like when writing to a textfield)
	void pushLockForWriting();
	void popLockForWriting();

	/// No locks
	bool isInputFree() const { return !cursorLocked && !keyTriggerLocked; }
	
	/// Returns characters that were written in last frame
	util::Str8 getWrittenString();
	/// Backspace count
	uint32 getEraseCharCount();
	
private:
	bool isTriggeredImpl(TriggerType t);

	TriggerState triggerStates[LastTriggerType];

	uint32 cursorLocked;
	uint32 keyTriggerLocked;
	
	util::Vec2d pos;
	util::Vec2d posChange;
	util::Str8 writtenStr;
	ui::hid::ActionListener<nodes::SignalType::String> writeListener;
	
	util::LinkedList<ui::hid::ActionListener<>> actionListeners;
};

extern UserInput _gUserInput; 
extern UserInput *gUserInput; 

} // clover

#endif // CLOVER_UI_USERINPUT_HPP
