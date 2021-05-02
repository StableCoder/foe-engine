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

#include "distributed_yaml.hpp"

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_struct_cleanup.hpp>

#include "../resource_pools.hpp"
#include "distributed_yaml_generator.hpp"
#include "entity.hpp"

#include <string>
#include <string_view>

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view indexDataFilePath = "index_data.yml";
constexpr std::string_view resourcesDirectoryPath = "resources";
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
        FOE_LOG(General, Fatal, "Failed to load Yaml file: {}", e.what());
        return false;
    } catch (YAML::BadFile const &e) {
        FOE_LOG(General, Fatal, "YAML::LoadFile failed: {}", e.what());
        return false;
    }

    return true;
}

} // namespace

foeDistributedYamlImporter::foeDistributedYamlImporter(
    foeDistributedYamlImporterGenerator *pGenerator,
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
        FOE_LOG(General, Error, "{}", e.what())
        return false;
    }

    return true;
}

} // namespace

foeIdGroup foeDistributedYamlImporter::group() const noexcept { return mGroup; }

std::string foeDistributedYamlImporter::name() const noexcept { return mRootDir.stem().string(); }

void foeDistributedYamlImporter::setGroupTranslator(foeIdGroupTranslator &&groupTranslation) {
    mGroupTranslator = std::move(groupTranslation);
}

bool foeDistributedYamlImporter::getDependencies(
    std::vector<foeIdGroupValueNameSet> &dependencies) {
    YAML::Node node;
    if (!openYamlFile(mRootDir / dependenciesFilePath, node))
        return false;

    return importDependenciesFromNode(node, dependencies);
}

bool foeDistributedYamlImporter::getGroupIndexData(foeIdIndexGenerator &ecsGroup) {
    YAML::Node node;
    if (!openYamlFile(mRootDir / indexDataFilePath, node))
        return false;

    try {
        yaml_read_index_generator("", node, ecsGroup);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to parse Group State Index Data from Yaml file: {}",
                e.what())
        return false;
    }

    return true;
}

