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

#include <foe/ecs/group_translator.hpp>

#include "error_code.hpp"

auto foeIdCreateTranslator(std::vector<foeIdGroupValueNameSet> const &source,
                           std::vector<foeIdGroupValueNameSet> const &destination,
                           foeIdGroup defaultGroup,
                           foeIdGroupTranslator *pTranslator) -> std::error_code {
    foeIdGroupTranslator newTranslator;

    for (auto const &src : source) {
        foeIdGroup dstGroup = FOE_INVALID_ID;
        for (auto const &dst : destination) {
            if (src.name == dst.name) {
                dstGroup = foeIdValueToGroup(dst.groupValue);
                break;
            }
        }

        if (dstGroup == FOE_INVALID_ID) {
            // Failed to find the matching group
            return FOE_ECS_ERROR_NO_MATCHING_DESTINATION_GROUP;
        }

        // Otherwise we found a valid translation, add it
        newTranslator.translations.emplace_back(foeIdGroupValueTranslation{
            .sourceGroupValue = src.groupValue,
            .destinationGroup = dstGroup,
        });
    }

    // Add the default value that 'persistent' IDs translate to
    newTranslator.translations.emplace_back(foeIdGroupValueTranslation{
        .sourceGroupValue = foeIdPersistentGroupValue,
        .destinationGroup = defaultGroup,
    });

    *pTranslator = std::move(newTranslator);

    return FOE_ECS_SUCCESS;
}

auto foeIdTranslateGroupValue(foeIdGroupTranslator const *pTranslator, foeIdGroupValue groupValue)
    -> foeIdGroup {
    for (auto const &it : pTranslator->translations) {
        if (it.sourceGroupValue == groupValue) {
            return it.destinationGroup;
        }
    }

    return FOE_INVALID_ID;
}