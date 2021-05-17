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

#include <foe/physics/system.hpp>

#include <foe/log.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/rigid_body.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "bt_glm_conversion.hpp"

#include <cassert>
#include <memory>

namespace {

std::vector<foeEntityID> mAwaitingResources;

std::unique_ptr<btBroadphaseInterface> broadphase{new btDbvtBroadphase{}};

std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig{
    new btDefaultCollisionConfiguration{}};

std::unique_ptr<btCollisionDispatcher> collisionDispatcher{
    new btCollisionDispatcher{collisionConfig.get()}};

std::unique_ptr<btSequentialImpulseConstraintSolver> solver{
    new btSequentialImpulseConstraintSolver{}};

std::unique_ptr<btDiscreteDynamicsWorld> physWorld{new btDiscreteDynamicsWorld{
    collisionDispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get()}};

} // namespace

void initPhysics() { physWorld->setGravity(btVector3{0, -9.8, 0}); }

namespace {

void removeObject(foeDataPool<foeEntityID, foePhysRigidBody> &rigidBodyPool,
                  foeEntityID entity,
                  foePhysRigidBody *pRigidBody) {
    // RigidBody
    if (pRigidBody == nullptr) {
        size_t dataOffset = rigidBodyPool.find(entity);
        if (dataOffset != rigidBodyPool.size()) {
            pRigidBody = rigidBodyPool.begin<1>() + dataOffset;
        } else {
            return;
        }
    }

    // If inactive
    if (pRigidBody->rigidBody == nullptr)
        return;

    physWorld->removeRigidBody(pRigidBody->rigidBody.get());
    pRigidBody->rigidBody.reset();
}

void addObject(foePhysCollisionShapeLoader &collisionShapeLoader,
               foePhysCollisionShapePool &collisionShapePool,
               foeDataPool<foeEntityID, foePhysRigidBody> &rigidBodyPool,
               foeDataPool<foeEntityID, std::unique_ptr<Position3D>> &positionPool,
               foeEntityID entity,
               foePhysRigidBody *pRigidBody,
               Position3D *pPosition,
               foePhysCollisionShape *pCollisionShape) {
    // RigidBody
    if (pRigidBody == nullptr) {
        size_t dataOffset = rigidBodyPool.find(entity);
        if (dataOffset != rigidBodyPool.size()) {
            pRigidBody = rigidBodyPool.begin<1>() + dataOffset;
        } else {
            return;
        }
    }

    // If already active
    if (pRigidBody->rigidBody != nullptr)
        return;

    // Position3D
    if (pPosition == nullptr) {
        size_t dataOffset = positionPool.find(entity);
        if (dataOffset != positionPool.size()) {
            pPosition = (positionPool.begin<1>() + dataOffset)->get();
        } else {
            return;
        }
    }

    // CollisionShape
    while (pCollisionShape == nullptr) {
        pCollisionShape = collisionShapePool.find(pRigidBody->collisionShape);
        if (pCollisionShape == nullptr) {
            // Make sure it's not an invalid ID, as if it is, leave
            if (pRigidBody->collisionShape != FOE_INVALID_ID)
                return;

            // Not found, a valid ID, add it as a resource now
            auto *pNewCollisionShape =
                new foePhysCollisionShape{pRigidBody->collisionShape, &collisionShapeLoader};

            if (!collisionShapePool.add(pNewCollisionShape)) {
                delete pNewCollisionShape;
            }
        }
    }
    if (pCollisionShape->loadState != foeResourceLoadState::Loaded) {
        if (pCollisionShape->loadState == foeResourceLoadState::Failed) {
            return;
        }

        pCollisionShape->requestLoad();
        mAwaitingResources.emplace_back(entity);
        return;
    }

    // We have everything we need now
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI{
        pRigidBody->mass, nullptr, pCollisionShape->data.collisionShape.get()};
    rigidBodyCI.m_startWorldTransform =
        glmToBtTransform(glm::vec3{0, 4, 0}, glm::quat{glm::vec3{0, 0, 0}});

    pRigidBody->rigidBody.reset(new btRigidBody{rigidBodyCI});

    physWorld->addRigidBody(pRigidBody->rigidBody.get());
}

} // namespace

void processPhysics(foePhysCollisionShapeLoader &collisionShapeLoader,
                    foePhysCollisionShapePool &collisionShapePool,
                    foeDataPool<foeEntityID, foePhysRigidBody> &rigidBodyPool,
                    foeDataPool<foeEntityID, std::unique_ptr<Position3D>> &positionPool,
                    float timePassed) {
    // Any previously attempted items that were waiting for external resources to be loaded
    auto awaitingResources = std::move(mAwaitingResources);
    for (auto const &it : awaitingResources) {
        addObject(collisionShapeLoader, collisionShapePool, rigidBodyPool, positionPool, it,
                  nullptr, nullptr, nullptr);
    }

    // Removals
    { // RigidBodies
        auto *pId = rigidBodyPool.rmbegin();
        auto *const pIdEnd = rigidBodyPool.rmend();
        auto *pData = rigidBodyPool.rmbegin<1>();

        for (; pId != pIdEnd; ++pId, ++pData) {
            removeObject(rigidBodyPool, *pId, pData);
        }
    }

    { // Position3D
        auto *pId = rigidBodyPool.rmbegin();
        auto *const pIdEnd = rigidBodyPool.rmend();

        for (; pId != pIdEnd; ++pId) {
            removeObject(rigidBodyPool, *pId, nullptr);
        }
    }

    // Insertions
    { // RigidBodies
        auto offsetIt = rigidBodyPool.inbegin();
        auto offsetEndIt = rigidBodyPool.inend();
        auto *const pId = rigidBodyPool.begin();
        auto *const pData = rigidBodyPool.begin<1>();

        for (; offsetIt != offsetEndIt; ++offsetIt) {
            addObject(collisionShapeLoader, collisionShapePool, rigidBodyPool, positionPool,
                      pId[*offsetIt], &pData[*offsetIt], nullptr, nullptr);
        }
    }

    { // Position3D
        auto offsetIt = positionPool.inbegin();
        auto offsetEndIt = positionPool.inend();
        auto *const pId = positionPool.begin();
        auto *const pData = positionPool.begin<1>();

        for (; offsetIt != offsetEndIt; ++offsetIt) {
            addObject(collisionShapeLoader, collisionShapePool, rigidBodyPool, positionPool,
                      pId[*offsetIt], nullptr, pData[*offsetIt].get(), nullptr);
        }
    }

    physWorld->stepSimulation(timePassed);

    { // Copy position data to Position3D objects
        auto *pId = rigidBodyPool.begin();
        auto *const pEndId = rigidBodyPool.end();
        auto *pData = rigidBodyPool.begin<1>();

        for (; pId != pEndId; ++pId, ++pData) {
            if (pData->rigidBody == nullptr)
                continue;

            auto posOffset = positionPool.find(*pId);
            assert(posOffset != positionPool.size());

            Position3D *pPosition = positionPool.begin<1>()[posOffset].get();

            glm::mat4 transform = btToGlmMat4(pData->rigidBody->getWorldTransform());
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform, scale, rotation, translation, skew, perspective);
            pPosition->position = translation;
            pPosition->orientation = rotation;
        }
    }

    if (rigidBodyPool.size() > 0 && rigidBodyPool.begin<1>()->rigidBody != nullptr) {
        glm::vec3 pos =
            btToGlmVec3(rigidBodyPool.begin<1>()->rigidBody->getWorldTransform().getOrigin());
        // FOE_LOG(General, Info, "Y Pos: {}", pos.y);
    }
}