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

#include <foe/imex/yaml/importer.hpp>

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/group_translator.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "common.hpp"
#include "error_code.hpp"
#include "import_functionality.hpp"
#include "log.hpp"

#include <string>
#include <string_view>

namespace {

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

foeYamlImporter::foeYamlImporter(foeIdGroup group, std::filesystem::path rootDir) :
    mRootDir{std::move(rootDir)}, mGroup{group} {}

foeIdGroup foeYamlImporter::group() const noexcept { return mGroup; }

std::string foeYamlImporter::name() const noexcept { return mRootDir.stem().string(); }

void foeYamlImporter::setGroupTranslator(foeEcsGroupTranslator groupTranslator) {
    mGroupTranslator = groupTranslator;
    mHasTranslation = true;
}

foeErrorCode foeYamlImporter::getDependencies(uint32_t *pDependencyCount,
                                              foeIdGroup *pDependencyGroups,
                                              uint32_t *pNamesLength,
                                              char *pNames) {
    YAML::Node dependenciesNode;

    if (!openYamlFile(mRootDir / dependenciesFilePath, dependenciesNode))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);

    struct DependencyNode {
        std::string name;
        foeIdGroupValue groupValue;
    };

    uint32_t namesLength = 0;
    std::vector<DependencyNode> dependencies;

    try {
        for (auto it = dependenciesNode.begin(); it != dependenciesNode.end(); ++it) {
            DependencyNode data;

            yaml_read_required("name", *it, data.name);
            yaml_read_required("group_id", *it, data.groupValue);

            namesLength += data.name.size() + 1;
            dependencies.emplace_back(data);
        }
    } catch (YAML::Exception const &e) {
        FOE_LOG(foeImexYaml, Error, "{}", e.what())
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DEPENDENCIES);
    }

    std::error_code errC = FOE_IMEX_YAML_SUCCESS;

    if (pDependencyGroups == nullptr && pNames == nullptr) {
        *pDependencyCount = dependencies.size();
        *pNamesLength = namesLength;
    } else {
        if (*pDependencyCount < dependencies.size() || *pNamesLength < namesLength)
            errC = FOE_IMEX_YAML_INCOMPLETE;

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

    return foeToErrorCode(errC);
}

namespace {

bool getGroupIndexData(std::filesystem::path path, foeIdIndexGenerator &ecsGroup) {
    YAML::Node node;
    if (!openYamlFile(path, node))
        return false;

    try {
        yaml_read_index_generator("", node, ecsGroup);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to parse Group State Index Data from Yaml file: {}",
                e.what())
        return false;
    }

    return true;
}

} // namespace

bool foeYamlImporter::getGroupEntityIndexData(foeIdIndexGenerator &ecsGroup) {
    return getGroupIndexData(mRootDir / entityIndexDataFilePath, ecsGroup);
}

bool foeYamlImporter::getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) {
    return getGroupIndexData(mRootDir / resourceIndexDataFilePath, ecsGroup);
}

bool foeYamlImporter::importStateData(foeEditorNameMap *pEntityNameMap,
                                      foeSimulation const *pSimulation) {
    if (!std::filesystem::exists(mRootDir / entityDirectoryPath))
        return true;

    for (auto &dirIt :
         std::filesystem::recursive_directory_iterator{mRootDir / entityDirectoryPath}) {
        FOE_LOG(foeImexYaml, Info, "Visiting: {}", dirIt.path().string())
        if (std::filesystem::is_directory(dirIt))
            continue;

        if (!std::filesystem::is_regular_file(dirIt)) {
            FOE_LOG(foeImexYaml, Warning,
                    "State data directory entry '{}' not a directory or regular file! Possible "
                    "corruption!",
                    dirIt.path().string())
            return false;
        }

        // Otherwise, parse the entity state data
        YAML::Node entityNode;
        if (!openYamlFile(dirIt, entityNode))
            return false;

        try {
            foeId entity;
            yaml_read_id_required("", entityNode, mGroupTranslator, entity);

            if (pEntityNameMap != nullptr) {
                std::string editorName;
                yaml_read_optional("editor_name", entityNode, editorName);

                if (!editorName.empty()) {
                    pEntityNameMap->add(entity, editorName);
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
                    searchIt->second(entityNode, mGroupTranslator, entity, pSimulation);
                } else {
                    FOE_LOG(foeImexYaml, Error,
                            "Failed to find importer for '{}' component key for {} entity ({})",
                            key, foeIdToString(entity), dirIt.path().string())
                    return false;
                }
            }

            FOE_LOG(foeImexYaml, Info, "Parsed entity {}", foeIdToString(entity))
        } catch (foeYamlException const &e) {
            FOE_LOG(foeImexYaml, Error, "Failed to parse entity state data: {}", e.what())
            return false;
        }
    }

    return true;
}

