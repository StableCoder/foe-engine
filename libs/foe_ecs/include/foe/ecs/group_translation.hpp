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

#ifndef FOE_ECS_GROUP_TRANSLATION_HPP
#define FOE_ECS_GROUP_TRANSLATION_HPP

#include <foe/ecs/export.h>
#include <foe/ecs/id.hpp>

#include <filesystem>
#include <vector>

class foeEcsGroups;

struct StateDataDependency {
    std::string name;
    foeIdGroup group;
    std::filesystem::path path;
};

class foeGroupTranslation {
  public:
    FOE_ECS_EXPORT bool generateTranslations(std::vector<StateDataDependency> const &dependencies,
                                             foeEcsGroups &idGroups);

    FOE_ECS_EXPORT
    bool targetFromNormalizedGroup(foeIdGroup normalizedGroup, foeIdGroup &group);

  public:
    struct Set {
        foeIdGroup normalizedSourceGroup;
        foeIdGroup target;
    };

    std::vector<Set> mTranslations;
};

#endif // FOE_ECS_GROUP_TRANSLATION_HPP