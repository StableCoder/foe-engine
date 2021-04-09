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

#include <foe/log.hpp>

namespace {

auto parseFileStem(std::filesystem::path const &path) -> foeId {
    std::string fullStem{path.stem().string()};

    // Group Stem
    auto firstStemEnd = fullStem.find_first_of('_');
    if (firstStemEnd == std::string::npos) {
        return FOE_INVALID_ID;
    }
    std::string idGroupStem = fullStem.substr(0, firstStemEnd);
    ++firstStemEnd;

    // Index Stem
    auto secondStemEnd = fullStem.find_first_of('_', firstStemEnd);
    std::string idIndexStem = fullStem.substr(firstStemEnd, secondStemEnd - firstStemEnd);
    if (idIndexStem.empty())
        return FOE_INVALID_ID;

    // @todo Add group parsing here
    foeIdGroup idGroup = 0;

    // Index parsing
    foeIdIndex idIndex = FOE_INVALID_ID;
    char *endCh;
    idIndex = std::strtoul(idIndexStem.c_str(), &endCh, 0);

    return idGroup | idIndex;
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

foeDistributedYamlImporter::foeDistributedYamlImporter(std::filesystem::path rootDir) :
    mRootDir{std::move(rootDir)} {}

bool foeDistributedYamlImporter::addImporter(std::string type,
                                             uint32_t version,
                                             ImportFunc function) {
    auto key = std::make_tuple(type, version);

    auto searchIt = mImportFunctions.find(key);
    if (searchIt != mImportFunctions.end()) {
        FOE_LOG(General, Error,
                "Could not add DistributedYamlImporter function for {}:{}, as it already exists",
                type, version);
        return false;
    }

    FOE_LOG(General, Info, "Adding DistributedYamlImporter function for {}:{}", type, version);
    mImportFunctions[key] = function;
    return true;
}

bool foeDistributedYamlImporter::removeImporter(std::string type, uint32_t version) {
    auto key = std::make_tuple(type, version);

    auto searchIt = mImportFunctions.find(key);
    if (searchIt == mImportFunctions.end()) {
        FOE_LOG(General, Error,
                "Could not remove DistributedYamlImporter function for {}:{}, as it isn't added",
                type, version);
        return false;
    }

    mImportFunctions.erase(searchIt);
    return true;
}

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool foeDistributedYamlImporter::getResource(foeId id, foeResourceCreateInfoBase **ppCreateInfo) {
    YAML::Node rootNode;
    for (auto &dirEntry :
         std::filesystem::recursive_directory_iterator{mRootDir / cResourceSubDir}) {
        if (dirEntry.is_regular_file()) {
            foeId fileId = parseFileStem(dirEntry);

            if (fileId == FOE_INVALID_ID || fileId != id)
                continue;

            if (fileId == id) {
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

        auto searchIt = mImportFunctions.find(key);
        if (searchIt == mImportFunctions.end()) {
            // Failed to find importer, leave
            return false;
        }

        searchIt->second(rootNode, ppCreateInfo);
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

    return true;
}