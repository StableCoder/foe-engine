// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/system.hpp>

#include <foe/ecs/id_to_string.hpp>
#include <foe/physics/component/rigid_body.hpp>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "bt_glm_conversion.hpp"
#include "log.hpp"
#include "result.h"

foePhysicsSystem::foePhysicsSystem() :
    mpBroadphase{new btDbvtBroadphase{}},
    mpCollisionConfig{new btDefaultCollisionConfiguration{}},
    mpCollisionDispatcher{new btCollisionDispatcher{mpCollisionConfig.get()}},
    mpSolver{new btSequentialImpulseConstraintSolver{}},
    mpWorld{new btDiscreteDynamicsWorld{mpCollisionDispatcher.get(), mpBroadphase.get(),
                                        mpSolver.get(), mpCollisionConfig.get()}} {
    mpWorld->setGravity(btVector3{0, -9.8, 0});
}

foePhysicsSystem::~foePhysicsSystem() {}

foeResultSet foePhysicsSystem::initialize(foeResourcePool resourcePool,
                                          foeCollisionShapeLoader *pCollisionShapeLoader,
                                          foeRigidBodyPool *pRigidBodyPool,
                                          foePosition3dPool *pPosition3dPool) {
    if (resourcePool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES);
    if (pCollisionShapeLoader == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_LOADER);
    if (pRigidBodyPool == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS);
    if (pPosition3dPool == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS);

    mResourcePool = resourcePool;
    mpCollisionShapeLoader = pCollisionShapeLoader;

    mpRigidBodyPool = pRigidBodyPool;
    mpPosition3dPool = pPosition3dPool;

    mAwaitingLoadingResources.clear();

    // As we're initializing, we need to go through and process any already available data, so that
    // per-tick processing can be streamlined to operate more quickly using deltas
    { // Iterate through active rigid bodies, the primary for this system
        auto *pId = mpRigidBodyPool->begin();
        auto const *const pEndId = mpRigidBodyPool->end();
        auto *pData = mpRigidBodyPool->begin<1>();

        for (; pId != pEndId; ++pId, ++pData) {
            addObject(*pId, pData, nullptr, nullptr);
        }
    }

    return to_foeResult(FOE_PHYSICS_SUCCESS);
}

void foePhysicsSystem::deinitialize() {
    // On the way out, go through ALL and remove from any worlds
    mAwaitingLoadingResources.clear();

    if (mpRigidBodyPool) {
        { // Iterate through 'active' rigid bodies, the primary for this system
            auto *pId = mpRigidBodyPool->begin();
            auto const *const pEndId = mpRigidBodyPool->end();
            auto *pData = mpRigidBodyPool->begin<1>();

            for (; pId != pEndId; ++pId, ++pData) {
                removeObject(*pId, pData);
            }
        }

        { // Iterate through 'removed' rigid bodies, the primary for this system
            auto *pId = mpRigidBodyPool->rmbegin();
            auto const *const pEndId = mpRigidBodyPool->rmend();
            auto *pData = mpRigidBodyPool->rmbegin<1>();

            for (; pId != pEndId; ++pId, ++pData) {
                removeObject(*pId, pData);
            }
        }
    }

    mpPosition3dPool = nullptr;
    mpRigidBodyPool = nullptr;

    mpCollisionShapeLoader = nullptr;
    mResourcePool = FOE_NULL_HANDLE;
}

bool foePhysicsSystem::initialized() const noexcept { return mpCollisionShapeLoader != nullptr; }

