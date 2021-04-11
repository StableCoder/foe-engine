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

#ifndef FOE_ECS_GROUPS_HPP
#define FOE_ECS_GROUPS_HPP

#include <foe/ecs/export.h>
#include <foe/ecs/id.hpp>
#include <foe/ecs/index_generator.hpp>

#include <array>
#include <memory>

class foeEcsGroups {
  public:
    FOE_ECS_EXPORT bool addGroup(std::unique_ptr<foeEcsIndexGenerator> &&group) noexcept;
    FOE_ECS_EXPORT void removeGroup(foeIdGroup groupID) noexcept;

    FOE_ECS_EXPORT foeEcsIndexGenerator *group(foeIdGroup groupID) noexcept;
    FOE_ECS_EXPORT foeEcsIndexGenerator *group(std::string_view groupName) noexcept;

    FOE_ECS_EXPORT foeEcsIndexGenerator *persistentGroup() noexcept;
    FOE_ECS_EXPORT foeEcsIndexGenerator *temporaryGroup() noexcept;

  private:
    foeEcsIndexGenerator mPersistentGroup{"Persistent", foePersistentGroup};
    foeEcsIndexGenerator mTemporaryGroup{"Temporary", foeTemporaryGroup};

    std::array<std::unique_ptr<foeEcsIndexGenerator>, foeMaxGeneralGroups> mGroups;
};

#endif // FOE_ECS_GROUPS_HPP