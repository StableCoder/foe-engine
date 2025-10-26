// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "save.hpp"

#include <foe/external/ImGuiFileDialog.h>
#include <foe/external/imgui.h>
#include <foe/imex/exporters.h>
#include <foe/imgui/state.hpp>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> menuNameArr{
    "File",
};

}

void foeImGuiSave::setSimulationState(foeSimulation *pSimulation) {
    mpSimulationState = pSimulation;
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

bool foeImGuiSave::renderMenuElements(ImGuiContext *pImGuiContext,
                                      void *pUserData,
                                      char const *pMenuName) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = reinterpret_cast<foeImGuiSave *>(pUserData);
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

void foeImGuiSave::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = reinterpret_cast<foeImGuiSave *>(pUserData);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (pData->mChooseExporterDialog) {
        pData->mChooseExporterDialog = false;
        ImGui::OpenPopup("ChooseExporterModal");

        // Get exporters
        pData->mValidExporter = false;
        foeImexGetExporters(&pData->mNumExporters, nullptr);
        pData->mpExporters.reset(new foeExporter[pData->mNumExporters]);
        foeImexGetExporters(&pData->mNumExporters, pData->mpExporters.get());
    }

    if (ImGui::BeginPopupModal("ChooseExporterModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Choose available exporter:\n\n");

        for (uint32_t i = 0; i < pData->mNumExporters; ++i) {
            if (foeCompareExporters(&(pData->mpExporters[pData->mSelectedExporter]),
                                    &pData->mpExporters[i]))
                pData->mValidExporter = true;
            if (ImGui::RadioButton(
                    pData->mpExporters[i].pName,
                    foeCompareExporters(&(pData->mpExporters[pData->mSelectedExporter]),
                                        &pData->mpExporters[i]))) {
                pData->mSelectedExporter = i;
                pData->mValidExporter = true;
            }
        }

        ImGui::Separator();

        if (pData->mValidExporter) {
            if (ImGui::Button("Use", ImVec2(120, 0))) {
                pData->mSaveFileDialog = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        }
        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            pData->mpExporters.reset();
        }

        ImGui::EndPopup();
    }

    if (pData->mSaveFileDialog) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        ImGuiFileDialog::Instance()->OpenDialog("SaveToLocation", "Choose File", nullptr, config);
        pData->mSaveFileDialog = false;
    }

    // display
    if (ImGuiFileDialog::Instance()->Display("SaveToLocation")) {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            pData->mpExporters[pData->mSelectedExporter].pExportFn(filePathName.data(),
                                                                   pData->mpSimulationState);

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