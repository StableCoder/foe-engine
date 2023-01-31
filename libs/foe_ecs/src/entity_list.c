// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/entity_list.h>

#include "result.h"

#include <stdlib.h>
#include <string.h>

typedef struct EntityList {
    foeEntityID *pList;
    uint32_t count;
    uint32_t capacity;
} EntityList;

FOE_DEFINE_HANDLE_CASTS(entity_list, EntityList, foeEcsEntityList)

foeResultSet foeEcsCreateEntityList(foeEcsEntityList *pEntityList) {
    EntityList *pNewEntityList = malloc(sizeof(EntityList));
    if (pNewEntityList == NULL)
        return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);
    memset(pNewEntityList, 0, sizeof(EntityList));

    *pEntityList = entity_list_to_handle(pNewEntityList);
    return to_foeResult(FOE_ECS_SUCCESS);
}

void foeEcsDestroyEntityList(foeEcsEntityList entityList) {
    EntityList *pEntityList = entity_list_from_handle(entityList);

    free(pEntityList->pList);
    free(pEntityList);
}

foeResultSet foeEcsResetEntityList(foeEcsEntityList entityList,
                                   uint32_t listCount,
                                   uint32_t const *pEntityListCounts,
                                   foeEntityID const *const *ppEntityLists) {
    EntityList *pEntityList = entity_list_from_handle(entityList);

    uint32_t totalCount = 0;
    for (uint32_t i = 0; i < listCount; ++i) {
        totalCount += pEntityListCounts[i];
    }

    if (pEntityList->capacity < totalCount) {
        foeEntityID *pNewList = malloc(totalCount * sizeof(foeEntityID));
        if (pNewList == NULL)
            return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);

        free(pEntityList->pList);

        pEntityList->pList = pNewList;
        pEntityList->capacity = totalCount;
    }

    if (totalCount > 0) {
        foeEntityID *pDst = pEntityList->pList;
        for (uint32_t i = 0; i < listCount; ++i) {
            memcpy(pDst, ppEntityLists[i], pEntityListCounts[i] * sizeof(foeEntityID));
            pDst += pEntityListCounts[i];
        }
    }
    pEntityList->count = totalCount;

    return to_foeResult(FOE_ECS_SUCCESS);
}

uint32_t foeEcsEntityListSize(foeEcsEntityList entityList) {
    EntityList *pEntityList = entity_list_from_handle(entityList);

    return pEntityList->count;
}

foeEntityID const *foeEcsEntityListPtr(foeEcsEntityList entityList) {
    EntityList *pEntityList = entity_list_from_handle(entityList);

    return pEntityList->pList;
}