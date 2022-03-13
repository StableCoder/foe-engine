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

#ifndef FOE_ECS_GROUP_TRANSLATOR_HPP
#define FOE_ECS_GROUP_TRANSLATOR_HPP

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>

#include <string>
#include <system_error>
#include <vector>

// Pairs an idGroupValue with a name, used when creating a translator
struct foeIdGroupValueNameSet {
    foeIdGroupValue groupValue;
    std::string name;
};

struct foeIdGroupValueTranslation {
    foeIdGroupValue sourceGroupValue;
    foeIdGroup destinationGroup;
};

struct foeIdGroupTranslator {
    std::vector<foeIdGroupValueTranslation> translations;
};

/** @brief Creates an GroupID translator
 * @param source Set of GroupIDs that the dataset was originally created with
 * @param destination Set of GroupIDs that the dataset is being imported to, which the source Groups
 * will need to be translatable to
 * @param pTranslator The new translator, nullptr if an issue occurred
 */
FOE_ECS_EXPORT auto foeIdCreateTranslator(std::vector<foeIdGroupValueNameSet> const &source,
                                          std::vector<foeIdGroupValueNameSet> const &destination,
                                          foeIdGroupTranslator *pTranslator) -> std::error_code;

/** @brief Translates a given group value through the given translator
 * @param pTranslator Set of group translations
 * @param groupValue Value to translate, if possible
 * @return The translated foeIdGroup, or 'foeIdTemporaryGroup' if no translation was found.
 */
FOE_ECS_EXPORT auto foeIdTranslateGroupValue(foeIdGroupTranslator const *pTranslator,
                                             foeIdGroupValue groupValue) -> foeIdGroup;

#endif // FOE_ECS_GROUP_TRANSLATOR_HPP