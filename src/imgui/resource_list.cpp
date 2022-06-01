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

#include "resource_list.hpp"

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/error_code.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/imgui/state.hpp>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> renderMenus{
    "View",
};

}

foeImGuiResourceList::foeImGuiResourceList(foeSimulation *pSimulation,
                                           foeSimulationImGuiRegistrar *pRegistrar) :
    mpSimulationState{pSimulation}, mpRegistrar{pRegistrar} {}

bool foeImGuiResourceList::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiResourceList::renderMenuElements,
                         foeImGuiResourceList::renderCustomUI, renderMenus.data(),
                         renderMenus.size());
}

void foeImGuiResourceList::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiResourceList::renderMenuElements,
                     foeImGuiResourceList::renderCustomUI, renderMenus.data(), renderMenus.size());
}

bool foeImGuiResourceList::renderMenuElements(ImGuiContext *pImGuiContext,
                                              void *pUserData,
                                              char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiResourceList *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiResourceList::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiResourceList *>(pUserData);

    pData->customUI();
}

bool foeImGuiResourceList::viewMainMenu() {
    if (ImGui::Selectable("Resource List", mOpen)) {
        mOpen = true;
        mFocus = true;
    }

    return true;
}

void foeImGuiResourceList::customUI() {
    if (!mOpen) {
        displayOpenResources();
        return;
    }

    if (mFocus) {
        ImGui::SetNextWindowFocus();
        mFocus = false;
    }

    // Get count of total entities to properly size the list
    size_t totalEntityCount = 0;

    struct PerGroupData {
        foeIdGroup group;
        foeIdIndex nextIndex;
        std::vector<foeIdIndex> recycledIndices;
    };
    std::vector<PerGroupData> groupIndiceData;
    groupIndiceData.reserve(foeIdNumDynamicGroups + 2);

    for (foeIdGroup groupValue = 0; groupValue <= foeIdGroupMaxValue; ++groupValue) {
        foeIdGroup group = foeIdValueToGroup(groupValue);

        foeEcsIndexes indexes = mpSimulationState->groupData.resourceIndexes(group);
        if (indexes == FOE_NULL_HANDLE)
            continue;

        foeResult result;
        PerGroupData indiceData{
            .group = group,
        };

        do {
            uint32_t count;
            foeEcsExportIndexes(indexes, nullptr, &count, nullptr);

            indiceData.recycledIndices.resize(count);
            result = foeEcsExportIndexes(indexes, &indiceData.nextIndex, &count,
                                         indiceData.recycledIndices.data());
            indiceData.recycledIndices.resize(count);
        } while (result.value != FOE_SUCCESS);
        std::sort(indiceData.recycledIndices.begin(), indiceData.recycledIndices.end());

        totalEntityCount +=
            indiceData.nextIndex - indiceData.recycledIndices.size() - foeIdIndexMinValue;
        groupIndiceData.emplace_back(std::move(indiceData));
    }

    // Start the List Window
    ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiWindowFlags_NoScrollbar);
    ImGui::Begin("Resource List", &mOpen);

    ImGui::BeginTable("ResourceListTable", 2,
                      ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                          ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                          ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);

    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("EditorName", ImGuiTableColumnFlags_None);
    ImGui::TableHeadersRow();

    // Total offset from start of all entities
    int currentOffset = 0;
    // Current selected group
    auto currentGroupData = groupIndiceData.begin();

    uint32_t strLength = 64;
    char *pTempStr = (char *)malloc(64);
    if (pTempStr == NULL)
        std::abort();

    ImGuiListClipper clipper;
    clipper.Begin(totalEntityCount);
    while (clipper.Step()) {
        for (int idx = clipper.DisplayStart; idx < clipper.DisplayEnd; ++idx) {
            while (idx >= currentOffset + currentGroupData->nextIndex -
                              currentGroupData->recycledIndices.size() - foeIdIndexMinValue) {
                currentOffset += currentGroupData->nextIndex -
                                 currentGroupData->recycledIndices.size() - foeIdIndexMinValue;
                ++currentGroupData;
            }

            foeIdIndex index = foeIdIndexMinValue + (idx - currentOffset);
            for (auto it : currentGroupData->recycledIndices) {
                if (it <= index)
                    ++index;
                else
                    break;
            }

            foeResourceID resourceID = foeIdCreate(currentGroupData->group, index);
            ResourceDisplayData *pDisplayData{nullptr};
            bool selected{false};

            // See if the entity already has a window displaying it's data
            for (auto &it : mDisplayed) {
                if (resourceID == it.resourceID) {
                    pDisplayData = &it;
                    break;
                }
            }

            ImGui::TableNextRow();

            // ID
            ImGui::TableNextColumn();
            if (ImGui::Selectable(foeIdToString(resourceID).c_str(), pDisplayData != nullptr)) {
                selected = true;
            }

            // EditorName
            ImGui::TableNextColumn();
            char const *pEditorName = "";
            if (mpSimulationState->pResourceNameMap != FOE_NULL_HANDLE) {
                uint32_t localStrLength = strLength;
                foeResult result;
                do {
                    result = mpSimulationState->pResourceNameMap->find(resourceID, &localStrLength,
                                                                       pTempStr);
                    if (result.value == FOE_ECS_INCOMPLETE) {
                        strLength = localStrLength;
                        pTempStr = (char *)realloc(pTempStr, localStrLength);
                        if (pTempStr == NULL)
                            std::abort();

                    } else if (result.value == FOE_ECS_SUCCESS) {
                        pEditorName = pTempStr;
                        break;
                    }
                } while (result.value != FOE_ECS_NO_MATCH);
            }

            if (ImGui::Selectable(pEditorName, pDisplayData != nullptr)) {
                selected = true;
            }

            if (selected) {
                if (pDisplayData != nullptr) {
                    pDisplayData->focus = true;
                } else {
                    mDisplayed.emplace_back(ResourceDisplayData{
                        .resourceID = resourceID,
                        .open = true,
                        .focus = true,
                    });
                }
            }
        }
    }

    ImGui::EndTable();

    ImGui::End();

    displayOpenResources();

    free(pTempStr);
}

