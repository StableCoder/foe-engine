/*
    Copyright (C) 2020 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <GLFW/glfw3.h>

#include <mutex>
#include <string>

#include "keyboard.hpp"
#include "mouse.hpp"

namespace {
std::once_flag glfwInitFlag;

GLFWwindow *pWindow = nullptr;
std::string title;
bool resized;

foeMouse mouse;
foeKeyboard keyboard;

void windowPreprocessing() { resized = false; }

void windowResizedCallback(GLFWwindow *, int, int) { resized = true; }

void keyCallback(GLFWwindow *, int key, int scancode, int action, int mods) {
    keyCallback(&keyboard, key, scancode, action, mods);
}

void charCallback(GLFWwindow *, unsigned int codepoint) { charCallback(&keyboard, codepoint); }

void positionCallback(GLFWwindow *, double xPos, double yPos) {
    cursorPositionCallback(&mouse, xPos, yPos);
}

void cursorEnterCallback(GLFWwindow *, int entered) { cursorEnterCallback(&mouse, entered); }

void scrollCallback(GLFWwindow *, double xOffset, double yOffset) {
    scrollCallback(&mouse, xOffset, yOffset);
}

void buttonCallback(GLFWwindow *, int button, int action, int mods) {
    buttonCallback(&mouse, button, action, mods);
}

} // namespace

bool foeCreateWindow(int width, int height, const char *pTitle) {
    // Return false if the window already exists
    if (pWindow != nullptr)
        return false;

    std::call_once(glfwInitFlag, [] { glfwInit(); });

    pWindow = glfwCreateWindow(width, height, pTitle, nullptr, nullptr);

    // Keyboard Callbacks
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetCharCallback(pWindow, charCallback);

    // Mouse Callbacks
    glfwSetCursorPosCallback(pWindow, positionCallback);
    glfwSetCursorEnterCallback(pWindow, cursorEnterCallback);
    glfwSetScrollCallback(pWindow, scrollCallback);
    glfwSetMouseButtonCallback(pWindow, buttonCallback);

    // Window Callbacks
    glfwSetWindowSizeCallback(pWindow, windowResizedCallback);

    return true;
}

void foeDestroyWindow() {
    glfwDestroyWindow(pWindow);
    pWindow = nullptr;
}

void foeWindowEventProcessing() {
    keyboardPreprocessing(&keyboard);
    mousePreprocessing(&mouse);
    windowPreprocessing();
    glfwPollEvents();
}

const char *foeWindowGetTitle() { return title.data(); }

void foeWindowSetTitle(const char *pTitle) {
    title = *pTitle;
    glfwSetWindowTitle(pWindow, pTitle);
}

bool foeWindowGetShouldClose() { return glfwWindowShouldClose(pWindow); }

void foeWindowSetShouldClose(bool terminate) {
    glfwSetWindowShouldClose(pWindow, terminate ? GLFW_TRUE : GLFW_FALSE);
}

void foeWindowGetSize(int *pWidth, int *pHeight) { glfwGetWindowSize(pWindow, pWidth, pHeight); }

bool foeWindowResized() { return resized; }

void foeWindowResize(int width, int height) { glfwSetWindowSize(pWindow, width, height); }

const foeMouse *foeGetMouse() { return &mouse; }
const foeKeyboard *foeGetKeyboard() { return &keyboard; }