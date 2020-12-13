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

#ifndef FOE_MODEL_FILE_IMPORTER_PLUGINS_HPP
#define FOE_MODEL_FILE_IMPORTER_PLUGINS_HPP

#include <foe/model/export.h>
#include <foe/model/importer.hpp>
#include <foe/plugin.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
 * @brief A common base for plugins used to import various file types
 */
class foeModelFileImporterPlugin {
  public:
    foeModelFileImporterPlugin(foePlugin plugin) : mPlugin{plugin} {}

    virtual ~foeModelFileImporterPlugin() {
        if (mPlugin != FOE_NULL_HANDLE) {
            foeDestroyPlugin(mPlugin);
        }
    }

    virtual std::string pluginName() = 0;

    virtual std::vector<std::string> supportedFileFormats() = 0;

    virtual std::unique_ptr<foeModelImporter> createImporter(char const *pFile) = 0;

  private:
    foePlugin mPlugin;
};

/**
 * @brief Given the path to the plugin (shared library), returns a plugin object
 * @param pluginPath Path to the plugin
 * @return Pointer to the impoterter plugin, nullptr if it failed
 */
FOE_MODEL_EXPORT std::unique_ptr<foeModelFileImporterPlugin> foeModelLoadFileImporterPlugin(
    std::filesystem::path pluginPath);

#endif // FOE_MODEL_FILE_IMPORTER_PLUGINS_HPP