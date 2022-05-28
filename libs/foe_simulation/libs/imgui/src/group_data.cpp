/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/simulation/imgui/group_data.hpp>

#include <foe/ecs/id_to_string.hpp>
#include <foe/imgui/state.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include <array>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace {

std::array<char const *, 1> renderMenus{
    "View",
};

std::string groupValueToShortHexStr(foeIdGroupValue group) {
    constexpr int groupWidth = (foeIdNumGroupBits / 4) + ((foeIdNumGroupBits % 4) ? 1 : 0);

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(groupWidth) << std::uppercase << std::setfill('0') << group;

    return ss.str();
}

void renderImporterData(foeImporterBase *pImporter) {
    ImGui::Text("Name: %s", pImporter->name().c_str());
}

void renderIndexData(foeIdIndexGenerator *pIndexGenerator, char const *subGroupName) {
    foeResult result;
    foeIdIndex nextFreeIndex;
    std::vector<foeIdIndex> recyclableIndices;

    do {
        uint32_t count;
        pIndexGenerator->exportState(nullptr, &count, nullptr);

        recyclableIndices.resize(count);
        result = pIndexGenerator->exportState(&nextFreeIndex, &count, recyclableIndices.data());
        recyclableIndices.resize(count);
    } while (result.value != FOE_SUCCESS);

    ImGui::Text("Next Free Index: %s / %u", foeIdToString(nextFreeIndex).c_str(), nextFreeIndex);
    ImGui::Text("Num Recycled: %zu", recyclableIndices.size());

    ImGui::TextUnformatted("\nRecycled Indices:");
    ImGui::BeginChild(subGroupName, ImVec2(0, 80), true, 0);

    for (auto it : recyclableIndices) {
        ImGui::Text("%s / %u", foeIdToString(it).c_str(), it);
    }

    ImGui::EndChild();
}

} // namespace

foeSimulationImGuiGroupData::foeSimulationImGuiGroupData(foeSimulation *pSimulation) :
    mpSimulationState{pSimulation} {}

bool foeSimulationImGuiGroupData::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeSimulationImGuiGroupData::renderMenuElements,
                         foeSimulationImGuiGroupData::renderCustomUI, renderMenus.data(),
                         renderMenus.size());
}

void foeSimulationImGuiGroupData::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeSimulationImGuiGroupData::renderMenuElements,
                     foeSimulationImGuiGroupData::renderCustomUI, renderMenus.data(),
                     renderMenus.size());
}

bool foeSimulationImGuiGroupData::renderMenuElements(ImGuiContext *pImGuiContext,
                                                     void *pUserData,
                                                     char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeSimulationImGuiGroupData *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeSimulationImGuiGroupData::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeSimulationImGuiGroupData *>(pUserData);

    pData->customUI();
}

bool foeSimulationImGuiGroupData::viewMainMenu() {
    if (ImGui::Selectable("Entity Group List", mOpen)) {
        mOpen = true;
        mFocus = true;
    }

    return true;
}

void foeSimulationImGuiGroupData::customUI() {
    if (!mOpen)
        return;

    if (mFocus) {
        ImGui::SetNextWindowFocus();
        mFocus = false;
    }

    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiWindowFlags_NoScrollbar);
    if (!ImGui::Begin("Entity Groups", &mOpen)) {
        ImGui::End();
        return;
    }

    { // Left Pane - Selection
        ImGui::Text("Data Groups:");
        ImGui::BeginChild("left pane", ImVec2(250, 0), false);
        ImGui::BeginTable("DataGroupTable", 3,
                          ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuter |
                              ImGuiTableFlags_SizingStretchProp);

        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Name");
        ImGui::TableHeadersRow();

        { // Temporary
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable(std::to_string(foeIdTemporaryGroupValue).c_str(),
                                  mSelected == foeIdTemporaryGroup)) {
                mSelected = foeIdTemporaryGroup;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable(groupValueToShortHexStr(foeIdTemporaryGroupValue).c_str(),
                                  mSelected == foeIdTemporaryGroup)) {
                mSelected = foeIdTemporaryGroup;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Temporary", mSelected == foeIdTemporaryGroup)) {
                mSelected = foeIdTemporaryGroup;
            }
        }

        { // Persistent
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable(std::to_string(foeIdPersistentGroupValue).c_str(),
                                  mSelected == foeIdPersistentGroup)) {
                mSelected = foeIdPersistentGroup;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable(groupValueToShortHexStr(foeIdPersistentGroupValue).c_str(),
                                  mSelected == foeIdPersistentGroup)) {
                mSelected = foeIdPersistentGroup;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Persistent", mSelected == foeIdPersistentGroup)) {
                mSelected = foeIdPersistentGroup;
            }
        }

        for (foeIdGroupValue gv = 0; gv < foeIdMaxDynamicGroupValue; ++gv) {
            foeIdGroup group = foeIdValueToGroup(gv);

            auto *pImporter = mpSimulationState->groupData.importer(group);
            if (pImporter != nullptr) {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                if (ImGui::Selectable(std::to_string(gv).c_str(), mSelected == group)) {
                    mSelected = group;
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(groupValueToShortHexStr(gv).c_str(), mSelected == group)) {
                    mSelected = group;
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(pImporter->name().c_str(), mSelected == group)) {
                    mSelected = group;
                }
            }
        }

        ImGui::EndTable();
        ImGui::EndChild();
    }

    { // Right Pane - Information
        ImGui::SameLine();
        ImGui::BeginGroup();

        ImGui::Text("Group Value: %u", foeIdGroupToValue(mSelected));
        ImGui::Text("GroupID: %s", foeIdToString(mSelected).c_str());

        auto *pImporter = mpSimulationState->groupData.importer(mSelected);
        if (pImporter != nullptr) {
            ImGui::Separator();
            ImGui::Text("Importer Data");

            renderImporterData(pImporter);
        }

        auto *pEntityIndices = mpSimulationState->groupData.entityIndices(mSelected);
        if (pEntityIndices != nullptr) {
            ImGui::Separator();
            ImGui::Text("Entity Index Data");

            renderIndexData(pEntityIndices, "EntityIndices");
        }

        auto *pResourceIndices = mpSimulationState->groupData.resourceIndices(mSelected);
        if (pResourceIndices != nullptr) {
            ImGui::Separator();
            ImGui::Text("Resource Index Data");

            renderIndexData(pResourceIndices, "ResourceIndices");
        }

        ImGui::EndGroup();
    }

    ImGui::End();
}