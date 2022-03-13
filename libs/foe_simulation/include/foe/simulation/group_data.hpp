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

#ifndef FOE_SIMULATION_GROUP_DATA_HPP
#define FOE_SIMULATION_GROUP_DATA_HPP

#include <foe/ecs/id.h>
#include <foe/ecs/index_generator.hpp>
#include <foe/imex/importer_base.hpp>
#include <foe/resource/create_info.h>
#include <foe/simulation/export.h>

#include <array>
#include <memory>
#include <string_view>

class foeGroupData {
  public:
    FOE_SIM_EXPORT bool addDynamicGroup(std::unique_ptr<foeIdIndexGenerator> &&pEntityIndices,
                                        std::unique_ptr<foeIdIndexGenerator> &&pResourceIndices,
                                        std::unique_ptr<foeImporterBase> &&pImporter);

    FOE_SIM_EXPORT bool setPersistentImporter(std::unique_ptr<foeImporterBase> &&pImporter);

    FOE_SIM_EXPORT auto entityIndices(foeIdGroup group) noexcept -> foeIdIndexGenerator *;
    FOE_SIM_EXPORT auto entityIndices(std::string_view groupName) noexcept -> foeIdIndexGenerator *;

    FOE_SIM_EXPORT auto resourceIndices(foeIdGroup group) noexcept -> foeIdIndexGenerator *;
    FOE_SIM_EXPORT auto resourceIndices(std::string_view groupName) noexcept
        -> foeIdIndexGenerator *;

    FOE_SIM_EXPORT auto importer(foeIdGroup group) noexcept -> foeImporterBase *;
    FOE_SIM_EXPORT auto importer(std::string_view groupName) noexcept -> foeImporterBase *;

    FOE_SIM_EXPORT auto persistentEntityIndices() noexcept -> foeIdIndexGenerator *;
    FOE_SIM_EXPORT auto persistentResourceIndices() noexcept -> foeIdIndexGenerator *;
    FOE_SIM_EXPORT auto persistentImporter() noexcept -> foeImporterBase *;

    FOE_SIM_EXPORT auto temporaryEntityIndices() noexcept -> foeIdIndexGenerator *;
    FOE_SIM_EXPORT auto temporaryResourceIndices() noexcept -> foeIdIndexGenerator *;

    // Used for resource loaders
    FOE_SIM_EXPORT foeResourceCreateInfo getResourceDefinition(foeId id);
    // Used for resource loaders
    FOE_SIM_EXPORT std::filesystem::path findExternalFile(std::filesystem::path externalFilePath);

  private:
    struct CombinedGroup {
        std::unique_ptr<foeIdIndexGenerator> pEntityIndices;
        std::unique_ptr<foeIdIndexGenerator> pResourceIndices;
        std::unique_ptr<foeImporterBase> pImporter;
    };

    static constexpr std::string_view cPersistentName = "Persistent";
    static constexpr std::string_view cTemporaryName = "Temporary";

    foeIdIndexGenerator mPersistentEntityIndices{foeIdPersistentGroup};
    foeIdIndexGenerator mPersistentResourceIndices{foeIdPersistentGroup};
    std::unique_ptr<foeImporterBase> mPersistentImporter;

    foeIdIndexGenerator mTemporaryEntityIndices{foeIdTemporaryGroup};
    foeIdIndexGenerator mTemporaryResourceIndices{foeIdTemporaryGroup};

    std::array<CombinedGroup, foeIdNumDynamicGroups> mDynamicGroups;
};

#endif // FOE_SIMULATION_GROUP_DATA_HPP