/*
    Copyright (C) 2022 George Cave.

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

FOE_ECS_EXPORT foeErrorCode foeEcsCreateGroupTranslator(uint32_t originalCount,
                                                        char const **ppOriginalNames,
                                                        foeIdGroup *pOriginalGroups,
                                                        uint32_t translatedCount,
                                                        char const **ppTranslatedNames,
                                                        foeIdGroup *pTranslatedGroups,
                                                        foeEcsGroupTranslator *pGroupTranslator);

FOE_ECS_EXPORT void foeEcsDestroyGroupTranslator(foeEcsGroupTranslator groupTranslator);

FOE_ECS_EXPORT foeErrorCode foeEcsGetTranslatedGroup(foeEcsGroupTranslator groupTranslator,
                                                     foeIdGroup originalGroup,
                                                     foeIdGroup *pTranslatedGroup);

FOE_ECS_EXPORT foeErrorCode foeEcsGetOriginalGroup(foeEcsGroupTranslator groupTranslator,
                                                   foeIdGroup translatedGroup,
                                                   foeIdGroup *pOriginalGroup);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_GROUP_TRANSLATOR_H