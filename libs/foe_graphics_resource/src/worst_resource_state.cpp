/*
    Copyright (C) 2022 George Cave.

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