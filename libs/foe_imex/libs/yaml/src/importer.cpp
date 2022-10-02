// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/yaml/importer.hpp>

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/indexes.hpp>
#include <foe/imex/type_defs.h>
#include <foe/imex/yaml/type_defs.h>
#include <foe/memory_mapped_file.h>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include "common.hpp"
#include "import_functionality.hpp"
#include "log.hpp"
#include "result.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace {

struct foeYamlImporter {
    foeStructureType sType;
    void *pNext;

    std::filesystem::path mRootDir;
    foeIdGroup mGroup;
    std::string mName;

    bool mHasTranslation{false};
    foeEcsGroupTranslator mGroupTranslator{FOE_NULL_HANDLE};
};

FOE_DEFINE_HANDLE_CASTS(importer, foeYamlImporter, foeImexImporter)

/** @brief Parses a Yaml filename to determine the IdGroupValue and IdIndex
 * @param path File path to parse
 * @param groupValue Returns the IdGroup, return foeIdPersistentGroup if there was no specified
 * IdGroup
 * @param index Returns the parsed IdIndex
 * @return True if successfully parsed, false otherwise
 */
bool parseFileStem(std::filesystem::path const &path,
                   foeIdGroupValue &groupValue,
                   foeIdIndex &index) {
    std::string fullStem{path.stem().string()};

    // Group Stem
    auto firstStemEnd = fullStem.find_first_of('_');
    if (firstStemEnd == std::string::npos) {
        return false;
    }
    std::string idGroupStem = fullStem.substr(0, firstStemEnd);
    ++firstStemEnd;

    // Index Stem
    auto secondStemEnd = fullStem.find_first_of('_', firstStemEnd);
    std::string idIndexStem = fullStem.substr(firstStemEnd, secondStemEnd - firstStemEnd);
    if (idIndexStem.empty())
        return false;

    // Parsing
    char *endCh;
    groupValue = foeIdPersistentGroupValue;
    if (!idGroupStem.empty()) {
        groupValue = std::strtoul(idGroupStem.c_str(), &endCh, 0);
    }

    index = std::strtoul(idIndexStem.c_str(), &endCh, 0);

    return true;
}

bool openYamlFile(std::filesystem::path path, YAML::Node &rootNode) {
    try {
        rootNode = YAML::LoadFile(path.string());
    } catch (YAML::ParserException const &e) {
        FOE_LOG(foeImexYaml, Fatal, "Failed to load Yaml file: {}", e.what());
        return false;
    } catch (YAML::BadFile const &e) {
        FOE_LOG(foeImexYaml, Fatal, "YAML::LoadFile failed: {}", e.what());
        return false;
    }

    return true;
}

} // namespace

void destroy(foeImexImporter importer) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    FOE_LOG(foeImexYaml, Verbose, "[{}] foeYamlImporter - Destroying ({})", (void *)pImporter,
            pImporter->mName.c_str());

    if (pImporter->mGroupTranslator != FOE_NULL_HANDLE)
        foeEcsDestroyGroupTranslator(pImporter->mGroupTranslator);

    delete pImporter;
    FOE_LOG(foeImexYaml, Verbose, "[{}] foeYamlImporter - Destroyed", (void *)pImporter);
}

