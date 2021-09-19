/*
    Copyright (C) 2021 George Cave - gcave@stablecoder.ca

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <catch.hpp>
#include <foe/wsi/window.h>

#include <cstring>
#include <thread>

using namespace std::chrono_literals;

TEST_CASE("WSI-GLFW3 - Creating a single window") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test)});

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

    REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test1)});
    REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test2)});

    if (test1 != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test1);
    if (test2 != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test2);
}

TEST_CASE("WSI-GLFW - Idle window processing loop") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test)});

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

    REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test)});

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

    REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test)});
    CHECK(std::string_view{foeWsiWindowGetTitle(test)} == "test window");

    foeWsiWindowSetTitle(test, "new window title");
    CHECK(std::string_view{foeWsiWindowGetTitle(test)} == "new window title");

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}

TEST_CASE("WSI-GLFW3 - Visibility") {
    foeWsiWindow test{FOE_NULL_HANDLE};

    SECTION("Created hidden") {
        REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", false, &test)});

        CHECK_FALSE(foeWsiWindowVisible(test));

        SECTION("Change to visible") {
            foeWsiWindowShow(test);
            CHECK(foeWsiWindowVisible(test));
        }
    }
    SECTION("Created visible") {
        REQUIRE_FALSE(std::error_code{foeWsiCreateWindow(128, 128, "test window", true, &test)});

        CHECK(foeWsiWindowVisible(test));

        SECTION("Change to hidden") {
            foeWsiWindowHide(test);

            CHECK_FALSE(foeWsiWindowVisible(test));
        }
    }

    if (test != FOE_NULL_HANDLE)
        foeWsiDestroyWindow(test);
}