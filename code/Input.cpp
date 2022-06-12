#include "Input.h"

#include <SDL.h>

#include "Common.h"

//////////////////////////////////////////////////////////////////////
// GAME SPECIFIC FUNCTIONS
//////////////////////////////////////////////////////////////////////

bool Input::GameUpHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_UP)
        || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_DPAD_UP)
        || Input::GetGamepad(0).leftThumbStickDelta.y > 0.5f;
}

bool Input::GameDownHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_DOWN)
        || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_DPAD_DOWN)
        || Input::GetGamepad(0).leftThumbStickDelta.y < -0.5f;
}

bool Input::GameLeftHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_LEFT)
        || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_DPAD_LEFT)
        || Input::GetGamepad(0).leftThumbStickDelta.x > 0.5f;
}

bool Input::GameRightHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_RIGHT)
        || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_DPAD_RIGHT)
        || Input::GetGamepad(0).leftThumbStickDelta.y < -0.5f;
}

bool Input::GameUpIsPressed()
{
    return Input::IsKeyPressed(SDL_SCANCODE_UP)
        || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_UP)
        || Input::GetGamepad(0).leftThumbStickDir.y < -0.5f;
    // || dot(Input::GetGamepad(0).leftThumbStickDir, {0.f, 1.f}) < QUARTER_PI;
}

bool Input::GameDownIsPressed()
{
    return Input::IsKeyPressed(SDL_SCANCODE_DOWN)
        || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_DOWN)
        || Input::GetGamepad(0).leftThumbStickDir.y > 0.5f;
    // || dot(Input::GetGamepad(0).leftThumbStickDir, {0.f, -1.f}) < QUARTER_PI;
}

bool Input::GameLeftIsPressed()
{
    return Input::IsKeyPressed(SDL_SCANCODE_LEFT)
        || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT)
        || Input::GetGamepad(0).leftThumbStickDir.x < -0.5f;
    // || dot(Input::GetGamepad(0).leftThumbStickDir, {1.f, 0.f}) < QUARTER_PI;
}

bool Input::GameRightIsPressed()
{
    return Input::IsKeyPressed(SDL_SCANCODE_RIGHT)
        || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT)
        || Input::GetGamepad(0).leftThumbStickDir.x > 0.5f;
    // || dot(Input::GetGamepad(0).leftThumbStickDir, {-1.f, 0.f}) < QUARTER_PI;
}

bool Input::GameJumpHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_SPACE) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_A);
}

bool Input::GameJumpHasBeenReleased()
{
    return Input::HasKeyBeenReleased(SDL_SCANCODE_SPACE) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_A);
}

bool Input::GameJumpIsPressed()
{
    return Input::IsKeyPressed(SDL_SCANCODE_SPACE) || Input::GetGamepad(0).IsPressed(GAMEPAD_A);
}

bool Input::GameAttackHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_Z) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_X);
}

bool Input::GamePickUpHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_C) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_B);
}

bool Input::GamePauseHasBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_ESCAPE) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_START);
}

bool Input::GameCycleItemLeftBeenPressed()
{
    return Input::GetGamepad(0).HasBeenPressed(GAMEPAD_LB); //Input::HasKeyBeenPressed(SDL_SCANCODE_Q) || 
}

bool Input::GameCycleItemRightBeenPressed()
{
    return Input::HasKeyBeenPressed(SDL_SCANCODE_X) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_RB);
}

bool Input::GameInteractButtonHasBeenPressed()
{
    return Input::GameUpHasBeenPressed();
}

//////////////////////////////////////////////////////////////////////
// GENERIC CLASSES AND FUNCTIONS BELOW
//////////////////////////////////////////////////////////////////////

