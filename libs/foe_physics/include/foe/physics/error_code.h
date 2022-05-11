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

#ifndef FOE_PHYSICS_ERROR_CODE_H
#define FOE_PHYSICS_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foePhysicsResult {
    FOE_PHYSICS_SUCCESS = 0,
    // Loaders
    FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED,
    FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO,
    // Physics System
    FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_LOADER,
    FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES,
    FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS,
    FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_ERROR_CODE_H