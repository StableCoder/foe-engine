// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <GLFW/glfw3.h>
#include <foe/wsi/keyboard.hpp>
#include <foe/wsi/mouse.hpp>
#include <foe/wsi/window.h>

#include "result.h"
#include "window.hpp"

#include <mutex>
#include <vector>

namespace {

std::mutex windowSync;
std::vector<foeWsiWindowGLFW *> windowList;

void windowPreprocessing(foeWsiWindowGLFW *pWindow) { pWindow->resized = false; }

void windowResizedCallback(GLFWwindow *pWindow, int, int) {
    auto *pWsiWindow = static_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    pWsiWindow->resized = true;
}

void keyCallback(GLFWwindow *pWindow, int key, int scancode, int action, int mods) {
    auto *pWsiWindow = static_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    keyCallback(&pWsiWindow->keyboard, key, scancode, action, mods);
}

void charCallback(GLFWwindow *pWindow, unsigned int codepoint) {
    auto *pWsiWindow = static_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    charCallback(&pWsiWindow->keyboard, codepoint);
}

void positionCallback(GLFWwindow *pWindow, double xPos, double yPos) {
    auto *pWsiWindow = static_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    cursorPositionCallback(&pWsiWindow->mouse, xPos, yPos);
}

void cursorEnterCallback(GLFWwindow *pWindow, int entered) {
    auto *pWsiWindow = static_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    cursorEnterCallback(&pWsiWindow->mouse, entered);
}

void scrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset) {
    auto *pWsiWindow = static_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    scrollCallback(&pWsiWindow->mouse, xOffset, yOffset);
}

void buttonCallback(GLFWwindow *pWindow, int button, int action, int mods) {
    auto *pWsiWindow = reinterpret_cast<foeWsiWindowGLFW *>(glfwGetWindowUserPointer(pWindow));

    buttonCallback(&pWsiWindow->mouse, button, action, mods);
}

foeResultSet foeWsiCreateWindowErrC(
    int width, int height, char const *pTitle, bool visible, foeWsiWindow *pWindow) {
    if (!glfwInit()) {
        return to_foeResult(FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND);
    }
    foeResultSet result = to_foeResult(FOE_WSI_SUCCESS);

    // Since this is exclusively a Vulkan platform, don't initialize OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (visible) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    foeWsiWindowGLFW *pNewWindow = new foeWsiWindowGLFW{
        .title = pTitle,
    };

    pNewWindow->pWindow = glfwCreateWindow(width, height, pTitle, nullptr, nullptr);
    if (pNewWindow->pWindow == nullptr) {
        result = to_foeResult(FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW);
        goto CREATE_FAILED;
    }

    // Set User Data Pointer
    glfwSetWindowUserPointer(pNewWindow->pWindow, pNewWindow);

    // Keyboard Callbacks
    glfwSetKeyCallback(pNewWindow->pWindow, keyCallback);
    glfwSetCharCallback(pNewWindow->pWindow, charCallback);

    // Mouse Callbacks
    glfwSetCursorPosCallback(pNewWindow->pWindow, positionCallback);
    glfwSetCursorEnterCallback(pNewWindow->pWindow, cursorEnterCallback);
    glfwSetScrollCallback(pNewWindow->pWindow, scrollCallback);
    glfwSetMouseButtonCallback(pNewWindow->pWindow, buttonCallback);

    // Window Callbacks
    glfwSetWindowSizeCallback(pNewWindow->pWindow, windowResizedCallback);

CREATE_FAILED:
    if (result.value != FOE_SUCCESS) {
        foeWsiDestroyWindow(window_to_handle(pNewWindow));
    } else {
        *pWindow = window_to_handle(pNewWindow);

        std::scoped_lock lock{windowSync};
        windowList.emplace_back(pNewWindow);
    }

    return result;
}

} // namespace

void foeWsiGlobalProcessing() {
    std::scoped_lock lock{windowSync};

    for (auto &pWindow : windowList) {
        keyboardPreprocessing(&pWindow->keyboard);
        mousePreprocessing(&pWindow->mouse);
        windowPreprocessing(pWindow);
    }

    glfwPollEvents();
}

foeResultSet foeWsiCreateWindow(
    int width, int height, char const *pTitle, bool visible, foeWsiWindow *pWindow) {
    return foeWsiCreateWindowErrC(width, height, pTitle, visible, pWindow);
}

void foeWsiDestroyWindow(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    windowSync.lock();
    for (auto it = windowList.begin(); it != windowList.end(); ++it) {
        if (*it == pWindow) {
            windowList.erase(it);
            break;
        }
    }
    windowSync.unlock();

    glfwDestroyWindow(pWindow->pWindow);

    delete pWindow;
}

void foeWsiWindowProcessing(foeWsiWindow window) {}

char const *foeWsiWindowGetTitle(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    return pWindow->title.data();
}

void foeWsiWindowSetTitle(foeWsiWindow window, char const *pTitle) {
    auto *pWindow = window_from_handle(window);

    pWindow->title = pTitle;
    glfwSetWindowTitle(pWindow->pWindow, pTitle);
}

bool foeWsiWindowGetShouldClose(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    return glfwWindowShouldClose(pWindow->pWindow);
}

void foeWsiWindowSetShouldClose(foeWsiWindow window, bool terminate) {
    auto *pWindow = window_from_handle(window);

    glfwSetWindowShouldClose(pWindow->pWindow, terminate ? GLFW_TRUE : GLFW_FALSE);
}

void foeWsiWindowGetSize(foeWsiWindow window, int *pWidth, int *pHeight) {
    auto *pWindow = window_from_handle(window);

    glfwGetWindowSize(pWindow->pWindow, pWidth, pHeight);
}

bool foeWsiWindowResized(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    return pWindow->resized;
}

void foeWsiWindowResize(foeWsiWindow window, int width, int height) {
    auto *pWindow = window_from_handle(window);

    glfwSetWindowSize(pWindow->pWindow, width, height);
}

bool foeWsiWindowVisible(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    return glfwGetWindowAttrib(pWindow->pWindow, GLFW_VISIBLE) == 1;
}

void foeWsiWindowHide(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    glfwHideWindow(pWindow->pWindow);
}

void foeWsiWindowShow(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    glfwShowWindow(pWindow->pWindow);
}

foeWsiKeyboard const *foeWsiGetKeyboard(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    return &pWindow->keyboard;
}

foeWsiMouse const *foeWsiGetMouse(foeWsiWindow window) {
    auto *pWindow = window_from_handle(window);

    return &pWindow->mouse;
}

void foeWsiWindowGetContentScale(foeWsiWindow window, float *pScaleX, float *pScaleY) {
    auto *pWindow = window_from_handle(window);

    glfwGetWindowContentScale(pWindow->pWindow, pScaleX, pScaleY);
}