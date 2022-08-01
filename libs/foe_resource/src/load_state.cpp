// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/resource.h>

extern "C" char const *foeResourceLoadStateToString(foeResourceLoadState state) {
    switch (state) {
    case FOE_RESOURCE_LOAD_STATE_UNLOADED:
        return "Unloaded";
    case FOE_RESOURCE_LOAD_STATE_FAILED:
        return "Failed";
    case FOE_RESOURCE_LOAD_STATE_LOADED:
        return "Loaded";
    }

    return "<unknown>";
}