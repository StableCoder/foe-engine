// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_GROUP_DATA_HPP
#define FOE_SIMULATION_GROUP_DATA_HPP

#include <foe/ecs/id.h>
#include <foe/ecs/indexes.h>
#include <foe/imex/importer_base.hpp>
#include <foe/resource/create_info.h>
#include <foe/simulation/export.h>

#include <array>
#include <memory>
#include <string_view>

class foeGroupData {
  public:
    FOE_SIM_EXPORT foeGroupData();
    FOE_SIM_EXPORT ~foeGroupData();

    FOE_SIM_EXPORT bool addDynamicGroup(foeEcsIndexes entityIndexes,
                                        foeEcsIndexes resourceIndexes,
                                        std::unique_ptr<foeImporterBase> &&pImporter);

    FOE_SIM_EXPORT bool setPersistentImporter(std::unique_ptr<foeImporterBase> &&pImporter);

    FOE_SIM_EXPORT auto entityIndexes(foeIdGroup group) noexcept -> foeEcsIndexes;
    FOE_SIM_EXPORT auto entityIndexes(std::string_view groupName) noexcept -> foeEcsIndexes;

    FOE_SIM_EXPORT auto resourceIndexes(foeIdGroup group) noexcept -> foeEcsIndexes;
    FOE_SIM_EXPORT auto resourceIndexes(std::string_view groupName) noexcept -> foeEcsIndexes;

    FOE_SIM_EXPORT auto importer(foeIdGroup group) noexcept -> foeImporterBase *;
    FOE_SIM_EXPORT auto importer(std::string_view groupName) noexcept -> foeImporterBase *;

    FOE_SIM_EXPORT auto persistentEntityIndexes() noexcept -> foeEcsIndexes;
    FOE_SIM_EXPORT auto persistentResourceIndexes() noexcept -> foeEcsIndexes;
    FOE_SIM_EXPORT auto persistentImporter() noexcept -> foeImporterBase *;

    FOE_SIM_EXPORT auto temporaryEntityIndexes() noexcept -> foeEcsIndexes;
    FOE_SIM_EXPORT auto temporaryResourceIndexes() noexcept -> foeEcsIndexes;

    // Used for resource loaders
    FOE_SIM_EXPORT foeResourceCreateInfo getResourceDefinition(foeId id);
    // Used for resource loaders
    FOE_SIM_EXPORT std::filesystem::path findExternalFile(std::filesystem::path externalFilePath);

  private:
    struct CombinedGroup {
        foeEcsIndexes entityIndexes{FOE_NULL_HANDLE};
        foeEcsIndexes resourceIndexes{FOE_NULL_HANDLE};
        std::unique_ptr<foeImporterBase> pImporter;

        ~CombinedGroup();
    };

    static constexpr std::string_view cPersistentName = "Persistent";
    static constexpr std::string_view cTemporaryName = "Temporary";

    foeEcsIndexes mPersistentEntityIndexes{FOE_NULL_HANDLE};
    foeEcsIndexes mPersistentResourceIndexes{FOE_NULL_HANDLE};
    std::unique_ptr<foeImporterBase> mPersistentImporter;

    foeEcsIndexes mTemporaryEntityIndexes{FOE_NULL_HANDLE};
    foeEcsIndexes mTemporaryResourceIndexes{FOE_NULL_HANDLE};

    std::array<CombinedGroup, foeIdNumDynamicGroups> mDynamicGroups;
};

#endif // FOE_SIMULATION_GROUP_DATA_HPP