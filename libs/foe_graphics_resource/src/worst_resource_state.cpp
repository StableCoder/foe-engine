// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "worst_resource_state.hpp"

foeResourceLoadState worstResourceLoadState(uint32_t resourceCount, foeResource *pResources) {
    foeResourceLoadState worstState = foeResourceLoadState::Loaded;

    for (uint32_t i = 0; i < resourceCount; ++i) {
        if (pResources[i] == FOE_NULL_HANDLE)
            continue;

        auto state = foeResourceGetState(pResources[i]);
        if (state == foeResourceLoadState::Failed)
            return foeResourceLoadState::Failed;
        if (state == foeResourceLoadState::Unloaded)
            worstState = foeResourceLoadState::Unloaded;
    }

    return worstState;
}