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

#include "resource_list.hpp"

#include <foe/ecs/editor_name_map.hpp>
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

foeImGuiResourceList::foeImGuiResourceList(foeSimulationState *pSimulationState,
                                           foeSimulationImGuiRegistrar *pRegistrar) :
    mpSimulationState{pSimulationState}, mpRegistrar{pRegistrar} {}

bool foeImGuiResourceList::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiResourceList::renderMenuElements,
                         foeImGuiResourceList::renderCustomUI, renderMenus.data(),
                         renderMenus.size());
}

void foeImGuiResourceList::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiResourceList::renderMenuElements,
                     foeImGuiResourceList::renderCustomUI, renderMenus.data(), renderMenus.size());
}

bool foeImGuiResourceList::renderMenuElements(void *pContext, char const *pMenu) {
    auto *pData = static_cast<foeImGuiResourceList *>(pContext);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiResourceList::renderCustomUI(void *pContext) {
    auto *pData = static_cast<foeImGuiResourceList *>(pContext);

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
        auto *pIndexGenerator = mpSimulationState->groupData.resourceIndices(group);
        if (pIndexGenerator == nullptr)
            continue;

        PerGroupData indiceData{
            .group = group,
        };
        pIndexGenerator->exportState(indiceData.nextIndex, indiceData.recycledIndices);

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

            foeResourceID resource = foeIdCreate(currentGroupData->group, index);
            ResourceDisplayData *pDisplayData{nullptr};
            bool selected{false};

            // See if the entity already has a window displaying it's data
            for (auto &it : mDisplayed) {
                if (resource == it.resource) {
                    pDisplayData = &it;
                    break;
                }
            }

            ImGui::TableNextRow();

            // ID
            ImGui::TableNextColumn();
            if (ImGui::Selectable(foeIdToString(resource).c_str(), pDisplayData != nullptr)) {
                selected = true;
            }

            // EditorName
            ImGui::TableNextColumn();
            std::string editorName = (mpSimulationState->pResourceNameMap)
                                         ? mpSimulationState->pResourceNameMap->find(resource)
                                         : "";
            if (ImGui::Selectable(editorName.c_str(), pDisplayData != nullptr)) {
                selected = true;
            }

            if (selected) {
                if (pDisplayData != nullptr) {
                    pDisplayData->focus = true;
                } else {
                    mDisplayed.emplace_back(ResourceDisplayData{
                        .resource = resource,
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

namespace {

template <typename ResourcePool>
auto getResourcePool(foeResourcePoolBase **pResourcePools, size_t poolCount) -> ResourcePool * {
    ResourcePool *pPool{nullptr};
    for (size_t i = 0; i < poolCount; ++i) {
        pPool = dynamic_cast<ResourcePool *>(pResourcePools[i]);
        if (pPool != nullptr)
            break;
    }

    return pPool;
}

} // namespace

bool foeImGuiResourceList::displayResource(ResourceDisplayData *pData) {
    if (!pData->open) {
        return false;
    }

    ImGui::SetNextWindowSize(ImVec2(0, 0), 0);

    if (pData->focus) {
        ImGui::SetNextWindowFocus();
        pData->focus = false;
    }

    std::string windowTitle = "Entity " + foeIdToString(pData->resource);

    if (!ImGui::Begin(windowTitle.data(), &pData->open, 0)) {
        ImGui::End();
        return false;
    }

    // EditorID
    if (mpSimulationState->pResourceNameMap != nullptr) {
        ImGui::Text("EditorID: %s",
                    mpSimulationState->pResourceNameMap->find(pData->resource).data());
    }

    mpRegistrar->displayResource(pData->resource, mpSimulationState->resourcePools.data(),
                                 mpSimulationState->resourcePools.size());

    ImGui::End();

    return true;
}
