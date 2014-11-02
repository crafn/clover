#ifndef CLOVER_HARDWARE_MOUSE_HPP
    static const int32 buttonCount= GLFW_MOUSE_BUTTON_LAST +1;
    bool curStates[buttonCount];
    bool prevStates[buttonCount];

    int32 curWheelState;
    int32 prevWheelState;