/** Input States **/
// Keyboard State - Access via SDL_Scancode https://wiki.libsdl.org/SDL_Scancode
INTERNAL u8 gCurrentKeyState[256];
INTERNAL u8 gJustPressedKeyState[256];
INTERNAL u8 gJustReleasedKeyState[256];
// Gamepad State - Look inside GamepadStatesWrapper and GamepadStates
INTERNAL Input::GamepadStatesWrapper gGamepadStates;

INTERNAL bool invertYAxis = false;
INTERNAL i16 SDL_JOYSTICK_DEAD_ZONE = 8000;
INTERNAL i16 SDL_JOYSTICK_MAX = 32767;

namespace Input {

    /** Keyboard **/
    bool IsKeyPressed(SDL_Scancode scancode)
    {
        return gCurrentKeyState[scancode];
    }

    bool IsKeyReleased(SDL_Scancode scancode)
    {
        return !IsKeyPressed(scancode);
    }

    bool HasKeyBeenPressed(SDL_Scancode scancode)
    {
        return gJustPressedKeyState[scancode];
    }

    bool HasKeyBeenReleased(SDL_Scancode scancode)
    {
        return gJustReleasedKeyState[scancode];
    }

    void ProcessSDLKeyDownEvent(SDL_KeyboardEvent keyEvent) {
        SDL_Scancode key = keyEvent.keysym.scancode;
        gCurrentKeyState[key] = 1;
        if (!keyEvent.repeat) {
            gJustPressedKeyState[key] = 1;
        }
    }

    void ProcessSDLKeyUpEvent(SDL_KeyboardEvent keyEvent) {
        // Never set JustPressed or JustReleased to 0 here.
        SDL_Scancode key = keyEvent.keysym.scancode;
        gCurrentKeyState[key] = 0;
        gJustReleasedKeyState[key] = 1;
    }

    void ResetKeyboardStates() {
        memset(gJustPressedKeyState, 0, 256);
        memset(gJustReleasedKeyState, 0, 256);
    }


    /** Gamepad **/

    GamepadState& GetGamepad(i32 playerNumber)
    {
        return gGamepadStates[playerNumber];
    }

