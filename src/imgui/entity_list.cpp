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

#include "entity_list.hpp"

#include <foe/ecs/editor_name_map.hpp>
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

foeImGuiEntityList::foeImGuiEntityList(foeSimulation *pSimulation,
                                       foeSimulationImGuiRegistrar *pRegistrar) :
    mpSimulationState{pSimulation}, mpRegistrar{pRegistrar} {}

bool foeImGuiEntityList::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiEntityList::renderMenuElements,
                         foeImGuiEntityList::renderCustomUI, renderMenus.data(),
                         renderMenus.size());
}

void foeImGuiEntityList::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiEntityList::renderMenuElements,
                     foeImGuiEntityList::renderCustomUI, renderMenus.data(), renderMenus.size());
}

bool foeImGuiEntityList::renderMenuElements(ImGuiContext *pImGuiContext,
                                            void *pUserData,
                                            char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiEntityList *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiEntityList::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiEntityList *>(pUserData);

    pData->customUI();
}

bool foeImGuiEntityList::viewMainMenu() {
    if (ImGui::Selectable("Entity List", mOpen)) {
        mOpen = true;
        mFocus = true;
    }

    return true;
}

void foeImGuiEntityList::customUI() {
    if (!mOpen) {
        displayOpenEntities();
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
        auto *pIndexGenerator = mpSimulationState->groupData.entityIndices(group);
        if (pIndexGenerator == nullptr)
            continue;

        foeResult result;
        PerGroupData indiceData{
            .group = group,
        };

        do {
            uint32_t count;
            pIndexGenerator->exportState(nullptr, &count, nullptr);

            indiceData.recycledIndices.resize(count);
            result = pIndexGenerator->exportState(&indiceData.nextIndex, &count,
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
    ImGui::Begin("Entity List", &mOpen);

    ImGui::BeginTable("EntityListTable", 2,
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

            foeEntityID entity = foeIdCreate(currentGroupData->group, index);
            EntityDisplayData *pDisplayData{nullptr};
            bool selected{false};

            // See if the entity already has a window displaying it's data
            for (auto &it : mDisplayed) {
                if (entity == it.entity) {
                    pDisplayData = &it;
                    break;
                }
            }

            ImGui::TableNextRow();

            // ID
            ImGui::TableNextColumn();
            if (ImGui::Selectable(foeIdToString(entity).c_str(), pDisplayData != nullptr)) {
                selected = true;
            }

            // EditorName
            ImGui::TableNextColumn();
            std::string editorName = (mpSimulationState->pEntityNameMap)
                                         ? mpSimulationState->pEntityNameMap->find(entity)
                                         : "";
            if (ImGui::Selectable(editorName.c_str(), pDisplayData != nullptr)) {
                selected = true;
            }

            if (selected) {
                if (pDisplayData != nullptr) {
                    pDisplayData->focus = true;
                } else {
                    mDisplayed.emplace_back(EntityDisplayData{
                        .entity = entity,
                        .open = true,
                        .focus = true,
                    });
                }
            }
        }
    }

    ImGui::EndTable();

    ImGui::End();

    displayOpenEntities();
}

void foeImGuiEntityList::displayOpenEntities() {
    for (auto it = mDisplayed.begin(); it != mDisplayed.end();) {
        if (!displayEntity(&(*it))) {
            // The window has been closed
            it = mDisplayed.erase(it);
        } else {
            ++it;
        }
    }
}

bool foeImGuiEntityList::displayEntity(EntityDisplayData *pData) {
    if (!pData->open) {
        return false;
    }

    ImGui::SetNextWindowSize(ImVec2(0, 0), 0);

    if (pData->focus) {
        ImGui::SetNextWindowFocus();
        pData->focus = false;
    }

    std::string windowTitle = "Entity " + foeIdToString(pData->entity);

    if (!ImGui::Begin(windowTitle.data(), &pData->open, 0)) {
        ImGui::End();
        return false;
    }

    // EditorID
    if (mpSimulationState->pEntityNameMap != nullptr) {
        ImGui::Text("EditorID: %s", mpSimulationState->pEntityNameMap->find(pData->entity).data());
    }

    mpRegistrar->displayEntity(pData->entity, mpSimulationState);

    ImGui::End();

    return true;
}
