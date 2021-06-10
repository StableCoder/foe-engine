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

#include <foe/imex/yaml/importer.hpp>

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/imex/yaml/generator.hpp>
#include <foe/resource/create_info_base.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "log.hpp"

#include <string>
#include <string_view>

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view resourceIndexDataFilePath = "resource_index_data.yml";
constexpr std::string_view resourceDirectoryPath = "resources";
constexpr std::string_view externalDirectoryPath = "external";
constexpr std::string_view stateIndexDataFilePath = "state_index_data.yml";
constexpr std::string_view stateDirectoryPath = "state";

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
    groupValue = foeIdPersistentGroup;
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

foeYamlImporter::foeYamlImporter(foeYamlImporterGenerator *pGenerator,
                                 foeIdGroup group,
                                 std::filesystem::path rootDir) :
    mGroup{group}, mRootDir{std::move(rootDir)}, mGenerator{pGenerator} {}

namespace {

bool importDependenciesFromNode(YAML::Node const &dependenciesNode,
                                std::vector<foeIdGroupValueNameSet> &dependencies) {
    try {
        for (auto it = dependenciesNode.begin(); it != dependenciesNode.end(); ++it) {
            foeIdGroupValueNameSet newDependency;

            yaml_read_required("name", *it, newDependency.name);
            yaml_read_required("group_id", *it, newDependency.groupValue);

            dependencies.emplace_back(newDependency);
        }
    } catch (YAML::Exception const &e) {
        FOE_LOG(foeImexYaml, Error, "{}", e.what())
        return false;
    }

    return true;
}

} // namespace

foeIdGroup foeYamlImporter::group() const noexcept { return mGroup; }

std::string foeYamlImporter::name() const noexcept { return mRootDir.stem().string(); }

void foeYamlImporter::setGroupTranslator(foeIdGroupTranslator &&groupTranslation) {
    mGroupTranslator = std::move(groupTranslation);
}

bool foeYamlImporter::getDependencies(std::vector<foeIdGroupValueNameSet> &dependencies) {
    YAML::Node node;
    if (!openYamlFile(mRootDir / dependenciesFilePath, node))
        return false;

    return importDependenciesFromNode(node, dependencies);
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
    return getGroupIndexData(mRootDir / stateIndexDataFilePath, ecsGroup);
}

bool foeYamlImporter::getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) {
    return getGroupIndexData(mRootDir / resourceIndexDataFilePath, ecsGroup);
}

bool foeYamlImporter::importStateData(foeEditorNameMap *pEntityNameMap,
                                      std::vector<foeComponentPoolBase *> &componentPools) {
    if (!std::filesystem::exists(mRootDir / stateDirectoryPath))
        return true;

    for (auto &dirIt :
         std::filesystem::recursive_directory_iterator{mRootDir / stateDirectoryPath}) {
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
            yaml_read_id_required("", entityNode, &mGroupTranslator, entity);

            if (pEntityNameMap != nullptr) {
                std::string editorName;
                yaml_read_optional("editor_name", entityNode, editorName);

                if (!editorName.empty()) {
                    pEntityNameMap->add(entity, editorName);
                }
            }

            for (auto const &it : mGenerator->mComponentFns) {
                if (auto subNode = entityNode[it.first]; subNode) {
                    it.second(entityNode, &mGroupTranslator, entity, componentPools);
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
bool foeYamlImporter::importResourceDefinitions(
    foeEditorNameMap *pNameMap,
    std::vector<foeResourceLoaderBase *> &resourceLoaders,
    std::vector<foeResourcePoolBase *> &resourcePools) {
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
            yaml_read_id_required("", node, &mGroupTranslator, resource);

            // Editor Name
            if (pNameMap != nullptr) {
                std::string editorName;
                yaml_read_optional("editor_name", node, editorName);

                if (!editorName.empty()) {
                    pNameMap->add(resource, editorName);
                }
            }

            // Resource Type
            for (auto const &it : mGenerator->mResourceFns) {
                if (auto subNode = node[it.first]; subNode) {
                    foeResourceCreateInfoBase *pCreateInfo{nullptr};
                    it.second.pImport(node, &mGroupTranslator, &pCreateInfo);

                    if (it.second.pCreate != nullptr) {
                        if (!it.second.pCreate(resource, pCreateInfo, resourceLoaders,
                                               resourcePools))
                            return false;
                    }

                    delete pCreateInfo;
                    break;
                }
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

foeResourceCreateInfoBase *foeYamlImporter::getResource(foeId id) {
    if (!std::filesystem::exists(mRootDir / resourceDirectoryPath))
        return nullptr;

    YAML::Node rootNode;
    foeIdIndex index = foeIdGetIndex(id);

    for (auto &dirEntry :
         std::filesystem::recursive_directory_iterator{mRootDir / resourceDirectoryPath}) {
        if (std::filesystem::is_directory(dirEntry))
            continue;

        if (dirEntry.is_regular_file()) {
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
    }
GOT_RESOURCE_NODE:

    try {
        foeResourceCreateInfoBase *pCreateInfo{nullptr};

        for (auto const &it : mGenerator->mResourceFns) {
            if (auto subNode = rootNode[it.first]; subNode) {
                it.second.pImport(rootNode, &mGroupTranslator, &pCreateInfo);
                return pCreateInfo;
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