bool foeDistributedYamlImporter::importStateData(StatePools *pStatePools) {
    for (auto &dirIt :
         std::filesystem::recursive_directory_iterator{mRootDir / stateDirectoryPath}) {
        FOE_LOG(General, Info, "Visiting: {}", dirIt.path().string())
        if (std::filesystem::is_directory(dirIt))
            continue;

        if (!std::filesystem::is_regular_file(dirIt)) {
            FOE_LOG(General, Warning,
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
            auto entity = yaml_read_entity(entityNode, mGroup, &mGroupTranslator, pStatePools);
            if (entity == FOE_INVALID_ID) {
                return false;
            } else {
                FOE_LOG(General, Info, "Successfully parsed entity {}", foeIdToString(entity))
            }
        } catch (foeYamlException const &e) {
            FOE_LOG(General, Error, "Failed to parse entity state data: {}", e.what())
            return false;
        }
    }

    return true;
}

bool foeDistributedYamlImporter::importResourceDefinitions(foeEditorNameMap *pNameMap,
                                                           ResourcePools *pResourcePools,
                                                           ResourceLoaders *pResourceLoaders) {
    for (auto &dirIt :
         std::filesystem::recursive_directory_iterator{mRootDir / resourcesDirectoryPath}) {
        if (std::filesystem::is_directory(dirIt))
            continue;

        if (!std::filesystem::is_regular_file(dirIt)) {
            FOE_LOG(General, Warning,
                    "Resource file '{}' not a directory or regular file! Possible corruption!",
                    dirIt.path().string())
            return false;
        }

        // Is a regular file continue...
        YAML::Node node;
        if (!openYamlFile(dirIt, node))
            return false;

        foeId resource;
        std::string editorName;
        try {
            // ID
            yaml_read_id_required("", node, nullptr, foeIdTypeResource, resource);

            // Editor Name
            yaml_read_optional("editor_name", node, editorName);

            // Resource type
            std::string type;
            uint32_t version;
            yaml_read_required("type", node, type);
            yaml_read_required("version", node, version);

            auto key = std::make_tuple(type, version);

            auto searchIt = mGenerator->mImportFunctions.find(key);
            if (searchIt == mGenerator->mImportFunctions.end()) {
                // Failed to find importer, leave
                return false;
            }

            foeResourceCreateInfoBase *pCreateInfo{nullptr};
            searchIt->second(node, &mGroupTranslator, &pCreateInfo);
            std::unique_ptr<foeResourceCreateInfoBase> createInfo{pCreateInfo};

            if (type == "armature" && version == 1) {
                auto armature =
                    std::make_unique<foeArmature>(resource, &pResourceLoaders->armature);

                pResourcePools->armature.add(armature.release());
            } else if (type == "mesh" && version == 1) {
                auto mesh = std::make_unique<foeMesh>(resource, &pResourceLoaders->mesh);

                pResourcePools->mesh.add(mesh.release());
            } else if (type == "material" && version == 1) {
                auto material =
                    std::make_unique<foeMaterial>(resource, &pResourceLoaders->material);

                pResourcePools->material.add(material.release());

                auto *pMaterialCI = static_cast<foeMaterialCreateInfo *>(createInfo.get());
                if (pMaterialCI->hasRasterizationSCI)
                    vk_struct_cleanup(&pMaterialCI->rasterizationSCI);
                if (pMaterialCI->hasDepthStencilSCI)
                    vk_struct_cleanup(&pMaterialCI->depthStencilSCI);
                if (pMaterialCI->hasColourBlendSCI)
                    vk_struct_cleanup(&pMaterialCI->colourBlendSCI);
            } else if (type == "vertex_descriptor" && version == 1) {
                auto vertexDescriptor = std::make_unique<foeVertexDescriptor>(
                    resource, &pResourceLoaders->vertexDescriptor);

                pResourcePools->vertexDescriptor.add(vertexDescriptor.release());
            } else if (type == "shader" && version == 1) {
                auto shader = std::make_unique<foeShader>(resource, &pResourceLoaders->shader);

                pResourcePools->shader.add(shader.release());
            } else if (type == "image" && version == 1) {
                auto image = std::make_unique<foeImage>(resource, &pResourceLoaders->image);

                pResourcePools->image.add(image.release());
            } else {
                std::abort();
            }
        } catch (foeYamlException const &e) {
            FOE_LOG(General, Error, "Failed to import resource definition: {}", e.what());
            return false;
        } catch (std::exception const &e) {
            FOE_LOG(General, Error, "Failed to import resource definition: {}", e.what());
            return false;
        } catch (...) {
            FOE_LOG(General, Error, "Failed to import resource definition with unknown exception");
            return false;
        }

        if (pNameMap != nullptr) {
            pNameMap->add(resource, editorName);
        }
    }

    return true;
}

foeResourceCreateInfoBase *foeDistributedYamlImporter::getResource(foeId id) {
    YAML::Node rootNode;
    foeIdIndex index = foeIdGetIndex(id);
    for (auto &dirEntry :
         std::filesystem::recursive_directory_iterator{mRootDir / resourcesDirectoryPath}) {
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
        std::string type;
        uint32_t version;
        yaml_read_required("type", rootNode, type);
        yaml_read_required("version", rootNode, version);

        auto key = std::make_tuple(type, version);

        auto searchIt = mGenerator->mImportFunctions.find(key);
        if (searchIt == mGenerator->mImportFunctions.end()) {
            // Failed to find importer, leave
            return nullptr;
        }

        foeResourceCreateInfoBase *pCreateInfo{nullptr};
        searchIt->second(rootNode, &mGroupTranslator, &pCreateInfo);
        return pCreateInfo;
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import resource definition: {}", e.what());
        return nullptr;
    } catch (std::exception const &e) {
        FOE_LOG(General, Error, "Failed to import resource definition: {}", e.what());
        return nullptr;
    } catch (...) {
        FOE_LOG(General, Error, "Failed to import resource definition with unknown exception");
        return nullptr;
    }
}