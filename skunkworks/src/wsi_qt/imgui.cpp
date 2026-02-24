// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "imgui.hpp"

#include <QWindow>

#include "window.hpp"

namespace {

char const *windowBackend(void *pContext) { return "Qt"; }

std::string windowTitle(void *pContext) {
    Qt_WindowData *pWindowData = (Qt_WindowData *)pContext;

    return pWindowData->pWindow->title().toStdString();
}

bool windowTerminationCalled(void *pContext) {
    Qt_WindowData *pWindowData = (Qt_WindowData *)pContext;

    return pWindowData->close;
}

void windowLogicalSize(void *pContext, int *pWidth, int *pHeight) {
    Qt_WindowData *pWindowData = (Qt_WindowData *)pContext;

    QSize windowSize = pWindowData->pWindow->size();

    *pWidth = windowSize.width();
    *pHeight = windowSize.height();
}

void windowPixelSize(void *pContext, int *pWidth, int *pHeight) {
    Qt_WindowData *pWindowData = (Qt_WindowData *)pContext;

    QSize windowSize = pWindowData->pWindow->size();
    auto windowScale = pWindowData->pWindow->devicePixelRatio();
    windowSize *= windowScale;

    *pWidth = windowSize.width();
    *pHeight = windowSize.height();
}

void windowContentScale(void *pContext, float *pX, float *pY) {
    Qt_WindowData *pWindowData = (Qt_WindowData *)pContext;

    auto windowScale = pWindowData->pWindow->devicePixelRatio();

    *pX = windowScale;
    *pY = windowScale;
}

} // namespace

bool imguiAddQtWindow(foeImGuiWindow *pImguiWindow,
                      Qt_WindowData *pWindowData,
                      KeyboardInput const *pKeyboard,
                      MouseInput const *pMouse) {
    return pImguiWindow->addWindow(pWindowData, windowBackend, windowTitle, windowTerminationCalled,
                                   windowLogicalSize, windowPixelSize, windowContentScale,
                                   pKeyboard, pMouse);
}