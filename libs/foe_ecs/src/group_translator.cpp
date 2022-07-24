// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/group_translator.h>

#include "result.h"

#include <stdlib.h>
#include <string.h>

namespace {

struct Translation {
    foeIdGroup originalGroup;
    foeIdGroup translatedGroup;
};

} // namespace

extern "C" foeResultSet foeEcsCreateGroupTranslator(uint32_t originalCount,
                                                    char const **ppOriginalNames,
                                                    foeIdGroup *pOriginalGroups,
                                                    uint32_t translatedCount,
                                                    char const **ppTranslatedNames,
                                                    foeIdGroup *pTranslatedGroups,
                                                    foeEcsGroupTranslator *pGroupTranslator) {
    foeEcsResult result{FOE_ECS_SUCCESS};
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
            result = FOE_ECS_ERROR_NO_MATCHING_GROUP;
            break;
        }
    }

    if (result != FOE_ECS_SUCCESS) {
        // If an error, then free the data
        free(pNewGroupTranslator);
    } else {
        // If no error, set the output variable
        *pGroupTranslator = (foeEcsGroupTranslator)pNewGroupTranslator;
    }

    return to_foeResult(result);
}

extern "C" void foeEcsDestroyGroupTranslator(foeEcsGroupTranslator groupTranslator) {
    free(groupTranslator);
}

extern "C" foeResultSet foeEcsGetTranslatedGroup(foeEcsGroupTranslator groupTranslator,
                                                 foeIdGroup originalGroup,
                                                 foeIdGroup *pTranslatedGroup) {
    uint32_t const *pCount = (uint32_t const *)groupTranslator;
    Translation const *pTranslations = (Translation const *)(pCount + 1);

    for (uint32_t i = 0; i < *pCount; ++i) {
        if (pTranslations[i].originalGroup == originalGroup) {
            *pTranslatedGroup = pTranslations[i].translatedGroup;
            return to_foeResult(FOE_ECS_SUCCESS);
        }
    }

    return to_foeResult(FOE_ECS_ERROR_NO_MATCHING_GROUP);
}

extern "C" foeResultSet foeEcsGetOriginalGroup(foeEcsGroupTranslator groupTranslator,
                                               foeIdGroup translatedGroup,
                                               foeIdGroup *pOriginalGroup) {
    uint32_t const *pCount = (uint32_t const *)groupTranslator;
    Translation const *pTranslations = (Translation const *)(pCount + 1);

    for (uint32_t i = 0; i < *pCount; ++i) {
        if (pTranslations[i].translatedGroup == translatedGroup) {
            *pOriginalGroup = pTranslations[i].originalGroup;
            return to_foeResult(FOE_ECS_SUCCESS);
        }
    }

    return to_foeResult(FOE_ECS_ERROR_NO_MATCHING_GROUP);
}