// Copyright (C) 2021-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/wsi/window.h>

#include <cstring>
#include <thread>

using namespace std::chrono_literals;

TEST_CASE("WSI-GLFW3 - Creating a single window") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test).value == FOE_SUCCESS);

    CHECK_FALSE(foeWsiWindowGetShouldClose(test));

    int width = 0, height = 0;
    foeWsiWindowGetSize(test, &width, &height);
    CHECK(width == 128);
    CHECK(height == 128);

    CHECK_FALSE(foeWsiWindowResized(test));

    CHECK(foeWsiGetKeyboard(test) != nullptr);
    CHECK(foeWsiGetMouse(test) != nullptr);

    float scaleX = 0.f, scaleY = 0.f;
    foeWsiWindowGetContentScale(test, &scaleX, &scaleY);
    CHECK(scaleX > 0.f);
    CHECK(scaleY > 0.f);

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}

TEST_CASE("WSI-GLFW3 - Creating multiple windows") {
    foeWsiWindow test1{FOE_NULL_HANDLE}, test2{FOE_NULL_HANDLE};

    REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test1).value == FOE_SUCCESS);
    REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test2).value == FOE_SUCCESS);

    if (test1 != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test1);
    if (test2 != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test2);
}

TEST_CASE("WSI-GLFW - Idle window processing loop") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test).value == FOE_SUCCESS);

    for (int i = 0; i < 25; ++i) {
        foeWsiWindowProcessing(test);
        foeWsiGlobalProcessing();
        std::this_thread::sleep_for(10ms);
    }

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}

TEST_CASE("WSI-GLFW - Resizing Window") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test).value == FOE_SUCCESS);

    foeWsiWindowResize(test, 256, 256);

    // Wait for the window to be resized by the windowing system
    for (int i = 0; i < 25; ++i) {
        foeWsiWindowProcessing(test);
        foeWsiGlobalProcessing();
        if (foeWsiWindowResized(test)) {
            break;
        }
        std::this_thread::sleep_for(10ms);
    }

    CHECK(foeWsiWindowResized(test));

    int width = 0, height = 0;
    foeWsiWindowGetSize(test, &width, &height);
    CHECK(width == 256);
    CHECK(height == 256);

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}

TEST_CASE("WSI-GLFW - Window Title") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test).value == FOE_SUCCESS);
    CHECK(std::string_view{foeWsiWindowGetTitle(test)} == "test window");

    foeWsiWindowSetTitle(test, "new window title");
    CHECK(std::string_view{foeWsiWindowGetTitle(test)} == "new window title");

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}

TEST_CASE("WSI-GLFW3 - Visibility") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    SECTION("Created hidden") {
        REQUIRE(foeWsiCreateWindow(128, 128, "test window", false, &test).value == FOE_SUCCESS);

        CHECK_FALSE(foeWsiWindowVisible(test));

        SECTION("Change to visible") {
            foeWsiWindowShow(test);
            CHECK(foeWsiWindowVisible(test));
        }
    }
    SECTION("Created visible") {
        REQUIRE(foeWsiCreateWindow(128, 128, "test window", true, &test).value == FOE_SUCCESS);

        CHECK(foeWsiWindowVisible(test));

        SECTION("Change to hidden") {
            foeWsiWindowHide(test);

            CHECK_FALSE(foeWsiWindowVisible(test));
        }
    }

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}