// @todo Add group translations for imported resource definitions
bool foeYamlImporter::importResourceDefinitions(foeEditorNameMap *pNameMap,
                                                foeSimulation const *pSimulation) {
    if (!std::filesystem::exists(mRootDir / resourceDirectoryPath))
        return true;

    for (auto &dirIt :
         std::filesystem::recursive_directory_iterator{mRootDir / resourceDirectoryPath}) {
        if (std::filesystem::is_directory(dirIt))
            continue;

        if (!std::filesystem::is_regular_file(dirIt)) {
            FOE_LOG(foeImexYaml, Warning,
                    "Resource file '{}' not a directory or regular file! Possible corruption!",
                    dirIt.path().string())
            return false;
        }

        // Is a regular file continue...
        YAML::Node node;
        if (!openYamlFile(dirIt, node))
            return false;

        foeId resource;

        try {
            // ID
            yaml_read_id_required("", node, mGroupTranslator, resource);

            // Editor Name
            if (pNameMap != nullptr) {
                std::string editorName;
                yaml_read_optional("editor_name", node, editorName);

                if (!editorName.empty()) {
                    pNameMap->add(resource, editorName);
                }
            }

            // Resource Type
            bool processed = false;
            auto lock = sharedLockImportFunctionality();
            auto const &resourceFnMap = getResourceFns();
            for (auto const &it : node) {
                std::string key = it.first.as<std::string>();
                if (key == "index_id" || key == "group_id" || key == "editor_name")
                    continue;

                auto searchIt = resourceFnMap.find(key);
                if (searchIt != resourceFnMap.end()) {
                    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
                    searchIt->second.pImport(node, mGroupTranslator, &createInfo);

                    if (searchIt->second.pCreate != nullptr) {
                        auto errC = searchIt->second.pCreate(resource, createInfo, pSimulation);
                        if (errC) {
                            return false;
                        }
                    }

                    foeDestroyResourceCreateInfo(createInfo);
                    processed = true;
                    break;
                } else {
                    FOE_LOG(foeImexYaml, Error,
                            "Failed to find importer for '{}' resource key for {} resource ({})",
                            key, foeIdToString(resource), dirIt.path().string())
                    return false;
                }
            }

            if (!processed) {
                FOE_LOG(foeImexYaml, Error, "Failed to generate resource for {} resource ({})",
                        foeIdToString(resource), dirIt.path().string())
                return false;
            }
        } catch (foeYamlException const &e) {
            FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
            return false;
        } catch (std::exception const &e) {
            FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
            return false;
        } catch (...) {
            FOE_LOG(foeImexYaml, Error,
                    "Failed to import resource definition with unknown exception");
            return false;
        }
    }

    return true;
}

std::string foeYamlImporter::getResourceEditorName(foeIdIndex resourceIndexID) {
    std::string editorName;
    if (!std::filesystem::exists(mRootDir / resourceDirectoryPath))
        return editorName;

    YAML::Node rootNode;

    for (auto const &dirEntry :
         std::filesystem::recursive_directory_iterator{mRootDir / resourceDirectoryPath}) {
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

    return editorName;

OPENED_YAML_FILE:
    try {
        // ID
        foeResourceID readID;
        yaml_read_id_required("", rootNode, nullptr, readID);

        if (readID != foeIdCreate(foeIdPersistentGroup, resourceIndexID))
            std::abort();

        // Editor Name
        yaml_read_optional("editor_name", rootNode, editorName);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
    } catch (std::exception const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
    } catch (...) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition with unknown exception");
    }

    return editorName;
}

foeResourceCreateInfo foeYamlImporter::getResource(foeId id) {
    if (!std::filesystem::exists(mRootDir / resourceDirectoryPath))
        return nullptr;

    YAML::Node rootNode;
    foeIdIndex index = foeIdGetIndex(id);

    for (auto &dirEntry :
         std::filesystem::recursive_directory_iterator{mRootDir / resourceDirectoryPath}) {
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
                it.second.pImport(rootNode, mGroupTranslator, &createInfo);
                return createInfo;
            }
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
        return nullptr;
    } catch (std::exception const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition: {}", e.what());
        return nullptr;
    } catch (...) {
        FOE_LOG(foeImexYaml, Error, "Failed to import resource definition with unknown exception");
        return nullptr;
    }

    return nullptr;
}

std::filesystem::path foeYamlImporter::findExternalFile(std::filesystem::path externalFilePath) {
    if (std::filesystem::exists(mRootDir / externalDirectoryPath / externalFilePath)) {
        return mRootDir / externalDirectoryPath / externalFilePath;
    }

    return std::filesystem::path{};
}