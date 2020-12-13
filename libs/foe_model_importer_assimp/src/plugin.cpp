/*
    Copyright (C) 2020 George Cave.

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

#include "plugin.h"

#include <foe/model/file_importer_plugins.hpp>

#include "importer.hpp"

namespace {

class foeModelAssimpFileImporterPlugin : public foeModelFileImporterPlugin {
  public:
    foeModelAssimpFileImporterPlugin(foePlugin plugin) : foeModelFileImporterPlugin{plugin} {}

    std::string pluginName() { return "Asset Importer (assimp)"; }

    std::vector<std::string> supportedFileFormats() {
        return {
            ".dae",
            ".fbx",
        };
    }

    std::unique_ptr<foeModelImporter> createImporter(char const *pFile) {
        return std::unique_ptr<foeModelImporter>(new foeModelFileAssimpImporter{pFile});
    }
};

} // namespace

extern "C" foeModelFileImporterPlugin *createModelFileImporterPlugin(foePlugin plugin) {
    return new foeModelAssimpFileImporterPlugin(plugin);
}