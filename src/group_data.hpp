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

#ifndef GROUP_DATA_HPP
#define GROUP_DATA_HPP

#include <foe/ecs/id.hpp>
#include <foe/ecs/index_generator.hpp>

#include <foe/imex/importer_base.hpp>

#include <array>
#include <memory>
#include <string_view>

class foeGroupData {
  public:
    bool addDynamicGroup(std::unique_ptr<foeIdIndexGenerator> &&pEntityIndices,
                         std::unique_ptr<foeIdIndexGenerator> &&pResourceIndices,
                         std::unique_ptr<foeImporterBase> &&pImporter);

    bool setPersistentImporter(std::unique_ptr<foeImporterBase> &&pImporter);

    auto entityIndices(foeIdGroup group) noexcept -> foeIdIndexGenerator *;
    auto entityIndices(std::string_view groupName) noexcept -> foeIdIndexGenerator *;

    auto resourceIndices(foeIdGroup group) noexcept -> foeIdIndexGenerator *;
    auto resourceIndices(std::string_view groupName) noexcept -> foeIdIndexGenerator *;

    auto importer(foeIdGroup group) noexcept -> foeImporterBase *;
    auto importer(std::string_view groupName) noexcept -> foeImporterBase *;

    auto persistentEntityIndices() noexcept -> foeIdIndexGenerator *;
    auto persistentResourceIndices() noexcept -> foeIdIndexGenerator *;
    auto persistentImporter() noexcept -> foeImporterBase *;

    auto temporaryEntityIndices() noexcept -> foeIdIndexGenerator *;
    auto temporaryResourceIndices() noexcept -> foeIdIndexGenerator *;

    bool getResourceDefinition(foeId id, foeResourceCreateInfoBase **ppCreateInfo);

  private:
    struct CombinedGroup {
        std::unique_ptr<foeIdIndexGenerator> pEntityIndices;
        std::unique_ptr<foeIdIndexGenerator> pResourceIndices;
        std::unique_ptr<foeImporterBase> pImporter;
    };

    static constexpr std::string_view cPersistentName = "Persistent";
    static constexpr std::string_view cTemporaryName = "Temporary";

    foeIdIndexGenerator mPersistentEntityIndices{"", foeIdPersistentGroup};
    foeIdIndexGenerator mPersistentResourceIndices{"", foeIdPersistentGroup};
    std::unique_ptr<foeImporterBase> mPersistentImporter;

    foeIdIndexGenerator mTemporaryEntityIndices{"", foeIdTemporaryGroup};
    foeIdIndexGenerator mTemporaryResourceIndices{"", foeIdTemporaryGroup};

    std::array<CombinedGroup, foeIdNumDynamicGroups> mDynamicGroups;
};

#endif // GROUP_DATA_HPP