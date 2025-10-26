// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "entity_list.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/name_map.h>
#include <foe/ecs/result.h>
#include <foe/external/imgui.h>
#include <foe/imgui/state.hpp>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>

#include <algorithm>
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
        foeEcsIndexes indexes = mpSimulationState->groupData.entityIndexes(group);
        if (indexes == FOE_NULL_HANDLE)
            continue;

        foeResultSet result;
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
            char const *pEditorName = "";
            if (mpSimulationState->entityNameMap != FOE_NULL_HANDLE) {
                uint32_t localStrLength = strLength;
                foeResultSet result;
                do {
                    result = foeEcsNameMapFindName(mpSimulationState->entityNameMap, entity,
                                                   &localStrLength, pTempStr);
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

    free(pTempStr);
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
    char *pEditorName = NULL;
    if (mpSimulationState->entityNameMap != nullptr) {
        uint32_t strLength = 0;
        foeResultSet result;
        do {
            result = foeEcsNameMapFindName(mpSimulationState->entityNameMap, pData->entity,
                                           &strLength, pEditorName);
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
    free(pEditorName);

    mpRegistrar->displayEntity(pData->entity, mpSimulationState);

    ImGui::End();

    return true;
}
