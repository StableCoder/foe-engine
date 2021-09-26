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

#include <ImGuiFileDialog.h>
#include <foe/imex/exporters.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> menuNameArr{
    "File",
};

}

void foeImGuiSave::setImGuiContext(ImGuiContext *pContext) { ImGui::SetCurrentContext(pContext); }

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
            pData->mSaveConfirmDialog = true;
        }
        if (ImGui::MenuItem("Save As...", nullptr, false, pData->mpSimulationState != nullptr)) {
            pData->mChooseExporterDialog = true;
        }

        return true;
    }

    return false;
}

void foeImGuiSave::renderCustomUI(void *pContext) {
    auto *pData = reinterpret_cast<foeImGuiSave *>(pContext);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (pData->mChooseExporterDialog) {
        pData->mChooseExporterDialog = false;
        ImGui::OpenPopup("ChooseExporterModal");
    }

    if (ImGui::BeginPopupModal("ChooseExporterModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Choose available exporter:\n\n");

        // Get number of available exporters
        bool validExporter{false};
        uint32_t numExporters;
        foeImexGetExporters(&numExporters, nullptr);
        std::unique_ptr<foeExporter[]> pExporters{new foeExporter[numExporters]};
        foeImexGetExporters(&numExporters, pExporters.get());

        for (uint32_t i = 0; i < numExporters; ++i) {
            if (pData->mSelectedExporter == pExporters[i])
                validExporter = true;
            if (ImGui::RadioButton(pExporters[i].pName,
                                   pData->mSelectedExporter == pExporters[i])) {
                pData->mSelectedExporter = pExporters[i];
                validExporter = true;
            }
        }

        ImGui::Separator();

        if (validExporter) {
            if (ImGui::Button("Use", ImVec2(120, 0))) {
                pData->mSaveFileDialog = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        }
        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (pData->mSaveFileDialog) {
        ImGuiFileDialog::Instance()->OpenModal("SaveToLocation", "Choose File", nullptr, ".", 1,
                                               nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
        pData->mSaveFileDialog = false;
    }

    // display
    if (ImGuiFileDialog::Instance()->Display("SaveToLocation")) {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            uint32_t numExporters{1};
            std::unique_ptr<foeExporter[]> pExporters{new foeExporter[numExporters]};
            foeImexGetExporters(&numExporters, pExporters.get());

            if (numExporters == 0) {
                // Need to deal with case of having no available exporters
                std::abort();
            }
            pExporters[0].pExportFn(filePathName.data(), pData->mpSimulationState);

            // action
            ImGui::CloseCurrentPopup();
        }
        if (!ImGuiFileDialog::Instance()->IsOk()) {
            ImGui::CloseCurrentPopup();
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    if (pData->mSaveConfirmDialog) {
        pData->mSaveConfirmDialog = false;
        ImGui::OpenPopup("SaveConfirmModal");
    }

    if (ImGui::BeginPopupModal("SaveConfirmModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to save?\n\n");
        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            // Currently, choose the first and use that
            uint32_t numExporters{1};
            std::unique_ptr<foeExporter[]> pExporters{new foeExporter[numExporters]};
            foeImexGetExporters(&numExporters, pExporters.get());

            if (numExporters == 0) {
                // Need to deal with case of having no available exporters
                std::abort();
            }
            pExporters[0].pExportFn("testExport", pData->mpSimulationState);

            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}