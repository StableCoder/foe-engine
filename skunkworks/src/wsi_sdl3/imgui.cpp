// Copyright (C) 2025-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "imgui.hpp"

#include "window.hpp"

namespace {

char const *windowBackend(void *pContext) { return "SDL3"; }

char const *windowTitle(void *pContext) {
    SDL3_WindowData *pWindowData = (SDL3_WindowData *)pContext;

    return SDL_GetWindowTitle(pWindowData->pWindow);
}

bool windowTerminationCalled(void *pContext) {
    SDL3_WindowData *pWindowData = (SDL3_WindowData *)pContext;

    return pWindowData->close;
}

void windowLogicalSize(void *pContext, int *pWidth, int *pHeight) {
    SDL3_WindowData *pWindowData = (SDL3_WindowData *)pContext;

    SDL_GetWindowSize(pWindowData->pWindow, pWidth, pHeight);
}

void windowPixelSize(void *pContext, int *pWidth, int *pHeight) {
    SDL3_WindowData *pWindowData = (SDL3_WindowData *)pContext;

    SDL_GetWindowSizeInPixels(pWindowData->pWindow, pWidth, pHeight);
}

void windowContentScale(void *pContext, float *pX, float *pY) {
    SDL3_WindowData *pWindowData = (SDL3_WindowData *)pContext;

    float scale = SDL_GetWindowDisplayScale(pWindowData->pWindow);

    *pX = scale;
    *pY = scale;
}

} // namespace

bool imguiAddSDL3Window(foeImGuiWindow *pImguiWindow,
                        SDL3_WindowData *pWindowData,
                        KeyboardInput const *pKeyboard,
                        MouseInput const *pMouse) {
    return pImguiWindow->addWindow(pWindowData, windowBackend, windowTitle, windowTerminationCalled,
                                   windowLogicalSize, windowPixelSize, windowContentScale,
                                   pKeyboard, pMouse);
}

