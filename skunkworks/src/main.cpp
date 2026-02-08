// Copyright (C) 2020-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "application.hpp"

int main(int argc, char **argv) {
    Application app;

    int retVal = app.initialize(argc, argv);
    if (retVal != 0)
        return retVal;

    retVal = app.mainloop();

    app.deinitialize();

    return retVal;
}