void foeImGuiResourceList::displayOpenResources() {
    for (auto it = mDisplayed.begin(); it != mDisplayed.end();) {
        if (!displayResource(&(*it))) {
            // The window has been closed
            it = mDisplayed.erase(it);
        } else {
            ++it;
        }
    }
}

bool foeImGuiResourceList::displayResource(ResourceDisplayData *pData) {
    if (!pData->open) {
        return false;
    }

    ImGui::SetNextWindowSize(ImVec2(0, 0), 0);

    if (pData->focus) {
        ImGui::SetNextWindowFocus();
        pData->focus = false;
    }

    std::string windowTitle = "Entity " + foeIdToString(pData->resourceID);

    if (!ImGui::Begin(windowTitle.data(), &pData->open, 0)) {
        ImGui::End();
        return false;
    }

    // EditorID
    char *pEditorName = NULL;
    if (mpSimulationState->pResourceNameMap != nullptr) {
        uint32_t strLength = 0;
        foeResult result;
        do {
            result = mpSimulationState->pResourceNameMap->find(pData->resourceID, &strLength,
                                                               pEditorName);
            if (result.value == FOE_ECS_SUCCESS && pEditorName != NULL) {
                break;
            } else if ((result.value == FOE_ECS_SUCCESS && pEditorName == NULL) ||
                       result.value == FOE_ECS_INCOMPLETE) {
                pEditorName = (char *)realloc(pEditorName, strLength);
                if (pEditorName == NULL)
                    std::abort();
            }
        } while (result.value != FOE_ECS_NO_MATCH);
    }
    ImGui::Text("EditorID: %s", pEditorName);

    mpRegistrar->displayResource(pData->resourceID, mpSimulationState);

    ImGui::End();

    return true;
}