ImGuiKey imguiSDL3KeyConvert(int keycode, int scancode) {
    // Keypad doesn't have individual key values in SDL3
    switch (scancode) {
    case SDL_SCANCODE_KP_0:
        return ImGuiKey_Keypad0;
    case SDL_SCANCODE_KP_1:
        return ImGuiKey_Keypad1;
    case SDL_SCANCODE_KP_2:
        return ImGuiKey_Keypad2;
    case SDL_SCANCODE_KP_3:
        return ImGuiKey_Keypad3;
    case SDL_SCANCODE_KP_4:
        return ImGuiKey_Keypad4;
    case SDL_SCANCODE_KP_5:
        return ImGuiKey_Keypad5;
    case SDL_SCANCODE_KP_6:
        return ImGuiKey_Keypad6;
    case SDL_SCANCODE_KP_7:
        return ImGuiKey_Keypad7;
    case SDL_SCANCODE_KP_8:
        return ImGuiKey_Keypad8;
    case SDL_SCANCODE_KP_9:
        return ImGuiKey_Keypad9;
    case SDL_SCANCODE_KP_PERIOD:
        return ImGuiKey_KeypadDecimal;
    case SDL_SCANCODE_KP_DIVIDE:
        return ImGuiKey_KeypadDivide;
    case SDL_SCANCODE_KP_MULTIPLY:
        return ImGuiKey_KeypadMultiply;
    case SDL_SCANCODE_KP_MINUS:
        return ImGuiKey_KeypadSubtract;
    case SDL_SCANCODE_KP_PLUS:
        return ImGuiKey_KeypadAdd;
    case SDL_SCANCODE_KP_ENTER:
        return ImGuiKey_KeypadEnter;
    case SDL_SCANCODE_KP_EQUALS:
        return ImGuiKey_KeypadEqual;
    default:
        break;
    }
    switch (keycode) {
    case SDLK_TAB:
        return ImGuiKey_Tab;
    case SDLK_LEFT:
        return ImGuiKey_LeftArrow;
    case SDLK_RIGHT:
        return ImGuiKey_RightArrow;
    case SDLK_UP:
        return ImGuiKey_UpArrow;
    case SDLK_DOWN:
        return ImGuiKey_DownArrow;
    case SDLK_PAGEUP:
        return ImGuiKey_PageUp;
    case SDLK_PAGEDOWN:
        return ImGuiKey_PageDown;
    case SDLK_HOME:
        return ImGuiKey_Home;
    case SDLK_END:
        return ImGuiKey_End;
    case SDLK_INSERT:
        return ImGuiKey_Insert;
    case SDLK_DELETE:
        return ImGuiKey_Delete;
    case SDLK_BACKSPACE:
        return ImGuiKey_Backspace;
    case SDLK_SPACE:
        return ImGuiKey_Space;
    case SDLK_RETURN:
        return ImGuiKey_Enter;
    case SDLK_ESCAPE:
        return ImGuiKey_Escape;
    // case SDLK_APOSTROPHE: return ImGuiKey_Apostrophe;
    case SDLK_COMMA:
        return ImGuiKey_Comma;
    // case SDLK_MINUS: return ImGuiKey_Minus;
    case SDLK_PERIOD:
        return ImGuiKey_Period;
    // case SDLK_SLASH: return ImGuiKey_Slash;
    case SDLK_SEMICOLON:
        return ImGuiKey_Semicolon;
    // case SDLK_EQUALS: return ImGuiKey_Equal;
    // case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
    // case SDLK_BACKSLASH: return ImGuiKey_Backslash;
    // case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
    // case SDLK_GRAVE: return ImGuiKey_GraveAccent;
    case SDLK_CAPSLOCK:
        return ImGuiKey_CapsLock;
    case SDLK_SCROLLLOCK:
        return ImGuiKey_ScrollLock;
    case SDLK_NUMLOCKCLEAR:
        return ImGuiKey_NumLock;
    case SDLK_PRINTSCREEN:
        return ImGuiKey_PrintScreen;
    case SDLK_PAUSE:
        return ImGuiKey_Pause;
    case SDLK_LCTRL:
        return ImGuiKey_LeftCtrl;
    case SDLK_LSHIFT:
        return ImGuiKey_LeftShift;
    case SDLK_LALT:
        return ImGuiKey_LeftAlt;
    case SDLK_LGUI:
        return ImGuiKey_LeftSuper;
    case SDLK_RCTRL:
        return ImGuiKey_RightCtrl;
    case SDLK_RSHIFT:
        return ImGuiKey_RightShift;
    case SDLK_RALT:
        return ImGuiKey_RightAlt;
    case SDLK_RGUI:
        return ImGuiKey_RightSuper;
    case SDLK_APPLICATION:
        return ImGuiKey_Menu;
    case SDLK_0:
        return ImGuiKey_0;
    case SDLK_1:
        return ImGuiKey_1;
    case SDLK_2:
        return ImGuiKey_2;
    case SDLK_3:
        return ImGuiKey_3;
    case SDLK_4:
        return ImGuiKey_4;
    case SDLK_5:
        return ImGuiKey_5;
    case SDLK_6:
        return ImGuiKey_6;
    case SDLK_7:
        return ImGuiKey_7;
    case SDLK_8:
        return ImGuiKey_8;
    case SDLK_9:
        return ImGuiKey_9;
    case SDLK_A:
        return ImGuiKey_A;
    case SDLK_B:
        return ImGuiKey_B;
    case SDLK_C:
        return ImGuiKey_C;
    case SDLK_D:
        return ImGuiKey_D;
    case SDLK_E:
        return ImGuiKey_E;
    case SDLK_F:
        return ImGuiKey_F;
    case SDLK_G:
        return ImGuiKey_G;
    case SDLK_H:
        return ImGuiKey_H;
    case SDLK_I:
        return ImGuiKey_I;
    case SDLK_J:
        return ImGuiKey_J;
    case SDLK_K:
        return ImGuiKey_K;
    case SDLK_L:
        return ImGuiKey_L;
    case SDLK_M:
        return ImGuiKey_M;
    case SDLK_N:
        return ImGuiKey_N;
    case SDLK_O:
        return ImGuiKey_O;
    case SDLK_P:
        return ImGuiKey_P;
    case SDLK_Q:
        return ImGuiKey_Q;
    case SDLK_R:
        return ImGuiKey_R;
    case SDLK_S:
        return ImGuiKey_S;
    case SDLK_T:
        return ImGuiKey_T;
    case SDLK_U:
        return ImGuiKey_U;
    case SDLK_V:
        return ImGuiKey_V;
    case SDLK_W:
        return ImGuiKey_W;
    case SDLK_X:
        return ImGuiKey_X;
    case SDLK_Y:
        return ImGuiKey_Y;
    case SDLK_Z:
        return ImGuiKey_Z;
    case SDLK_F1:
        return ImGuiKey_F1;
    case SDLK_F2:
        return ImGuiKey_F2;
    case SDLK_F3:
        return ImGuiKey_F3;
    case SDLK_F4:
        return ImGuiKey_F4;
    case SDLK_F5:
        return ImGuiKey_F5;
    case SDLK_F6:
        return ImGuiKey_F6;
    case SDLK_F7:
        return ImGuiKey_F7;
    case SDLK_F8:
        return ImGuiKey_F8;
    case SDLK_F9:
        return ImGuiKey_F9;
    case SDLK_F10:
        return ImGuiKey_F10;
    case SDLK_F11:
        return ImGuiKey_F11;
    case SDLK_F12:
        return ImGuiKey_F12;
    case SDLK_F13:
        return ImGuiKey_F13;
    case SDLK_F14:
        return ImGuiKey_F14;
    case SDLK_F15:
        return ImGuiKey_F15;
    case SDLK_F16:
        return ImGuiKey_F16;
    case SDLK_F17:
        return ImGuiKey_F17;
    case SDLK_F18:
        return ImGuiKey_F18;
    case SDLK_F19:
        return ImGuiKey_F19;
    case SDLK_F20:
        return ImGuiKey_F20;
    case SDLK_F21:
        return ImGuiKey_F21;
    case SDLK_F22:
        return ImGuiKey_F22;
    case SDLK_F23:
        return ImGuiKey_F23;
    case SDLK_F24:
        return ImGuiKey_F24;
    case SDLK_AC_BACK:
        return ImGuiKey_AppBack;
    case SDLK_AC_FORWARD:
        return ImGuiKey_AppForward;
    default:
        break;
    }

    // Fallback to scancode
    switch (scancode) {
    case SDL_SCANCODE_GRAVE:
        return ImGuiKey_GraveAccent;
    case SDL_SCANCODE_MINUS:
        return ImGuiKey_Minus;
    case SDL_SCANCODE_EQUALS:
        return ImGuiKey_Equal;
    case SDL_SCANCODE_LEFTBRACKET:
        return ImGuiKey_LeftBracket;
    case SDL_SCANCODE_RIGHTBRACKET:
        return ImGuiKey_RightBracket;
    case SDL_SCANCODE_NONUSBACKSLASH:
        return ImGuiKey_Oem102;
    case SDL_SCANCODE_BACKSLASH:
        return ImGuiKey_Backslash;
    case SDL_SCANCODE_SEMICOLON:
        return ImGuiKey_Semicolon;
    case SDL_SCANCODE_APOSTROPHE:
        return ImGuiKey_Apostrophe;
    case SDL_SCANCODE_COMMA:
        return ImGuiKey_Comma;
    case SDL_SCANCODE_PERIOD:
        return ImGuiKey_Period;
    case SDL_SCANCODE_SLASH:
        return ImGuiKey_Slash;
    default:
        break;
    }
    return ImGuiKey_None;
}