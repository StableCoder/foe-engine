// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_SAVE_HPP
#define IMGUI_SAVE_HPP

#include <foe/handle.h>
#include <foe/imex/exporters.h>

#include <memory>

FOE_DEFINE_HANDLE(foeSimulation)

struct ImGuiContext;
class foeImGuiState;

class foeImGuiSave {
  public:
    void setSimulationState(foeSimulation simulation);
    void clearSimulationState();

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext,
                                   void *pUserData,
                                   char const *pMenuName);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    foeSimulation mSimulation{FOE_NULL_HANDLE};

    uint32_t mNumExporters{0};
    uint32_t mSelectedExporter{0};
    bool mValidExporter{false};
    std::unique_ptr<foeExporter[]> mpExporters;

    bool mChooseExporterDialog{false};
    bool mSaveFileDialog{false};
    bool mSaveConfirmDialog{false};
};

#endif // IMGUI_SAVE_HPP