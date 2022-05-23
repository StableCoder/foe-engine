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

#ifndef FOE_PHYSICS_REGISTRATION_H
#define FOE_PHYSICS_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/physics/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_PHYSICS_EXPORT int foePhysicsFunctionalityID();

FOE_PHYSICS_EXPORT foeResult foePhysicsRegisterFunctionality();

FOE_PHYSICS_EXPORT void foePhysicsDeregisterFunctionality();

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_REGISTRATION_H