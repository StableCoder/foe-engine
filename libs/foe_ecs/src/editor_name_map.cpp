/*
    Copyright (C) 2021 George Cave.

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

#include "log.hpp"

foeId foeEditorNameMap::find(std::string_view editorName) {
    foeId id = FOE_INVALID_ID;

    mSync.lock_shared();

    auto searchIt = mEditorToId.find(std::string{editorName});
    if (searchIt != mEditorToId.end())
        id = searchIt->second;

    mSync.unlock_shared();

    return id;
}

std::string foeEditorNameMap::find(foeId id) {
    std::string entityName;

    mSync.lock_shared();

    auto searchIt = mIdToEditor.find(id);
    if (searchIt != mIdToEditor.end())
        entityName = searchIt->second;

    mSync.unlock_shared();

    return entityName;
}

bool foeEditorNameMap::add(foeId id, std::string editorName) {
    if (id == FOE_INVALID_ID) {
        FOE_LOG(foeECS, Warning, "Attempted to add an invalid ID with an editor name of '{}'",
                editorName)
        return false;
    }
    if (editorName.empty()) {
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
    if (mEditorToId.find(editorName) != mEditorToId.end()) {
        FOE_LOG(foeECS, Warning,
                "Attempted to add ID {} that with editor name {} that is already used by ID {}",
                foeIdToString(id), editorName, foeIdToString(mEditorToId.find(editorName)->second))
        return false;
    }

    mIdToEditor.emplace(id, editorName);
    mEditorToId.emplace(editorName, id);

    return true;
}

bool foeEditorNameMap::update(foeId id, std::string editorName) {
    if (editorName.empty()) {
        FOE_LOG(foeECS, Warning, "Attempted to update ID {} with a blank editor name",
                foeIdToString(id))
        return false;
    }

    // Make sure editor name not already in use
    if (mEditorToId.find(editorName) != mEditorToId.end()) {
        FOE_LOG(foeECS, Warning,
                "Attempted to update ID {} with editor name '{}' already used by ID {}",
                foeIdToString(id), editorName, foeIdToString(mEditorToId.find(editorName)->second))
        return false;
    }

    std::scoped_lock lock{mSync};
    auto searchIt = mIdToEditor.find(id);
    if (searchIt == mIdToEditor.end()) {
        FOE_LOG(foeECS, Info, "Attempted to update ID {} that did not have an editorName",
                foeIdToString(id))
        return false;
    }

    mEditorToId.erase(editorName);
    mEditorToId.emplace(editorName, id);

    searchIt->second = editorName;

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