foeResultSet getGroupID(foeImexImporter importer, foeIdGroup *pGroupID) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    *pGroupID = pImporter->mGroup;
    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResultSet getGroupName(foeImexImporter importer, char const **ppGroupName) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    *ppGroupName = pImporter->mName.c_str();
    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResultSet setGroupTranslator(foeImexImporter importer, foeEcsGroupTranslator groupTranslator) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    pImporter->mGroupTranslator = groupTranslator;
    pImporter->mHasTranslation = true;

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResultSet getDependencies(foeImexImporter importer,
                             uint32_t *pDependencyCount,
                             foeIdGroup *pDependencyGroups,
                             uint32_t *pNamesLength,
                             char *pNames) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    YAML::Node dependenciesNode;

    if (!openYamlFile(pImporter->mRootDir / dependenciesFilePath, dependenciesNode))
        return to_foeResult(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);

    struct DependencyNode {
        std::string name;
        foeIdGroupValue groupValue;
    };

    uint32_t namesLength = 0;
    std::vector<DependencyNode> dependencies;

    try {
        for (auto it = dependenciesNode.begin(); it != dependenciesNode.end(); ++it) {
            DependencyNode data;

            yaml_read_string("name", *it, data.name);
            yaml_read_uint32_t("group_id", *it, data.groupValue);

            namesLength += data.name.size() + 1;
            dependencies.emplace_back(data);
        }
    } catch (YAML::Exception const &e) {
        FOE_LOG(foeImexYaml, Error, "{}", e.what())
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DEPENDENCIES);
    }

    foeResultSet result = to_foeResult(FOE_IMEX_YAML_SUCCESS);

    if (pDependencyGroups == nullptr && pNames == nullptr) {
        *pDependencyCount = dependencies.size();
        *pNamesLength = namesLength;
    } else {
        if (*pDependencyCount < dependencies.size() || *pNamesLength < namesLength)
            result = to_foeResult(FOE_IMEX_YAML_INCOMPLETE);

        char *const pEndName = pNames + *pNamesLength;
        uint32_t const processedCount = std::min(*pDependencyCount, (uint32_t)dependencies.size());

        for (uint32_t i = 0; i < processedCount; ++i) {
            pDependencyGroups[i] = foeIdValueToGroup(dependencies[i].groupValue);

            size_t copyLength = dependencies[i].name.size() + 1;
            if (pEndName - pNames < copyLength) {
                copyLength = pEndName - pNames;
            }

            strncpy(pNames, dependencies[i].name.data(), copyLength);

            pNames += copyLength;
        }

        *pDependencyCount = processedCount;
        *pNamesLength = std::min(*pNamesLength, namesLength);

        if (*pNamesLength != 0) {
            --pNames;
            *pNames = '\0';
        }
    }

    return result;
}

namespace {

foeResultSet getGroupIndexData(std::filesystem::path path, foeEcsIndexes indexes) {
    YAML::Node node;
    if (!openYamlFile(path, node))
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_FILE);

    try {
        yaml_read_indexes("", node, indexes);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to parse Group State Index Data from Yaml file: {}",
                e.what())
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA);
    }

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

} // namespace

foeResultSet getGroupEntityIndexData(foeImexImporter importer, foeEcsIndexes indexes) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    return getGroupIndexData(pImporter->mRootDir / entityIndexDataFilePath, indexes);
}

foeResultSet getGroupResourceIndexData(foeImexImporter importer, foeEcsIndexes indexes) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    return getGroupIndexData(pImporter->mRootDir / resourceIndexDataFilePath, indexes);
}

