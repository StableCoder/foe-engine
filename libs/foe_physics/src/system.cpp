// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/system.hpp>

#include <foe/ecs/id_to_string.hpp>
#include <foe/physics/component/rigid_body.h>
#include <foe/physics/component/rigid_body_pool.h>
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
                                          foeRigidBodyPool rigidBodyPool,
                                          foePosition3dPool *pPosition3dPool) {
    if (resourcePool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES);
    if (pCollisionShapeLoader == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_LOADER);
    if (rigidBodyPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS);
    if (pPosition3dPool == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS);

    mResourcePool = resourcePool;
    mpCollisionShapeLoader = pCollisionShapeLoader;

    mRigidBodyPool = rigidBodyPool;
    mpPosition3dPool = pPosition3dPool;

    mAwaitingLoadingResources.clear();

    // As we're initializing, we need to go through and process any already available data, so that
    // per-tick processing can be streamlined to operate more quickly using deltas
    { // Iterate through active rigid bodies, the primary for this system
        foeEntityID const *pID = foeEcsComponentPoolIdPtr(rigidBodyPool);
        foeEntityID const *const pEndID = pID + foeEcsComponentPoolSize(rigidBodyPool);
        foeRigidBody *pData = (foeRigidBody *)foeEcsComponentPoolDataPtr(rigidBodyPool);

        for (; pID != pEndID; ++pID, ++pData) {
            addObject(*pID, pData, nullptr, nullptr);
        }
    }

    return to_foeResult(FOE_PHYSICS_SUCCESS);
}

void foePhysicsSystem::deinitialize() {
    // On the way out, go through ALL and remove from any worlds
    mAwaitingLoadingResources.clear();

    if (mRigidBodyPool) {
        { // Iterate through 'active' rigid bodies, the primary for this system
            foeEntityID const *pID = foeEcsComponentPoolIdPtr(mRigidBodyPool);
            foeEntityID const *const pEndID = pID + foeEcsComponentPoolSize(mRigidBodyPool);
            foeRigidBody *pData = (foeRigidBody *)foeEcsComponentPoolDataPtr(mRigidBodyPool);

            for (; pID != pEndID; ++pID, ++pData) {
                removeObject(*pID, pData);
            }
        }

        { // Iterate through 'removed' rigid bodies, the primary for this system
            foeEntityID const *pID = foeEcsComponentPoolRemovedIdPtr(mRigidBodyPool);
            foeEntityID const *const pEndID = pID + foeEcsComponentPoolRemoved(mRigidBodyPool);
            foeRigidBody *pData = (foeRigidBody *)foeEcsComponentPoolRemovedDataPtr(mRigidBodyPool);

            for (; pID != pEndID; ++pID, ++pData) {
                removeObject(*pID, pData);
            }
        }
    }

    mpPosition3dPool = nullptr;
    mRigidBodyPool = FOE_NULL_HANDLE;

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
        foeEntityID const *pID = foeEcsComponentPoolRemovedIdPtr(mRigidBodyPool);
        foeEntityID const *const pEndID = pID + foeEcsComponentPoolRemoved(mRigidBodyPool);
        foeRigidBody *pData = (foeRigidBody *)foeEcsComponentPoolRemovedDataPtr(mRigidBodyPool);

        for (; pID != pEndID; ++pID, ++pData) {
            removeObject(*pID, pData);
        }
    }

    { // foePosition3d
        auto *pId = mpPosition3dPool->rmbegin();
        auto *const pIdEnd = mpPosition3dPool->rmend();

        for (; pId != pIdEnd; ++pId) {
            removeObject(*pId, nullptr);
        }
    }

    // Insertions
    { // RigidBodies
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(mRigidBodyPool);
        foeRigidBody *const pStartData =
            (foeRigidBody *const)foeEcsComponentPoolDataPtr(mRigidBodyPool);

        size_t const *pOffset = foeEcsComponentPoolInsertedOffsetPtr(mRigidBodyPool);
        size_t const *const pEndOffset = pOffset + foeEcsComponentPoolInserted(mRigidBodyPool);

        for (; pOffset != pEndOffset; ++pOffset) {
            addObject(pStartID[*pOffset], pStartData + *pOffset, nullptr, nullptr);
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
        foeEntityID const *pID = foeEcsComponentPoolIdPtr(mRigidBodyPool);
        foeEntityID const *const pEndID = pID + foeEcsComponentPoolSize(mRigidBodyPool);
        foeRigidBody *pData = (foeRigidBody *)foeEcsComponentPoolDataPtr(mRigidBodyPool);

        for (; pID != pEndID; ++pID, ++pData) {
            if (pData->pRigidBody == nullptr)
                continue;

            auto posOffset = mpPosition3dPool->find(*pID);
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
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(mRigidBodyPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(mRigidBodyPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            pRigidBody = (foeRigidBody *)foeEcsComponentPoolDataPtr(mRigidBodyPool) + offset;
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
            foeResourceLoadData(collisionShape);
            mAwaitingLoadingResources.emplace_back(entity);
        }

        return;
    } else if (foeResourceGetType(collisionShape) != FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE) {
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
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
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(mRigidBodyPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(mRigidBodyPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            pRigidBody = (foeRigidBody *)foeEcsComponentPoolDataPtr(mRigidBodyPool) + offset;
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