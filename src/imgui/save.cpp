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

#include "save.hpp"

#include <foe/imex/yaml/exporter.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> menuNameArr{
    "File",
};

}

void foeImGuiSave::setSimulationState(foeSimulationState *pSimulationState) {
    mpSimulationState = pSimulationState;
}

void foeImGuiSave::clearSimulationState() { mpSimulationState = nullptr; }

bool foeImGuiSave::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiSave::renderMenuElements, foeImGuiSave::renderCustomUI,
                         menuNameArr.data(), menuNameArr.size());
}

void foeImGuiSave::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiSave::renderMenuElements, foeImGuiSave::renderCustomUI,
                     menuNameArr.data(), menuNameArr.size());
}

bool foeImGuiSave::renderMenuElements(void *pContext, char const *pMenuName) {
    auto *pData = reinterpret_cast<foeImGuiSave *>(pContext);
    std::string_view menuName{pMenuName};

    if (menuName == "File") {
        if (ImGui::MenuItem("Save", nullptr, false, pData->mpSimulationState != nullptr)) {
            foeYamlExporter yamlExporter;
            yamlExporter.exportState("testExport", pData->mpSimulationState);
        }

        return true;
    }

    return false;
}

void foeImGuiSave::renderCustomUI(void *pContext) {}