foeResultSet importStateData(foeImexImporter importer,
                             foeEcsNameMap nameMap,
                             foeSimulation const *pSimulation) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    if (!std::filesystem::exists(pImporter->mRootDir / entityDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_EXIST);

    for (auto &dirIt :
         std::filesystem::recursive_directory_iterator{pImporter->mRootDir / entityDirectoryPath}) {
        FOE_LOG(foeImexYaml, Info, "Visiting: {}", dirIt.path().string())
        if (std::filesystem::is_directory(dirIt))
            continue;

        if (!std::filesystem::is_regular_file(dirIt)) {
            FOE_LOG(foeImexYaml, Warning,
                    "State data directory entry '{}' not a directory or regular file! Possible "
                    "corruption!",
                    dirIt.path().string())
            return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_FILE_NOT_FILE);
        }

        // Otherwise, parse the entity state data
        YAML::Node entityNode;
        if (!openYamlFile(dirIt, entityNode))
            return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_ENTITY_FILE);

        try {
            foeId entity;
            if (!yaml_read_foeEntityID("", entityNode, pImporter->mGroupTranslator, entity)) {
                FOE_LOG(foeImexYaml, Error,
                        "Failed to read foeEntityID for entity data in file: {}",
                        dirIt.path().string())
                std::abort();
            }

            if (nameMap != FOE_NULL_HANDLE) {
                std::string editorName;
                yaml_read_string("editor_name", entityNode, editorName);

                if (!editorName.empty()) {
                    foeEcsNameMapAdd(nameMap, entity, editorName.c_str());
                }
            }

            auto lock = sharedLockImportFunctionality();
            auto const &componentFnMap = getComponentFns();
            for (auto const &it : entityNode) {
                std::string key = it.first.as<std::string>();
                if (key == "index_id" || key == "group_id" || key == "editor_name")
                    continue;

                auto searchIt = componentFnMap.find(key);
                if (searchIt != componentFnMap.end()) {
                    searchIt->second(entityNode, pImporter->mGroupTranslator, entity, pSimulation);
                } else {
                    FOE_LOG(foeImexYaml, Error,
                            "Failed to find importer for '{}' component key for {} entity ({})",
                            key, foeIdToString(entity), dirIt.path().string())
                    return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_FIND_COMPONENT_IMPORTER);
                }
            }

            FOE_LOG(foeImexYaml, Info, "Parsed entity {}", foeIdToString(entity))
        } catch (foeYamlException const &e) {
            FOE_LOG(foeImexYaml, Error, "Failed to parse entity state data: {}", e.what())
            return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_IMPORT_COMPONENT);
        }
    }

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResultSet getResourceEditorName(foeImexImporter importer,
                                   foeIdIndex resourceIndexID,
                                   uint32_t *pNameLength,
                                   char *pName) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    if (!std::filesystem::exists(pImporter->mRootDir / resourceDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_RESOURCE_FILE);

    YAML::Node rootNode;

    for (auto const &dirEntry : std::filesystem::recursive_directory_iterator{
             pImporter->mRootDir / resourceDirectoryPath}) {
        if (!dirEntry.is_regular_file())
            continue;

        foeIdGroupValue fileGroupValue;
        foeIdIndex fileIndex;
        if (!parseFileStem(dirEntry, fileGroupValue, fileIndex))
            continue;

        // Check the IndexID
        if (fileIndex != resourceIndexID)
            continue;

        // Check the GroupID (must be persistent, names can only be set by the initial group)
        if (foeIdValueToGroup(fileGroupValue) != foeIdPersistentGroup)
            continue;

        // If here, found the file
        if (openYamlFile(dirEntry, rootNode))
            goto OPENED_YAML_FILE;
    }

    return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_RESOURCE_FILE);

OPENED_YAML_FILE:
    try {
        // ID
        foeResourceID readID;
        if (!yaml_read_foeResourceID("", rootNode, nullptr, readID)) {
            FOE_LOG(foeImexYaml, Error, "Failed to read top-level foeResourceID from file")
            std::abort();
        }

        if (readID != foeIdCreate(foeIdPersistentGroup, resourceIndexID))
            std::abort();

        // Editor Name
        std::string editorName;
        yaml_read_string("editor_name", rootNode, editorName);

        if (pName == NULL) {
            // If no return buffer provided, just return length of found name
            *pNameLength = editorName.size();
            return imex_to_foeResult(FOE_IMEX_SUCCESS);
        }

        if (*pNameLength < editorName.size()) {
            // Not enough space for the full name, copy what we can to the return buffer
            memcpy(pName, editorName.data(), *pNameLength);
            return imex_to_foeResult(FOE_IMEX_ERROR_INCOMPLETE);
        }

        // Copy the full name, adjust the returned number of bytes
        memcpy(pName, editorName.data(), editorName.size());
        *pNameLength = editorName.size();
        return imex_to_foeResult(FOE_IMEX_SUCCESS);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
    } catch (std::exception const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
    } catch (...) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition with unknown exception");
    }

    return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA);
}