    void ProcessSDLControllerButtonDownEvent(SDL_ControllerButtonEvent gamepadButtonEvent) {
        /* SDL_GameControllerButton reference:
        SDL_CONTROLLER_BUTTON_A 0
        SDL_CONTROLLER_BUTTON_B 1
        SDL_CONTROLLER_BUTTON_X 2
        SDL_CONTROLLER_BUTTON_Y 3
        SDL_CONTROLLER_BUTTON_BACK 4
        SDL_CONTROLLER_BUTTON_GUIDE 5
        SDL_CONTROLLER_BUTTON_START 6
        SDL_CONTROLLER_BUTTON_LEFTSTICK 7
        SDL_CONTROLLER_BUTTON_RIGHTSTICK 8
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER 9
        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER 10
        SDL_CONTROLLER_BUTTON_DPAD_UP 11
        SDL_CONTROLLER_BUTTON_DPAD_DOWN 12
        SDL_CONTROLLER_BUTTON_DPAD_LEFT 13
        SDL_CONTROLLER_BUTTON_DPAD_RIGHT 14
        */

        i32 joystickInstanceID = gamepadButtonEvent.which;
        i32 gamepadIndex = gGamepadStates.GamepadIndexFromInstanceID(joystickInstanceID);
        u8 buttonIndex = gamepadButtonEvent.button;

        switch (buttonIndex) {
        case 0: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_A;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_A;
        }
              break;
        case 1: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_B;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_B;
        }
              break;
        case 2: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_X;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_X;
        }
              break;
        case 3: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_Y;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_Y;
        }
              break;
        case 4: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_BACK;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_BACK;
        }
              break;
        case 6: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_START;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_START;
        }
              break;
        case 7: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_LTHUMB;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_LTHUMB;
        }
              break;
        case 8: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_RTHUMB;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_RTHUMB;
        }
              break;
        case 9: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_LB;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_LB;
        }
              break;
        case 10: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_RB;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_RB;
        }
               break;
        case 11: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_DPAD_UP;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_DPAD_UP;
        }
               break;
        case 12: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_DPAD_DOWN;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_DPAD_DOWN;
        }
               break;
        case 13: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_DPAD_LEFT;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_DPAD_LEFT;
        }
               break;
        case 14: {
            gGamepadStates[gamepadIndex].currentButtonState |= GAMEPAD_DPAD_RIGHT;
            gGamepadStates[gamepadIndex].justPressedButtonState |= GAMEPAD_DPAD_RIGHT;
        }
               break;
        }
    }

    void ProcessSDLControllerButtonUpEvent(SDL_ControllerButtonEvent gamepadButtonEvent) {
        i32 joystickInstanceID = gamepadButtonEvent.which;
        i32 gamepadIndex = gGamepadStates.GamepadIndexFromInstanceID(joystickInstanceID);
        u8 buttonIndex = gamepadButtonEvent.button;

        switch (buttonIndex) {
        case 0: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_A;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_A;
        }
              break;
        case 1: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_B;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_B;
        }
              break;
        case 2: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_X;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_X;
        }
              break;
        case 3: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_Y;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_Y;
        }
              break;
        case 4: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_BACK;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_BACK;
        }
              break;
        case 6: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_START;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_START;
        }
              break;
        case 7: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_LTHUMB;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_LTHUMB;
        }
              break;
        case 8: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_RTHUMB;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_RTHUMB;
        }
              break;
        case 9: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_LB;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_LB;
        }
              break;
        case 10: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_RB;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_RB;
        }
               break;
        case 11: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_DPAD_UP;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_DPAD_UP;
        }
               break;
        case 12: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_DPAD_DOWN;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_DPAD_DOWN;
        }
               break;
        case 13: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_DPAD_LEFT;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_DPAD_LEFT;
        }
               break;
        case 14: {
            gGamepadStates[gamepadIndex].currentButtonState ^= GAMEPAD_DPAD_RIGHT;
            gGamepadStates[gamepadIndex].justReleasedButtonState |= GAMEPAD_DPAD_RIGHT;
        }
               break;
        }
    }

    void ProcessSDLControllerAxisEvent(SDL_ControllerAxisEvent gamepadAxisEvent) {
        i32 joystickInstanceID = gamepadAxisEvent.which;
        i32 gamepadIndex = gGamepadStates.GamepadIndexFromInstanceID(joystickInstanceID);
        u8 axisIndex = gamepadAxisEvent.axis;
        i16 axisValue = gamepadAxisEvent.value;

        switch (axisIndex) {
        case SDL_CONTROLLER_AXIS_LEFTX: {
            if (GM_abs(axisValue) > SDL_JOYSTICK_DEAD_ZONE) {
                vec2 leftAxisDir = gGamepadStates[gamepadIndex].leftThumbStickDir * (float)SDL_JOYSTICK_MAX;
                float oldX = leftAxisDir.x;
                leftAxisDir.x = (float)axisValue;
                leftAxisDir /= (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].leftThumbStickDir = leftAxisDir;

                vec2 leftAxisDelta = gGamepadStates[gamepadIndex].leftThumbStickDelta;
                leftAxisDelta.x = leftAxisDir.x - oldX;
                gGamepadStates[gamepadIndex].leftThumbStickDelta = leftAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
            else {
                float oldX = gGamepadStates[gamepadIndex].leftThumbStickDir.x * (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].leftThumbStickDir.x = 0.f;

                vec2 leftAxisDelta = gGamepadStates[gamepadIndex].leftThumbStickDelta;
                leftAxisDelta.x = -oldX;
                gGamepadStates[gamepadIndex].leftThumbStickDelta = leftAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
        }
                                      break;
        case SDL_CONTROLLER_AXIS_LEFTY: {
            if (GM_abs(axisValue) > SDL_JOYSTICK_DEAD_ZONE) {
                vec2 leftAxisDir = gGamepadStates[gamepadIndex].leftThumbStickDir * (float)SDL_JOYSTICK_MAX;
                float oldY = leftAxisDir.y;
                leftAxisDir.y = (float)(invertYAxis ? -axisValue : axisValue);
                leftAxisDir /= (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].leftThumbStickDir = leftAxisDir;

                vec2 leftAxisDelta = gGamepadStates[gamepadIndex].leftThumbStickDelta;
                leftAxisDelta.y = leftAxisDir.y - oldY;
                gGamepadStates[gamepadIndex].leftThumbStickDelta = leftAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
            else {
                float oldY = gGamepadStates[gamepadIndex].leftThumbStickDir.y * (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].leftThumbStickDir.y = 0.f;

                vec2 leftAxisDelta = gGamepadStates[gamepadIndex].leftThumbStickDelta;
                leftAxisDelta.y = -oldY;
                gGamepadStates[gamepadIndex].leftThumbStickDelta = leftAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
        }
                                      break;
        case SDL_CONTROLLER_AXIS_RIGHTX: {
            if (GM_abs(axisValue) > SDL_JOYSTICK_DEAD_ZONE) {
                vec2 rightAxisDir = gGamepadStates[gamepadIndex].rightThumbStickDir * (float)SDL_JOYSTICK_MAX;
                float oldX = rightAxisDir.x;
                rightAxisDir.x = (float)axisValue;
                rightAxisDir /= (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].rightThumbStickDir = rightAxisDir;

                vec2 rightAxisDelta = gGamepadStates[gamepadIndex].rightThumbStickDelta;
                rightAxisDelta.x = rightAxisDir.x - oldX;
                gGamepadStates[gamepadIndex].leftThumbStickDelta = rightAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
            else {
                float oldX = gGamepadStates[gamepadIndex].rightThumbStickDir.x * (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].rightThumbStickDir.x = 0.f;

                vec2 rightAxisDelta = gGamepadStates[gamepadIndex].rightThumbStickDelta;
                rightAxisDelta.x = -oldX;
                gGamepadStates[gamepadIndex].rightThumbStickDelta = rightAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
        }
                                       break;
        case SDL_CONTROLLER_AXIS_RIGHTY: {
            if (GM_abs(axisValue) > SDL_JOYSTICK_DEAD_ZONE) {
                vec2 rightAxisDir = gGamepadStates[gamepadIndex].rightThumbStickDir * (float)SDL_JOYSTICK_MAX;
                float oldY = rightAxisDir.y;
                rightAxisDir.y = (float)(invertYAxis ? -axisValue : axisValue);
                rightAxisDir /= (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].rightThumbStickDir = rightAxisDir;

                vec2 rightAxisDelta = gGamepadStates[gamepadIndex].rightThumbStickDelta;
                rightAxisDelta.y = rightAxisDir.y - oldY;
                gGamepadStates[gamepadIndex].leftThumbStickDelta = rightAxisDelta;
            }
            else {
                float oldY = gGamepadStates[gamepadIndex].rightThumbStickDir.y * (float)SDL_JOYSTICK_MAX;
                gGamepadStates[gamepadIndex].rightThumbStickDir.y = 0.f;

                vec2 rightAxisDelta = gGamepadStates[gamepadIndex].rightThumbStickDelta;
                rightAxisDelta.y = -oldY;
                gGamepadStates[gamepadIndex].rightThumbStickDelta = rightAxisDelta / (float)SDL_JOYSTICK_MAX;
            }
        }
                                       break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
            if (GM_abs(axisValue) > SDL_JOYSTICK_DEAD_ZONE) {
                gGamepadStates[gamepadIndex].leftTrigger =
                    (axisValue + ((float)SDL_JOYSTICK_MAX)) / (2 * ((float)SDL_JOYSTICK_MAX));
            }
            else {
                gGamepadStates[gamepadIndex].leftTrigger = 0.f;
            }
        }
                                            break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
            if (GM_abs(axisValue) > SDL_JOYSTICK_DEAD_ZONE) {
                gGamepadStates[gamepadIndex].rightTrigger =
                    (axisValue + ((float)SDL_JOYSTICK_MAX)) / (2 * ((float)SDL_JOYSTICK_MAX));
            }
            else {
                gGamepadStates[gamepadIndex].rightTrigger = 0.f;
            }
        }
                                             break;
        }
    }

    void ResetControllerStates() {
        for (int i = 0; i < MAX_GAMEPAD_COUNT; ++i) {
            gGamepadStates[i].justPressedButtonState = 0;
            gGamepadStates[i].justReleasedButtonState = 0;
        }
    }

    void SDLControllerConnected(i32 deviceIndex) {
        SDL_GameController* deviceHandle = SDL_GameControllerOpen(deviceIndex);
        i32 joystickInstanceID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(deviceHandle));
        gGamepadStates.OnConnect(joystickInstanceID);

        printf("Gamepad connected. Instance ID: %d\n", joystickInstanceID);
    }

    void SDLControllerRemoved(i32 joystickInstanceID) {
        gGamepadStates.OnDisconnect(joystickInstanceID);
        SDL_GameControllerClose(SDL_GameControllerFromInstanceID(joystickInstanceID));

        printf("Gamepad disconnected. Instance ID: %d\n", joystickInstanceID);
    }

    void SetInvertYAxis(bool invert)
    {
        invertYAxis = invert;
    }

    /** GamepadState Implementation **/

    bool GamepadState::IsPressed(u16 button /* bitwise-OR-able */) const {
        return currentButtonState & button;
    }

    bool GamepadState::IsReleased(u16 button /* bitwise-OR-able */) const {
        return !IsPressed(button);
    }

    bool GamepadState::HasBeenPressed(u16 button) const {
        return justPressedButtonState & button;
    }

    bool GamepadState::HasBeenReleased(u16 button) const {
        return justReleasedButtonState & button;
    }


    /** GamepadStatesWrapper Implementation **/

    GamepadState& GamepadStatesWrapper::AtInstanceID(const i32 instanceID) {
        return states[mapInstanceIDtoGamepadIndex[instanceID]];
    }

    u8 GamepadStatesWrapper::GamepadIndexFromInstanceID(const i32 instanceID) {
        return mapInstanceIDtoGamepadIndex[instanceID];
    }

    void GamepadStatesWrapper::OnConnect(i32 instanceID) {
        mapInstanceIDtoGamepadIndex[instanceID] = gamepadCount;
        states[gamepadCount].isConnected = true;
        ++gamepadCount;
    }

    void GamepadStatesWrapper::OnDisconnect(i32 instanceID) {
        i32 gamepadIndex = mapInstanceIDtoGamepadIndex[instanceID];

        // shift game states left
        for (i32 i = gamepadIndex; i < gamepadCount - 1; ++i) {
            states[i] = states[i + 1];
        }

        // reset last game state. we shift these over so just resetting the last is enough.
        states[gamepadCount - 1].isConnected = false;
        states[gamepadCount - 1].currentButtonState = 0;
        states[gamepadCount - 1].justPressedButtonState = 0;
        states[gamepadCount - 1].justReleasedButtonState = 0;
        states[gamepadCount - 1].leftTrigger = 0.f;
        states[gamepadCount - 1].rightTrigger = 0.f;
        states[gamepadCount - 1].leftThumbStickDir = { 0.f, 0.f };
        states[gamepadCount - 1].leftThumbStickDir = { 0.f, 0.f };

        // shift indices left
        for (int i = 0; i < 128; ++i) {
            if (gamepadIndex < mapInstanceIDtoGamepadIndex[i]) {
                --mapInstanceIDtoGamepadIndex[i];
            }
        }

        --gamepadCount;
    }
}
