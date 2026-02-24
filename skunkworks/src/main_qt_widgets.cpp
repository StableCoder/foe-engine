// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QApplication>

#include "application.hpp"

#include <thread>

// Widget Window Test
#include <QVBoxLayout>
#include <QWidget>

#include "wsi_qt/qt_vulkan_window.hpp"

#include <cassert>

class WidgetWindowTest : public QWidget {
  public:
    WidgetWindowTest(foeQtVulkanWindow *topWindow, foeQtVulkanWindow *bottomWindow) :
        pTopWindow{topWindow}, pBottomWindow{bottomWindow} {
        QWidget *wrapper = QWidget::createWindowContainer(topWindow);
        QWidget *wrapper2 = QWidget::createWindowContainer(bottomWindow);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(wrapper, 1);
        layout->addWidget(wrapper2, 1);
        setLayout(layout);
    }

  private:
    foeQtVulkanWindow *pTopWindow;
    foeQtVulkanWindow *pBottomWindow;
};

int main(int argc, char **argv) {
    QApplication qtApplication(argc, argv);
    WidgetWindowTest *pWidgetWindowTest = nullptr;
    Application skunkworksApplication;
    int skunkworksRetVal;

    skunkworksApplication.setQtGuiApplication(&qtApplication);

    { // Qt Widget Test Window
        std::vector<Application::ImportedQtWindowData> windowData;

        for (size_t i = 0; i < 2; ++i) {
            std::atomic_bool *pNeedRebuildSwapchain = new std::atomic_bool{true};
            std::atomic_bool *pActive = new std::atomic_bool{false};

            foeQtVulkanWindow *pWindow =
                new foeQtVulkanWindow{&qtApplication, pActive, pNeedRebuildSwapchain};

            windowData.emplace_back(Application::ImportedQtWindowData{
                .pQtWindow = pWindow,
                .pNeedSwapchainBuild = pNeedRebuildSwapchain,
                .pActive = pActive,
            });
        }

        pWidgetWindowTest = new WidgetWindowTest{windowData[0].pQtWindow, windowData[1].pQtWindow};

        pWidgetWindowTest->resize(1280, 720);
        pWidgetWindowTest->show();

        skunkworksApplication.setQtWindows(windowData);
    }

    std::thread mainThread([&] {
        skunkworksRetVal = skunkworksApplication.initialize(argc, argv);
        if (skunkworksRetVal == 0)
            skunkworksRetVal = skunkworksApplication.mainloop();
        skunkworksApplication.deinitialize();
    });

    int qtReturnVal = qtApplication.exec();

    mainThread.join();

    if (pWidgetWindowTest)
        delete pWidgetWindowTest;

    if (qtReturnVal != 0)
        return qtReturnVal;

    return skunkworksRetVal;
}