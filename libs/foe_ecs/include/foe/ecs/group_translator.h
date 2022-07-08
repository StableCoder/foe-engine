// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_GROUP_TRANSLATOR_H
#define FOE_ECS_GROUP_TRANSLATOR_H

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeEcsGroupTranslator)

FOE_ECS_EXPORT foeResult foeEcsCreateGroupTranslator(uint32_t originalCount,
                                                     char const **ppOriginalNames,
                                                     foeIdGroup *pOriginalGroups,
                                                     uint32_t translatedCount,
                                                     char const **ppTranslatedNames,
                                                     foeIdGroup *pTranslatedGroups,
                                                     foeEcsGroupTranslator *pGroupTranslator);

FOE_ECS_EXPORT void foeEcsDestroyGroupTranslator(foeEcsGroupTranslator groupTranslator);

FOE_ECS_EXPORT foeResult foeEcsGetTranslatedGroup(foeEcsGroupTranslator groupTranslator,
                                                  foeIdGroup originalGroup,
                                                  foeIdGroup *pTranslatedGroup);

FOE_ECS_EXPORT foeResult foeEcsGetOriginalGroup(foeEcsGroupTranslator groupTranslator,
                                                foeIdGroup translatedGroup,
                                                foeIdGroup *pOriginalGroup);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_GROUP_TRANSLATOR_H