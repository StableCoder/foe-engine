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

#include <foe/ecs/group_translator.h>

#include "error_code.hpp"

#include <string.h>

namespace {

struct Translation {
    foeIdGroup originalGroup;
    foeIdGroup translatedGroup;
};

} // namespace

extern "C" foeErrorCode foeEcsCreateGroupTranslator(uint32_t originalCount,
                                                    char const **ppOriginalNames,
                                                    foeIdGroup *pOriginalGroups,
                                                    uint32_t translatedCount,
                                                    char const **ppTranslatedNames,
                                                    foeIdGroup *pTranslatedGroups,
                                                    foeEcsGroupTranslator *pGroupTranslator) {
    std::error_code errC{FOE_ECS_SUCCESS};
    size_t translatorSize = sizeof(uint32_t) + (originalCount * sizeof(Translation));
    void *pNewGroupTranslator = malloc(translatorSize);

    uint32_t *pCount = (uint32_t *)pNewGroupTranslator;
    *pCount = originalCount;

    Translation *pTranslations = (Translation *)(pCount + 1);
    for (uint32_t i = 0; i < originalCount; ++i) {
        bool found = false;

        for (uint32_t j = 0; j < translatedCount; ++j) {
            if (strcmp(ppOriginalNames[i], ppTranslatedNames[j]) == 0) {
                *pTranslations = Translation{
                    .originalGroup = pOriginalGroups[i],
                    .translatedGroup = pTranslatedGroups[j],
                };
                ++pTranslations;
                found = true;
                break;
            }
        }

        if (!found) {
            errC = FOE_ECS_ERROR_NO_MATCHING_GROUP;
            break;
        }
    }

    if (errC) {
        // If an error, then free the data
        free(pNewGroupTranslator);
    } else {
        // If no error, set the output variable
        *pGroupTranslator = (foeEcsGroupTranslator)pNewGroupTranslator;
    }

    return foeToErrorCode(errC);
}

extern "C" void foeEcsDestroyGroupTranslator(foeEcsGroupTranslator groupTranslator) {
    free(groupTranslator);
}

extern "C" foeErrorCode foeEcsGetTranslatedGroup(foeEcsGroupTranslator groupTranslator,
                                                 foeIdGroup originalGroup,
                                                 foeIdGroup *pTranslatedGroup) {
    uint32_t const *pCount = (uint32_t const *)groupTranslator;
    Translation const *pTranslations = (Translation const *)(pCount + 1);

    for (uint32_t i = 0; i < *pCount; ++i) {
        if (pTranslations[i].originalGroup == originalGroup) {
            *pTranslatedGroup = pTranslations[i].translatedGroup;
            return foeToErrorCode(FOE_ECS_SUCCESS);
        }
    }

    return foeToErrorCode(FOE_ECS_ERROR_NO_MATCHING_GROUP);
}

extern "C" foeErrorCode foeEcsGetOriginalGroup(foeEcsGroupTranslator groupTranslator,
                                               foeIdGroup translatedGroup,
                                               foeIdGroup *pOriginalGroup) {
    uint32_t const *pCount = (uint32_t const *)groupTranslator;
    Translation const *pTranslations = (Translation const *)(pCount + 1);

    for (uint32_t i = 0; i < *pCount; ++i) {
        if (pTranslations[i].translatedGroup == translatedGroup) {
            *pOriginalGroup = pTranslations[i].originalGroup;
            return foeToErrorCode(FOE_ECS_SUCCESS);
        }
    }

    return foeToErrorCode(FOE_ECS_ERROR_NO_MATCHING_GROUP);
}