foeResultSet getResourceCreateInfo(foeImexImporter importer,
                                   foeId id,
                                   foeResourceCreateInfo *pResourceCreateInfo) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    if (!std::filesystem::exists(pImporter->mRootDir / resourceDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_FILE);

    YAML::Node rootNode;
    foeIdIndex index = foeIdGetIndex(id);

    for (auto &dirEntry : std::filesystem::recursive_directory_iterator{pImporter->mRootDir /
                                                                        resourceDirectoryPath}) {
        if (!dirEntry.is_regular_file())
            continue;

        foeIdGroupValue fileGroupValue;
        foeIdIndex fileIndex;
        if (!parseFileStem(dirEntry, fileGroupValue, fileIndex))
            continue;

        if (fileIndex == FOE_INVALID_ID || fileIndex != index)
            continue;

        if (fileIndex == index) {
            if (openYamlFile(dirEntry, rootNode))
                goto GOT_RESOURCE_NODE;
        }
    }
GOT_RESOURCE_NODE:

    try {
        foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

        auto lock = sharedLockImportFunctionality();
        for (auto const &it : getResourceFns()) {
            if (auto subNode = rootNode[it.first]; subNode) {
                it.second.pImport(rootNode, pImporter->mGroupTranslator, &createInfo);
                *pResourceCreateInfo = createInfo;
                return to_foeResult(FOE_IMEX_YAML_SUCCESS);
            }
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA);
        ;
    } catch (std::exception const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA);
    } catch (...) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition with unknown exception");
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA);
    }

    return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA);
}

foeResultSet findExternalFile(foeImexImporter importer,
                              char const *pExternalFilePath,
                              foeManagedMemory *pManagedMemory) {
    foeYamlImporter *pImporter = importer_from_handle(importer);

    std::filesystem::path path = pImporter->mRootDir / externalDirectoryPath / pExternalFilePath;
    if (std::filesystem::exists(path)) {
        return foeCreateMemoryMappedFile(path.string().c_str(), pManagedMemory);
    } else {
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_FILE);
    }
}

namespace {

foeImexImporterCalls cImporterCalls{
    .sType = FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS,
    .destroyImporter = destroy,
    .getGroupID = getGroupID,
    .getGroupName = getGroupName,
    .setGroupTranslator = setGroupTranslator,
    .getDependencies = getDependencies,
    .getGroupEntityIndexData = getGroupEntityIndexData,
    .getGroupResourceIndexData = getGroupResourceIndexData,
    .importStateData = importStateData,
    .getResourceEditorName = getResourceEditorName,
    .getResourceCreateInfo = getResourceCreateInfo,
    .findExternalFile = findExternalFile,
};

}

foeResultSet foeCreateYamlImporter(foeIdGroup group,
                                   char const *pRootDir,
                                   foeImexImporter *pImporter) {
    std::filesystem::path fsPath{pRootDir};

    // Root Directory
    if (!std::filesystem::is_directory(fsPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY);

    // Dependencies File
    if (!std::filesystem::exists(fsPath / dependenciesFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / dependenciesFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE);

    // Resource Index Data File
    if (!std::filesystem::exists(fsPath / resourceIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / resourceIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE);

    // Entity Index Data File
    if (!std::filesystem::exists(fsPath / entityIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / entityIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE);

    // Check optional directories (resources, components and external data), fail if they fail
    // Resources Directory
    if (std::filesystem::exists(fsPath / resourceDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / resourceDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY);

    // Entity Component Data Directory
    if (std::filesystem::exists(fsPath / entityDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / entityDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY);

    // External Data Directory
    if (std::filesystem::exists(fsPath / externalDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / externalDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY);

    // If here, then we're clear to create the importer
    foeYamlImporter *pNewImporter = new (std::nothrow) foeYamlImporter{
        .sType = FOE_IMEX_YAML_STRUCTURE_TYPE_IMPORTER,
        .pNext = &cImporterCalls,
        .mRootDir = pRootDir,
        .mGroup = group,
        .mName = std::filesystem::path{pRootDir}.stem().string(),
    };
    if (pNewImporter == NULL)
        return to_foeResult(FOE_IMEX_YAML_ERROR_OUT_OF_MEMORY);

    *pImporter = importer_to_handle(pNewImporter);

    FOE_LOG(foeImexYaml, Verbose, "[{}] foeYamlImporter - Created ({})", (void *)pNewImporter,
            pNewImporter->mName.c_str())

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}