#include "mouse.hpp"
        curStates[i]= false;
        prevStates[i]= false;
    }
    curWheelState= 0;
    prevWheelState= 0;
	return position;
		rawPosition.x= min.x; changed= true;
	}
	else if (rawPosition.x >= max.x){
		rawPosition.x= max.x; changed= true;
	}
	if (rawPosition.y <= min.x ){
		rawPosition.y= min.x; changed= true;
	}
	else if (rawPosition.y >= max.y){
		rawPosition.y= max.y; changed= true;
	}
    return curWheelState-prevWheelState;

	//glfwGetMousePos(&rawPosition.x, &rawPosition.y);
    for (int32 i=0; i<Mouse::buttonCount; i++){
        //gMouse->prevStates[i]= gMouse->curStates[i];
        //gMouse->curStates[i]= glfwGetMouseButton(i);
    }
	
    prevWheelState= curWheelState;

    //curWheelState= glfwGetMouseWheel();
	util::Coord view_stretch(util::Coord::View_Stretch);