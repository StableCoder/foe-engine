// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "application.hpp"

int main(int argc, char **argv) {
    Application app;

    auto [continueRun, retVal] = app.initialize(argc, argv);
    if (continueRun) {
        retVal = app.mainloop();
    }
    app.deinitialize();

    return retVal;
}