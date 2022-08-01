// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "worst_resource_state.hpp"

foeResourceLoadState worstResourceLoadState(uint32_t resourceCount, foeResource *pResources) {
    foeResourceLoadState worstState = FOE_RESOURCE_LOAD_STATE_LOADED;

    for (uint32_t i = 0; i < resourceCount; ++i) {
        if (pResources[i] == FOE_NULL_HANDLE)
            continue;

        auto state = foeResourceGetState(pResources[i]);
        if (state == FOE_RESOURCE_LOAD_STATE_FAILED)
            return FOE_RESOURCE_LOAD_STATE_FAILED;
        if (state == FOE_RESOURCE_LOAD_STATE_UNLOADED)
            worstState = FOE_RESOURCE_LOAD_STATE_UNLOADED;
    }

    return worstState;
}