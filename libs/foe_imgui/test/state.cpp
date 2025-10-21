// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/external/imgui.h>
#include <foe/imgui/state.hpp>

#include <array>

namespace {

std::array<char const *, 4> menuArr{"File", "Edit", "Custom", "Help"};

char const *testStr = "Test";
int testContextVar;

bool menuFn(ImGuiContext *pImGuiContext, void *pContext, char const *pMenuName) {
    auto *pCount = static_cast<int *>(pContext);

    std::string_view menu{pMenuName};
    for (auto const &it : menuArr) {
        if (menu == it) {
            (*pCount)++;
            return true;
        }
    }

    return false;
}

void customFn(ImGuiContext *pImGuiContext, void *pContext) {}

auto *pContext = ImGui::CreateContext();

} // namespace

TEST_CASE("ImGui State Add/Remove", "[foe][imgui]") {
    foeImGuiState testState;

    testState.setImGuiContext(pContext);
    ImGui::SetCurrentContext(pContext);

    SECTION("Adding no context/functions fails") {
        REQUIRE_FALSE(testState.addUI(nullptr, nullptr, nullptr, nullptr, 0));
        REQUIRE_FALSE(testState.addUI(nullptr, nullptr, nullptr, &testStr, 1));
    }
    SECTION("Adding real unique element combinations succeeds") {
        REQUIRE(testState.addUI(&testContextVar, nullptr, nullptr, nullptr, 0));
        REQUIRE(testState.addUI(nullptr, menuFn, nullptr, nullptr, 0));
        REQUIRE(testState.addUI(nullptr, nullptr, customFn, nullptr, 0));
        REQUIRE(testState.addUI(nullptr, menuFn, customFn, nullptr, 0));
        REQUIRE(testState.addUI(&testContextVar, menuFn, customFn, nullptr, 0));

        SECTION("Re-adding same element fails") {
            REQUIRE_FALSE(testState.addUI(&testContextVar, nullptr, nullptr, nullptr, 0));
            REQUIRE_FALSE(testState.addUI(nullptr, menuFn, nullptr, nullptr, 0));
            REQUIRE_FALSE(testState.addUI(nullptr, nullptr, customFn, nullptr, 0));
            REQUIRE_FALSE(testState.addUI(nullptr, menuFn, customFn, nullptr, 0));
            REQUIRE_FALSE(testState.addUI(&testContextVar, menuFn, customFn, nullptr, 0));
        }

        SECTION("Re-adding same element with different menu strings fails") {
            REQUIRE_FALSE(testState.addUI(&testContextVar, nullptr, nullptr, &testStr, 1));
            REQUIRE_FALSE(testState.addUI(nullptr, menuFn, nullptr, &testStr, 1));
            REQUIRE_FALSE(testState.addUI(nullptr, nullptr, customFn, &testStr, 1));
            REQUIRE_FALSE(testState.addUI(nullptr, menuFn, customFn, &testStr, 1));
            REQUIRE_FALSE(testState.addUI(&testContextVar, menuFn, customFn, &testStr, 1));
        }

        SECTION("Removing items") {
            testState.addUI(&testContextVar, nullptr, nullptr, nullptr, 0);
            testState.addUI(nullptr, menuFn, nullptr, nullptr, 0);
            testState.addUI(nullptr, nullptr, customFn, nullptr, 0);
            testState.addUI(nullptr, menuFn, customFn, nullptr, 0);
            testState.addUI(&testContextVar, menuFn, customFn, nullptr, 0);
        }

        SECTION("Removing non-existant") {
            testState.addUI(nullptr, nullptr, nullptr, nullptr, 0);
            testState.addUI(nullptr, nullptr, nullptr, &testStr, 1);
        }
    }
    SECTION("Removing non-existing items causes no harm") {
        testState.addUI(nullptr, nullptr, nullptr, nullptr, 0);
        testState.addUI(nullptr, nullptr, nullptr, &testStr, 1);
    }

    SECTION("Test that running UI calls all the sub base types") {
        ImGuiIO &io = ImGui::GetIO();
        io.FontGlobalScale = 1.0f;
        io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
        io.DisplaySize = ImVec2(1024, 1024);
        uint8_t *p1;
        int p2, p3;
        io.Fonts->GetTexDataAsRGBA32(&p1, &p2, &p3);

        ImGui::NewFrame();

        int callCount = 0;

        REQUIRE(testState.addUI(&callCount, menuFn, customFn, menuArr.data(), menuArr.size()));
        testState.runUI();

        ImGui::EndFrame();
        ImGui::DestroyContext();
    }
}