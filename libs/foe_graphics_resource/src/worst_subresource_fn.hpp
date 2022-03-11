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

#ifndef WORST_SUBRESOURCE_FN_HPP
#define WORST_SUBRESOURCE_FN_HPP

#include <foe/resource/resource.h>

template <typename SubResource, typename... SubResources>
foeResourceLoadState getWorstSubResourceState(SubResource subResource,
                                              SubResources... nextSubResources) {
    if (subResource != FOE_NULL_HANDLE) {
        auto state = foeResourceGetState(subResource);
        if (state != foeResourceLoadState::Loaded) {
            return state;
        }
    }

    if constexpr (sizeof...(SubResources) != 0) {
        // Not the last provided one, keep going
        return getWorstSubResourceState(nextSubResources...);
    } else {
        // End of the line, return that they're all at least in the good 'loaded' state
        return foeResourceLoadState::Loaded;
    }
}

#endif // WORST_SUBRESOURCE_FN_HPP