// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "qt_vulkan_window.hpp"

#include <QPlatformSurfaceEvent>
#include <foe/utf_string_conversion.h>

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../window_surface.hpp"

foeQtVulkanWindow::foeQtVulkanWindow(QGuiApplication *pGuiApp,
                                     std::atomic_bool *pRenderWindowFlag,
                                     std::atomic_bool *pNeedSwapchainRebuild,
                                     QWindow *parent) :
    QWindow(parent),
    pGuiApplication{pGuiApp},
    pRenderWindowFlag{pRenderWindowFlag},
    pNeedSwapchainRebuild{pNeedSwapchainRebuild} {
    setSurfaceType(QSurface::VulkanSurface);
}

foeQtVulkanWindow::~foeQtVulkanWindow() {}

void foeQtVulkanWindow::processKeyboardEvents(KeyboardInput *pKeyboard) {
    inputSync.lock();
    auto keyboardQueue = std::move(keyboardEvents);
    inputSync.unlock();

    pKeyboard->preprocessing();

    while (!keyboardQueue.empty()) {
        auto const event = keyboardQueue.front();
        keyboardQueue.pop();

        if (event.type == QEvent::KeyPress) {
            pKeyboard->pressedCodes.emplace_back(event.keycode, event.scancode);
            pKeyboard->downCodes.emplace_back(event.keycode, event.scancode);

            pKeyboard->unicodeChar = event.unicodeCodepoint;
        } else if (event.type == QEvent::KeyRelease) {
            // remove the code pair form the set of held-down codes
            bool codeFound = false;
            auto const endIt = pKeyboard->downCodes.end();
            for (auto it = pKeyboard->downCodes.begin(); it != endIt; ++it) {
                if (it->keycode == event.keycode && it->scancode == event.scancode) {
                    pKeyboard->downCodes.erase(it);
                    codeFound = true;
                    break;
                }
            }

            // if the exact same code pair could not be found, meaning it wasn't entered
            // previously, then something is very wrong and needs to be fixed
            assert(codeFound);
        }
    }
}

void foeQtVulkanWindow::processMouseEvents(MouseInput *pMouse) {
    inputSync.lock();
    auto mouseQueue = std::move(mouseEvents);
    inputSync.unlock();

    pMouse->preprocessing();

    while (!mouseQueue.empty()) {
        auto const event = mouseQueue.front();
        mouseQueue.pop();

        if (event.type == QEvent::MouseButtonPress) {
            pMouse->downButtons.insert(event.button);
            pMouse->pressedButtons.insert(event.button);
        } else if (event.type == QEvent::MouseButtonRelease) {
            pMouse->releasedButtons.insert(event.button);
            pMouse->downButtons.erase(event.button);
        } else if (event.type == QEvent::MouseMove) {
            pMouse->position.x = event.position.x;
            pMouse->position.y = event.position.y;
        } else if (event.type == QEvent::Enter) {
            pMouse->inWindow = true;
        } else if (event.type == QEvent::Leave) {
            pMouse->inWindow = false;
        }
    }
}

bool foeQtVulkanWindow::event(QEvent *e) {
    switch (e->type()) {
    case QEvent::Resize:
        *pNeedSwapchainRebuild = true;
        break;

    case QEvent::Expose:
        if (isExposed())
            *pRenderWindowFlag = true;
        else
            *pRenderWindowFlag = false;
        break;

    case QEvent::Show:
        *pRenderWindowFlag = false;
        break;

    case QEvent::Hide:
        *pRenderWindowFlag = false;
        break;

    case QEvent::Paint:
    case QEvent::UpdateRequest:
        printf("paint/updateRequest\n");
        *pRenderWindowFlag = true;
        break;

    case QEvent::PlatformSurface:
        // destroy the swapchain and surface here
        printf("platformSurface\n");
        if (static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() ==
            QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
            printf("SurfaceAboutToBeDestroyed\n");
            *pRenderWindowFlag = false;
        }
        break;

    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        QKeyEvent const *event = static_cast<QKeyEvent *>(e);
        uint32_t unicodeCodepoint = 0;
        if (e->type() == QEvent::KeyPress) {
            // get unicode char
            std::string text = event->text().toStdString();
            size_t srcCount = text.size();
            size_t dstCount = 1;

            foeResult result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)text.c_str(),
                                                 &dstCount, &unicodeCodepoint);
        }

        inputSync.lock();
        keyboardEvents.emplace(KeyboardEvent{
            .type = e->type(),
            .keycode = (uint32_t)event->key(),
            .scancode = event->nativeScanCode(),
            .unicodeCodepoint = unicodeCodepoint,
        });
        inputSync.unlock();
    } break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease: {
        QMouseEvent const *event = static_cast<QMouseEvent *>(e);
        inputSync.lock();
        mouseEvents.emplace(MouseEvent{
            .type = e->type(),
            .button = event->button(),
        });
        inputSync.unlock();
    } break;

    case QEvent::MouseMove: {
        QMouseEvent const *event = static_cast<QMouseEvent *>(e);
        inputSync.lock();
        mouseEvents.emplace(MouseEvent{
            .type = QEvent::MouseMove,
            .position =
                {
                    .x = event->position().x(),
                    .y = event->position().y(),
                },
        });
        inputSync.unlock();
    } break;

    case QEvent::Enter:
    case QEvent::Leave: {
        inputSync.lock();
        mouseEvents.emplace(MouseEvent{
            .type = e->type(),
        });
        inputSync.unlock();
    } break;

    default:
        break;
    }

    return QWindow::event(e);
}