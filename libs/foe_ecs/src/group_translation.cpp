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

#include <foe/ecs/group_translation.hpp>

#include <foe/ecs/groups.hpp>

bool foeGroupTranslation::generateTranslations(std::vector<StateDataDependency> const &dependencies,
                                               foeEcsGroups &idGroups) {
    std::vector<Set> newSets;
    newSets.reserve(dependencies.size());

    for (auto const &it : dependencies) {
        auto *targetGroup = idGroups.group(it.name);

        if (targetGroup == nullptr)
            return false;

        newSets.emplace_back(foeGroupTranslation::Set{
            .normalizedSourceGroup = it.group,
            .target = targetGroup->groupID(),
        });
    }

    mTranslations = std::move(newSets);

    return true;
}

bool foeGroupTranslation::targetFromNormalizedGroup(foeIdGroup normalizedGroup, foeIdGroup &group) {
    for (auto const &it : mTranslations) {
        if (it.normalizedSourceGroup == normalizedGroup) {
            group = it.target;
            return true;
        }
    }

    return false;
}