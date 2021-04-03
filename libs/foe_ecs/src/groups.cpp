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

#include <foe/ecs/groups.hpp>

bool foeEcsGroups::addGroup(std::unique_ptr<foeEcsIndexGenerator> &&newGroup) noexcept {
    auto const normalizedGroup = foeEcsGetNormalizedGroupID(newGroup->groupID());

    // Must be a group withing the general group values
    if (normalizedGroup >= MaxGeneralGroups)
        return false;

    // Don't do it if there's a group already there
    if (mGroups[normalizedGroup] != nullptr)
        return false;

    // Check against duplicate group names
    if (newGroup->name() == "Temporary" || newGroup->name() == "Persistent")
        return false;

    for (auto const &group : mGroups) {
        if (group != nullptr && group->name() == newGroup->name()) {
            return false;
        }
    }

    mGroups[normalizedGroup] = std::move(newGroup);
    return true;
}

void foeEcsGroups::removeGroup(foeIdGroup groupID) noexcept {
    auto const normalizedGroup = foeEcsGetNormalizedGroupID(groupID);

    if (normalizedGroup >= MaxGeneralGroups)
        return;

    mGroups[normalizedGroup].reset();
}

foeEcsIndexGenerator *foeEcsGroups::group(foeIdGroup groupID) noexcept {
    auto group = foeEcsGetGroupID(groupID);
    if (group == Persistent) {
        return persistentGroup();
    } else if (group == Temporary) {
        return temporaryGroup();
    }

    auto const normalizedGroup = foeEcsGetNormalizedGroupID(groupID);

    return mGroups[normalizedGroup].get();
}

foeEcsIndexGenerator *foeEcsGroups::group(std::string_view groupName) noexcept {
    if (groupName == "Persistent") {
        return persistentGroup();
    } else if (groupName == "Temporary") {
        return temporaryGroup();
    }

    for (auto const &group : mGroups) {
        if (group != nullptr && group->name() == groupName) {
            return group.get();
        }
    }

    return nullptr;
}

foeEcsIndexGenerator *foeEcsGroups::persistentGroup() noexcept { return &mPersistentGroup; }

foeEcsIndexGenerator *foeEcsGroups::temporaryGroup() noexcept { return &mTemporaryGroup; }