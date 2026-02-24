// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef QT_VULKAN_WINDOW_HPP
#define QT_VULKAN_WINDOW_HPP

#include <QWindow>

#include <mutex>
#include <queue>

class QGuiApplication;
struct KeyboardInput;
struct MouseInput;
struct WindowSurfaceData;

struct KeyboardEvent {
    enum QEvent::Type type;
    uint32_t keycode;
    uint32_t scancode;
    uint32_t unicodeCodepoint;
};

struct MouseEvent {
    struct Vec2 {
        double x;
        double y;
    };

    enum QEvent::Type type;
    enum Qt::MouseButton button;
    Vec2 position;
};

class foeQtVulkanWindow : public QWindow {
  public:
    foeQtVulkanWindow(QGuiApplication *pGuiApp,
                      std::atomic_bool *pRenderWindowFlag,
                      std::atomic_bool *pNeedSwapchainRebuild,
                      QWindow *parent = nullptr);
    ~foeQtVulkanWindow();

    void processKeyboardEvents(KeyboardInput *pKeyboard);
    void processMouseEvents(MouseInput *pMouse);

  Q_SIGNALS:
  protected:
    bool event(QEvent *e) override;

  private:
    Q_DISABLE_COPY(foeQtVulkanWindow)

    QGuiApplication *const pGuiApplication;

    std::atomic_bool *pRenderWindowFlag;
    std::atomic_bool *pNeedSwapchainRebuild;

    std::mutex inputSync;
    std::queue<KeyboardEvent> keyboardEvents;
    std::queue<MouseEvent> mouseEvents;
};

#endif // QT_VULKAN_WINDOW_HPP