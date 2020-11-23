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
#include <foe/imgui/base.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

TEST_CASE("ImGui State Add/Remove", "[foe][imgui]") {
    foeImGuiBase testBase, testBase2;
    foeImGuiState testState;

    SECTION("Adding nothing fails") { REQUIRE_FALSE(testState.addUI(nullptr)); }
    SECTION("Adding real element succeeds") {
        REQUIRE(testState.addUI(&testBase));
        REQUIRE(testState.addUI(&testBase2));

        SECTION("Re-adding same element fails") { REQUIRE_FALSE(testState.addUI(&testBase)); }

        SECTION("Removing first item") { testState.removeUI(&testBase); }
        SECTION("Removing second item") { testState.removeUI(&testBase2); }
        SECTION("Removing non-existant") { testState.removeUI(nullptr); }
    }
    SECTION("Removing non-existant item") { testState.removeUI(nullptr); }

    SECTION("Test that running UI calls all the sub base types") {
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        io.FontGlobalScale = 1.0f;
        io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
        io.DisplaySize = ImVec2(1024, 1024);
        uint8_t *p1;
        int p2, p3;
        io.Fonts->GetTexDataAsRGBA32(&p1, &p2, &p3);

        ImGui::NewFrame();

        REQUIRE(testState.addUI(&testBase));
        testState.runUI();

        ImGui::EndFrame();
        ImGui::DestroyContext();
    }
}