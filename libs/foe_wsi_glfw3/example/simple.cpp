// Copyright (C) 2020-2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/wsi/window.h>

int main() {
    foeWsiWindow window;
    foeWsiCreateWindow(1280, 720, "Simple Example", false, &window);

    while (!foeWsiWindowGetShouldClose(window)) {
        foeWsiWindowProcessing(window);
        foeWsiGlobalProcessing();

        auto *pMouse = foeWsiGetMouse(window);
        auto *pKeyboard = foeWsiGetKeyboard(window);
    }

    foeWsiDestroyWindow(window);

    return 0;
}