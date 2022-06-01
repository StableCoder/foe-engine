/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/ecs/editor_name_map.hpp>

#include <foe/ecs/error_code.h>
#include <foe/ecs/id_to_string.hpp>

#include "log.hpp"
#include "result.h"

foeId foeEditorNameMap::find(char const *pName) {
    foeId id = FOE_INVALID_ID;

    mSync.lock_shared();

    auto searchIt = mEditorToId.find(pName);
    if (searchIt != mEditorToId.end())
        id = searchIt->second;

    mSync.unlock_shared();

    return id;
}

foeResult foeEditorNameMap::find(foeId id, uint32_t *pNameLength, char *pName) {
    std::shared_lock lock{mSync};

    auto searchIt = mIdToEditor.find(id);
    if (searchIt != mIdToEditor.end()) {
        foeResult result = to_foeResult(FOE_ECS_SUCCESS);
        if (pName == nullptr) {
            *pNameLength = searchIt->second.size() + 1;
            return result;
        }

        // Otherwise copy operation
        uint32_t copySize = std::min((uint32_t)searchIt->second.size() + 1, *pNameLength);
        if (copySize < searchIt->second.size() + 1)
            result = to_foeResult(FOE_ECS_INCOMPLETE);
        memcpy(pName, searchIt->second.data(), copySize);
        *pNameLength = copySize;

        return result;
    }

    return to_foeResult(FOE_ECS_NO_MATCH);
}

bool foeEditorNameMap::add(foeId id, char const *pName) {
    if (id == FOE_INVALID_ID) {
        FOE_LOG(foeECS, Warning, "Attempted to add an invalid ID with an editor name of '{}'",
                pName)
        return false;
    }
    if (strlen(pName) == 0) {
        FOE_LOG(foeECS, Warning, "Attempted to add ID {} with a blank editor name",
                foeIdToString(id))
        return false;
    }

    std::scoped_lock lock{mSync};

    if (mIdToEditor.find(id) != mIdToEditor.end()) {
        FOE_LOG(foeECS, Warning, "Attempted to add ID {} that already has an editor name",
                foeIdToString(id))
        return false;
    }
    if (mEditorToId.find(pName) != mEditorToId.end()) {
        FOE_LOG(foeECS, Warning,
                "Attempted to add ID {} that with editor name {} that is already used by ID {}",
                foeIdToString(id), pName, foeIdToString(mEditorToId.find(pName)->second))
        return false;
    }

    mIdToEditor.emplace(id, pName);
    mEditorToId.emplace(pName, id);

    return true;
}

bool foeEditorNameMap::update(foeId id, char const *pName) {
    if (strlen(pName) == 0) {
        FOE_LOG(foeECS, Warning, "Attempted to update ID {} with a blank editor name",
                foeIdToString(id))
        return false;
    }

    // Make sure editor name not already in use
    if (mEditorToId.find(pName) != mEditorToId.end()) {
        FOE_LOG(foeECS, Warning,
                "Attempted to update ID {} with editor name '{}' already used by ID {}",
                foeIdToString(id), pName, foeIdToString(mEditorToId.find(pName)->second))
        return false;
    }

    std::scoped_lock lock{mSync};
    auto searchIt = mIdToEditor.find(id);
    if (searchIt == mIdToEditor.end()) {
        FOE_LOG(foeECS, Info, "Attempted to update ID {} that did not have an editorName",
                foeIdToString(id))
        return false;
    }

    mEditorToId.erase(pName);
    mEditorToId.emplace(pName, id);

    searchIt->second = pName;

    return true;
}

bool foeEditorNameMap::remove(foeId id) {
    std::scoped_lock lock{mSync};

    auto searchIt = mIdToEditor.find(id);
    if (searchIt == mIdToEditor.end()) {
        FOE_LOG(foeECS, Info, "Attempted to remove ID {} that did not have an editorName",
                foeIdToString(id))
        return false;
    }

    mEditorToId.erase(searchIt->second);
    mIdToEditor.erase(searchIt);

    return true;
}