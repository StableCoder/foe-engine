/*
    Copyright (C) 2021 George Cave.

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
#include <foe/simulation/core.hpp>
#include <foe/simulation/state.hpp>

TEST_CASE("SimState - EditorNameMap not created when addNameMaps set to false") {
    auto *pSimState = foeCreateSimulation(false);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap == nullptr);
    REQUIRE(pSimState->pEntityNameMap == nullptr);

    foeDestroySimulation(pSimState);
}

TEST_CASE("SimState - EditorNameMap created when addNameMaps set to true") {
    auto *pSimState = foeCreateSimulation(true);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap != nullptr);
    REQUIRE(pSimState->pEntityNameMap != nullptr);

    foeDestroySimulation(pSimState);
}