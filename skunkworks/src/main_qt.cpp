// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>

#include "application.hpp"

#include <thread>

int main(int argc, char **argv) {
    QGuiApplication qtApplication(argc, argv);
    Application skunkworksApplication;
    int skunkworksRetVal;

    skunkworksApplication.setQtGuiApplication(&qtApplication);

    std::thread mainThread([&] {
        skunkworksRetVal = skunkworksApplication.initialize(argc, argv);
        if (skunkworksRetVal == 0)
            skunkworksRetVal = skunkworksApplication.mainloop();
        skunkworksApplication.deinitialize();
    });

    int qtReturnVal = qtApplication.exec();

    mainThread.join();

    if (qtReturnVal != 0)
        return qtReturnVal;

    return skunkworksRetVal;
}