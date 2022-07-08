// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/resource.h>

extern "C" char const *foeResourceLoadStateToString(foeResourceLoadState state) {
    switch (state) {
    case foeResourceLoadState::Unloaded:
        return "Unloaded";
    case foeResourceLoadState::Failed:
        return "Failed";
    case foeResourceLoadState::Loaded:
        return "Loaded";
    }

    return "<unknown>";
}