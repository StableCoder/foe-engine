/*
    Copyright (C) 2020 George Cave.

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
#include <foe/plugin.h>

#ifndef TEST_PLUGIN_DIR
#define TEST_PLUGIN_DIR nullptr
#endif

static_assert(TEST_PLUGIN_DIR != nullptr,
              "TEST_PLUGIN_DIR must be defined with the location of the 'test_foe_plugin_so' "
              "shared library.");

TEST_CASE("Plugin (SO/DLL) doesn't exist, and creating it fails to provide a non-null handle",
          "[foe][plugin]") {
    foePlugin test{FOE_NULL_HANDLE};

    foeCreatePlugin("no-way-this-exists", &test);

    CHECK(test == FOE_NULL_HANDLE);
}

TEST_CASE("Plugin (SO/DLL) exists, and can be created", "[foe][plugin]") {
    foePlugin test{FOE_NULL_HANDLE};

    foeCreatePlugin(TEST_PLUGIN_DIR, &test);

    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("Can find the test function 'testFunc'") {
        int (*testFunc)(int) = reinterpret_cast<int (*)(int)>(foeGetPluginSymbol(test, "testFunc"));

        REQUIRE(testFunc != nullptr);

        CHECK(testFunc(10) == 100);
    }

    SECTION("Can't find a symbol that doesn't exist") {
        void *testFunc = foeGetPluginSymbol(test, "testFunc_doesnt_exist");
        CHECK(testFunc == nullptr);
    }

    foeDestroyPlugin(test);
}