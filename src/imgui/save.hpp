// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_SAVE_HPP
#define IMGUI_SAVE_HPP

#include <foe/imex/exporters.h>

struct ImGuiContext;
class foeImGuiState;
struct foeSimulation;

class foeImGuiSave {
  public:
    void setSimulationState(foeSimulation *pSimulation);
    void clearSimulationState();

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext,
                                   void *pUserData,
                                   char const *pMenuName);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    foeSimulation *mpSimulationState{nullptr};

    foeExporter mSelectedExporter{};

    bool mChooseExporterDialog{false};
    bool mSaveFileDialog{false};
    bool mSaveConfirmDialog{false};
};

#endif // IMGUI_SAVE_HPP