void foePhysicsSystem::process(float timePassed) {
    // Any previously attempted items that were waiting for external resources to be loaded
    auto awaitingResources = std::move(mAwaitingLoadingResources);
    for (auto const &it : awaitingResources) {
        addObject(it, nullptr, nullptr, nullptr);
    }

    // Removals
    { // RigidBodies
        auto *pId = mpRigidBodyPool->rmbegin();
        auto *const pIdEnd = mpRigidBodyPool->rmend();
        auto *pData = mpRigidBodyPool->rmbegin<1>();

        for (; pId != pIdEnd; ++pId, ++pData) {
            removeObject(*pId, pData);
        }
    }

    { // foePosition3d
        auto *pId = mpRigidBodyPool->rmbegin();
        auto *const pIdEnd = mpRigidBodyPool->rmend();

        for (; pId != pIdEnd; ++pId) {
            removeObject(*pId, nullptr);
        }
    }

    // Insertions
    { // RigidBodies
        auto offsetIt = mpRigidBodyPool->inbegin();
        auto offsetEndIt = mpRigidBodyPool->inend();
        auto *const pId = mpRigidBodyPool->begin();
        auto *const pData = mpRigidBodyPool->begin<1>();

        for (; offsetIt != offsetEndIt; ++offsetIt) {
            addObject(pId[*offsetIt], &pData[*offsetIt], nullptr, nullptr);
        }
    }

    { // foePosition3d
        auto offsetIt = mpPosition3dPool->inbegin();
        auto offsetEndIt = mpPosition3dPool->inend();
        auto *const pId = mpPosition3dPool->begin();
        auto *const pData = mpPosition3dPool->begin<1>();

        for (; offsetIt != offsetEndIt; ++offsetIt) {
            addObject(pId[*offsetIt], nullptr, pData[*offsetIt].get(), nullptr);
        }
    }

    mpWorld->stepSimulation(timePassed);

    { // Copy position data to foePosition3d objects
        auto *pId = mpRigidBodyPool->begin();
        auto *const pEndId = mpRigidBodyPool->end();
        auto *pData = mpRigidBodyPool->begin<1>();

        for (; pId != pEndId; ++pId, ++pData) {
            if (pData->pRigidBody == nullptr)
                continue;

            auto posOffset = mpPosition3dPool->find(*pId);
            assert(posOffset != mpPosition3dPool->size());

            foePosition3d *pPosition = mpPosition3dPool->begin<1>()[posOffset].get();

            glm::mat4 transform = btToGlmMat4(pData->pRigidBody->getWorldTransform());
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
}

void foePhysicsSystem::addObject(foeEntityID entity,
                                 foeRigidBody *pRigidBody,
                                 foePosition3d *pPosition,
                                 foeResource collisionShape) {
    // RigidBody
    if (pRigidBody == nullptr) {
        size_t dataOffset = mpRigidBodyPool->find(entity);
        if (dataOffset != mpRigidBodyPool->size()) {
            pRigidBody = mpRigidBodyPool->begin<1>() + dataOffset;
        } else {
            return;
        }
    }

    // If already active
    if (pRigidBody->pRigidBody)
        return;

    // foePosition3d
    if (pPosition == nullptr) {
        size_t dataOffset = mpPosition3dPool->find(entity);
        if (dataOffset != mpPosition3dPool->size()) {
            pPosition = (mpPosition3dPool->begin<1>() + dataOffset)->get();
        } else {
            return;
        }
    }

    // CollisionShape
    while (collisionShape == FOE_NULL_HANDLE) {
        collisionShape = foeResourcePoolFind(mResourcePool, pRigidBody->collisionShape);

        if (collisionShape == FOE_NULL_HANDLE)
            collisionShape = foeResourcePoolAdd(mResourcePool, pRigidBody->collisionShape,
                                                FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE,
                                                sizeof(foeCollisionShape));
    }

    if (auto loadState = foeResourceGetState(collisionShape);
        loadState != FOE_RESOURCE_LOAD_STATE_LOADED) {
        if (loadState == FOE_RESOURCE_LOAD_STATE_UNLOADED &&
            !foeResourceGetIsLoading(collisionShape)) {
            foeResourceLoad(collisionShape, false);
            mAwaitingLoadingResources.emplace_back(entity);
        }

        return;
    } else if (foeResourceGetType(collisionShape) != FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE) {
        FOE_LOG(foePhysics, Error,
                "foePhysicsSystem - Failed to load {} rigid body because the given "
                "collision shape {} is not a collision shape resource.",
                foeIdToString(entity), foeIdToString(pRigidBody->collisionShape))
        return;
    }

    // Get Resource Data Pointers
    auto const *pCollisionShape = (foeCollisionShape const *)foeResourceGetData(collisionShape);

    // We have everything we need now
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI{pRigidBody->mass, nullptr,
                                                         pCollisionShape->collisionShape.get()};
    rigidBodyCI.m_startWorldTransform =
        glmToBtTransform(pPosition->position, pPosition->orientation);

    pRigidBody->pRigidBody = new btRigidBody{rigidBodyCI};

    mpWorld->addRigidBody(pRigidBody->pRigidBody);
}

void foePhysicsSystem::removeObject(foeEntityID entity, foeRigidBody *pRigidBody) {
    // RigidBody
    if (pRigidBody == nullptr) {
        size_t dataOffset = mpRigidBodyPool->find(entity);
        if (dataOffset != mpRigidBodyPool->size()) {
            pRigidBody = mpRigidBodyPool->begin<1>() + dataOffset;
        } else {
            return;
        }
    }

    // If inactive
    if (pRigidBody->pRigidBody == nullptr)
        return;

    mpWorld->removeRigidBody(pRigidBody->pRigidBody);

    delete pRigidBody->pRigidBody;
    pRigidBody->pRigidBody = nullptr;
}