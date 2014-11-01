#include "keyboard.hpp"

namespace clover {
namespace hardware {

util::Str8 Keyboard::writtenStr;
uint32 Keyboard::backspaceCount;

Keyboard *gKeyboard=0;


Keyboard::Keyboard(){
    for (int32 i=0; i<keyCount; i++){
        currentKBStates[i]=  GLFW_RELEASE;
        previousKBStates[i]= GLFW_RELEASE;
    }
	
	backspaceCount= 0;
	
	//glfwSetCharCallback(charCallback);
	//glfwSetKeyCallback(keyCallback);
	//glfwEnable(GLFW_KEY_REPEAT);
}
/*
void GLFWCALL Keyboard::charCallback(int32 character, int32 action){

    if (action == GLFW_PRESS){

        if (character != GLFW_KEY_LSHIFT &&
            character != GLFW_KEY_RSHIFT){

            writtenStr += character;
        }
		

    }
}

void GLFWCALL Keyboard::keyCallback(int32 character, int32 action){
	if (action == GLFW_PRESS){
		if (character == GLFW_KEY_BACKSPACE)
			++backspaceCount;
	}
}*/

bool Keyboard::isDown(int32 code){
    if (code >= keyCount) return false;
    if (currentKBStates[code] == GLFW_PRESS)
        return true;

    return false;

}


bool Keyboard::isPressed(int32 code){
    if (code >= keyCount) return false;
    if (currentKBStates[code] == GLFW_PRESS && previousKBStates[code] == GLFW_RELEASE)
        return true;

    return false;
}


bool Keyboard::isReleased(int32 code){
    if (code >= keyCount) return false;
    if (currentKBStates[code] == GLFW_RELEASE && previousKBStates[code] == GLFW_PRESS)
        return true;

    return false;
}

util::Str8 Keyboard::getWrittenStr(){
    return writtenStr;
}

uint32 Keyboard::getWrittenBackspaceCount(){
	return backspaceCount;
}

void Keyboard::update(){
	
	for (int32 i=0; i<keyCount; i++){
        previousKBStates[i]= currentKBStates[i];
        //currentKBStates[i]= glfwGetKey(i);
    }
}

void Keyboard::clearWriting(){
	writtenStr= "";
	backspaceCount= 0;
}

} // hardware
} // clover