// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/name_map.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/result.h>

#include "log.hpp"
#include "result.h"

#include <map>
#include <shared_mutex>
#include <string>

namespace {

struct NameMap {
    std::shared_mutex sync{};

    std::map<std::string, foeId> editorToId;
    std::map<foeId, std::string> idToEditor;
};

FOE_DEFINE_HANDLE_CASTS(name_map, NameMap, foeEcsNameMap)

} // namespace

extern "C" foeResultSet foeEcsCreateNameMap(foeEcsNameMap *pNameMap) {
    NameMap *pNewNameMap = new (std::nothrow) NameMap;
    if (pNewNameMap == nullptr)
        return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);

    *pNameMap = name_map_to_handle(pNewNameMap);

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" void foeEcsDestroyNameMap(foeEcsNameMap nameMap) {
    NameMap *pNameMap = name_map_from_handle(nameMap);

    delete pNameMap;
}

extern "C" foeResultSet foeEcsNameMapFindID(foeEcsNameMap nameMap, char const *pName, foeId *pID) {
    NameMap *pNameMap = name_map_from_handle(nameMap);
    std::shared_lock lock{pNameMap->sync};

    auto searchIt = pNameMap->editorToId.find(pName);
    if (searchIt != pNameMap->editorToId.end()) {
        *pID = searchIt->second;
        return to_foeResult(FOE_ECS_SUCCESS);
    }

    return to_foeResult(FOE_ECS_NO_MATCH);
}

extern "C" foeResultSet foeEcsNameMapFindName(foeEcsNameMap nameMap,
                                              foeId id,
                                              uint32_t *pNameLength,
                                              char *pName) {
    NameMap *pNameMap = name_map_from_handle(nameMap);
    std::shared_lock lock{pNameMap->sync};

    auto searchIt = pNameMap->idToEditor.find(id);
    if (searchIt != pNameMap->idToEditor.end()) {
        foeResultSet result = to_foeResult(FOE_ECS_SUCCESS);
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

extern "C" foeResultSet foeEcsNameMapAdd(foeEcsNameMap nameMap, foeId id, char const *pName) {
    if (id == FOE_INVALID_ID) {
        FOE_LOG(foeECS, Warning, "Attempted to add an invalid ID with an editor name of '{}'",
                pName)
        return to_foeResult(FOE_ECS_ERROR_INVALID_ID);
    }
    if (strlen(pName) == 0) {
        FOE_LOG(foeECS, Warning, "Attempted to add ID {} with a blank editor name",
                foeIdToString(id))
        return to_foeResult(FOE_ECS_ERROR_EMPTY_NAME);
    }

    NameMap *pNameMap = name_map_from_handle(nameMap);
    std::unique_lock lock{pNameMap->sync};

    if (pNameMap->idToEditor.find(id) != pNameMap->idToEditor.end()) {
        FOE_LOG(foeECS, Warning, "Attempted to add ID {} that already has an editor name",
                foeIdToString(id))
        return to_foeResult(FOE_ECS_ERROR_ID_ALREADY_EXISTS);
    }
    if (pNameMap->editorToId.find(pName) != pNameMap->editorToId.end()) {
        FOE_LOG(foeECS, Warning,
                "Attempted to add ID {} that with editor name {} that is already used by ID {}",
                foeIdToString(id), pName, foeIdToString(pNameMap->editorToId.find(pName)->second))
        return to_foeResult(FOE_ECS_ERROR_NAME_ALREADY_EXISTS);
    }

    pNameMap->idToEditor.emplace(id, pName);
    pNameMap->editorToId.emplace(pName, id);

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" foeResultSet foeEcsNameMapUpdate(foeEcsNameMap nameMap, foeId id, char const *pName) {
    if (strlen(pName) == 0) {
        FOE_LOG(foeECS, Warning, "Attempted to update ID {} with a blank editor name",
                foeIdToString(id))
        return to_foeResult(FOE_ECS_ERROR_EMPTY_NAME);
    }

    NameMap *pNameMap = name_map_from_handle(nameMap);
    std::unique_lock lock{pNameMap->sync};

    // Make sure editor name not already in use
    if (pNameMap->editorToId.find(pName) != pNameMap->editorToId.end()) {
        FOE_LOG(foeECS, Warning,
                "Attempted to update ID {} with editor name '{}' already used by ID {}",
                foeIdToString(id), pName, foeIdToString(pNameMap->editorToId.find(pName)->second))
        return to_foeResult(FOE_ECS_ERROR_NAME_ALREADY_EXISTS);
    }

    auto searchIt = pNameMap->idToEditor.find(id);
    if (searchIt == pNameMap->idToEditor.end()) {
        FOE_LOG(foeECS, Info, "Attempted to update ID {} that did not have an editorName",
                foeIdToString(id))
        return to_foeResult(FOE_ECS_ERROR_NO_MATCH);
    }

    pNameMap->editorToId.erase(pName);
    pNameMap->editorToId.emplace(pName, id);

    searchIt->second = pName;

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" foeResultSet foeEcsNameMapRemove(foeEcsNameMap nameMap, foeId id) {
    NameMap *pNameMap = name_map_from_handle(nameMap);
    std::unique_lock lock{pNameMap->sync};

    auto searchIt = pNameMap->idToEditor.find(id);
    if (searchIt == pNameMap->idToEditor.end()) {
        FOE_LOG(foeECS, Info, "Attempted to remove ID {} that did not have an editorName",
                foeIdToString(id))
        return to_foeResult(FOE_ECS_NO_MATCH);
    }

    pNameMap->editorToId.erase(searchIt->second);
    pNameMap->idToEditor.erase(searchIt);

    return to_foeResult(FOE_ECS_SUCCESS);
}