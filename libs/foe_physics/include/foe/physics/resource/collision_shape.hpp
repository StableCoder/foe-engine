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

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/physics/export.h>
#include <foe/simulation/core/resource.hpp>

class foeCollisionShapeLoader;

struct FOE_PHYSICS_EXPORT foeCollisionShape : public foeResourceBase {
    friend foeCollisionShapeLoader;

    foeCollisionShape(foeResourceID resource, foeResourceFns const *pResourceFns);
    ~foeCollisionShape();

    void loadCreateInfo();
    void loadResource(bool refreshCreateInfo);
    void unloadResource();

    struct Data {
        void *pUnloadContext{nullptr};
        void (*pUnloadFn)(void *, void *, uint32_t, bool){nullptr};
        std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;
        std::unique_ptr<btCollisionShape> collisionShape;
    } data;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP