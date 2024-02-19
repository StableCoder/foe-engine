// Copyright (C) 2021-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/system.h>

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/physics/component/rigid_body.h>
#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/type_defs.h>
#include <foe/position/component/3d.hpp>
#include <foe/position/component/3d_pool.h>
#include <foe/resource/resource.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "bt_glm_conversion.hpp"
#include "log.hpp"
#include "result.h"

#include <algorithm>
#include <vector>

namespace {

struct ActiveWorldObject {
    foeEntityID entity;
    foeResource collisionShape;
    float mass;
    btRigidBody *pRigidBody;
};

struct PhysicsSystem {
    // External Data
    foeResourcePool resourcePool;
    foeRigidBodyPool rigidBodyPool;
    foePosition3dPool positionPool;

    // Entity Lists
    foeEcsEntityList positionModifiedEntityList;

    // Physics World Instance Items
    btBroadphaseInterface *pBroadphase;
    btDefaultCollisionConfiguration *pCollisionConfig;
    btCollisionDispatcher *pCollisionDispatcher;
    btSequentialImpulseConstraintSolver *pSolver;
    btDiscreteDynamicsWorld *pWorld;

    // Currently active physics objects
    std::vector<ActiveWorldObject> activeWorldObjects;

    // Lists entities that need some resources to load before being added to a world
    std::vector<foeEntityID> awaitingLoadingResources;
};

FOE_DEFINE_HANDLE_CASTS(physics_system, PhysicsSystem, foePhysicsSystem)

[[nodiscard]] foeResultSet addWorldObject(PhysicsSystem *pPhysicsSystem,
                                          foeEntityID entity,
                                          foeRigidBody *pRigidBody,
                                          foePosition3d *pPosition,
                                          foeResource collisionShape) {
    auto searchIt = std::lower_bound(
        pPhysicsSystem->activeWorldObjects.begin(), pPhysicsSystem->activeWorldObjects.end(),
        entity,
        [](ActiveWorldObject const &obj, foeEntityID const entity) { return obj.entity < entity; });

    // If already added, don't re-add it
    if (searchIt != pPhysicsSystem->activeWorldObjects.end() && searchIt->entity == entity)
        return to_foeResult(FOE_PHYSICS_SUCCESS);

    // RigidBody
    if (pRigidBody == nullptr) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(pPhysicsSystem->rigidBodyPool);
        foeEntityID const *const pEndID =
            pStartID + foeEcsComponentPoolSize(pPhysicsSystem->rigidBodyPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            pRigidBody =
                (foeRigidBody *)foeEcsComponentPoolDataPtr(pPhysicsSystem->rigidBodyPool) + offset;
        } else {
            return to_foeResult(FOE_PHYSICS_SUCCESS);
        }
    }

    // foePosition3d
    if (pPosition == nullptr) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(pPhysicsSystem->positionPool);
        foeEntityID const *const pEndID =
            pStartID + foeEcsComponentPoolSize(pPhysicsSystem->positionPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            pPosition =
                *((foePosition3d **)foeEcsComponentPoolDataPtr(pPhysicsSystem->positionPool) +
                  offset);
        } else {
            return to_foeResult(FOE_PHYSICS_SUCCESS);
        }
    }

    // CollisionShape
    while (collisionShape == FOE_NULL_HANDLE) {
        collisionShape =
            foeResourcePoolFind(pPhysicsSystem->resourcePool, pRigidBody->collisionShape);

        if (collisionShape == FOE_NULL_HANDLE)
            collisionShape =
                foeResourcePoolAdd(pPhysicsSystem->resourcePool, pRigidBody->collisionShape);
    }

CHECK_COLLISION_SHAPE_LOADED:
    if (auto resourceState = foeResourceGetState(collisionShape);
        (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) == 0) {
        if ((resourceState & FOE_RESOURCE_STATE_LOADING_BIT) == 0) {
            foeResourceLoadData(collisionShape);
        }
        pPhysicsSystem->awaitingLoadingResources.emplace_back(entity);

        // No longer holding on to this resource reference
        foeResourceDecrementRefCount(collisionShape);

        return to_foeResult(FOE_PHYSICS_SUCCESS);
    }
    if (foeResourceGetType(collisionShape) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED) {
        // Resource has been replaced, advance to it instead
        foeResource replacementResource = foeResourceGetReplacement(collisionShape);

        foeResourceDecrementRefCount(collisionShape);

        collisionShape = replacementResource;

        goto CHECK_COLLISION_SHAPE_LOADED;
    }

    // Get CollisionShape data, if it exists in the acquired resource
    auto const *pCollisionShape = (foeCollisionShape const *)foeResourceGetTypeData(
        collisionShape, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE);

    if (pCollisionShape == nullptr) {
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                "foePhysicsSystem - Failed to load {} rigid body because the given "
                "resource {} is not a collision shape resource.",
                foeIdToString(entity), foeIdToString(pRigidBody->collisionShape))

        // No longer holding on to this resource reference
        foeResourceDecrementRefCount(collisionShape);

        return to_foeResult(FOE_PHYSICS_SUCCESS);
    }

    // We have everything we need now
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI{pRigidBody->mass, nullptr,
                                                         pCollisionShape->collisionShape.get()};
    rigidBodyCI.m_startWorldTransform =
        glmToBtTransform(pPosition->position, pPosition->orientation);

    ActiveWorldObject newObject = {
        .entity = entity,
        .collisionShape = collisionShape,
        .mass = pRigidBody->mass,
        .pRigidBody = (btRigidBody *)malloc(sizeof(btRigidBody)),
    };
    if (newObject.pRigidBody == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
    new (newObject.pRigidBody) btRigidBody{rigidBodyCI};

    foeResourceIncrementUseCount(collisionShape);

    pPhysicsSystem->pWorld->addRigidBody(newObject.pRigidBody);

    pPhysicsSystem->activeWorldObjects.insert(searchIt, newObject);

    return to_foeResult(FOE_PHYSICS_SUCCESS);
}

void removeWorldObject(PhysicsSystem *pPhysicsSystem, foeEntityID entity) {
    auto searchIt = std::lower_bound(
        pPhysicsSystem->activeWorldObjects.begin(), pPhysicsSystem->activeWorldObjects.end(),
        entity,
        [](ActiveWorldObject const &obj, foeEntityID const entity) { return obj.entity < entity; });

    if (searchIt == pPhysicsSystem->activeWorldObjects.end() || searchIt->entity != entity)
        return;

    pPhysicsSystem->pWorld->removeRigidBody(searchIt->pRigidBody);

    foeResourceDecrementRefCount(searchIt->collisionShape);
    foeResourceDecrementUseCount(searchIt->collisionShape);

    searchIt->pRigidBody->~btRigidBody();
    free(searchIt->pRigidBody);

    pPhysicsSystem->activeWorldObjects.erase(searchIt);
}

} // namespace

extern "C" foeResultSet foePhysicsCreateSystem(foePhysicsSystem *pPhysicsSystem) {
    PhysicsSystem *pNewPhysicsSystem = new (std::nothrow) PhysicsSystem{};
    if (pNewPhysicsSystem == nullptr)
        return to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);

    *pPhysicsSystem = physics_system_to_handle(pNewPhysicsSystem);
    return to_foeResult(FOE_PHYSICS_SUCCESS);
}

extern "C" void foePhysicsDestroySystem(foePhysicsSystem physicsSystem) {
    PhysicsSystem *pPhysicsSystem = physics_system_from_handle(physicsSystem);

    delete pPhysicsSystem;
}

extern "C" foeResultSet foePhysicsInitializeSystem(foePhysicsSystem physicsSystem,
                                                   foeResourcePool resourcePool,
                                                   foeRigidBodyPool rigidBodyPool,
                                                   foePosition3dPool positionPool) {
    PhysicsSystem *pPhysicsSystem = physics_system_from_handle(physicsSystem);

    if (resourcePool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES);
    if (rigidBodyPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS);
    if (positionPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS);

    // Set external data
    pPhysicsSystem->resourcePool = resourcePool;
    pPhysicsSystem->rigidBodyPool = rigidBodyPool;
    pPhysicsSystem->positionPool = positionPool;

    // Setup internal data
    foeResultSet result = to_foeResult(FOE_PHYSICS_SUCCESS);

    // Entity List
    result = foeEcsCreateEntityList(&pPhysicsSystem->positionModifiedEntityList);
    if (result.value != FOE_SUCCESS)
        goto INITIALIZATION_FAILED;
    result =
        foeEcsComponentPoolAddEntityList(positionPool, pPhysicsSystem->positionModifiedEntityList);
    if (result.value != FOE_SUCCESS)
        goto INITIALIZATION_FAILED;

    // Physics World
    pPhysicsSystem->pBroadphase = new (std::nothrow) btDbvtBroadphase;
    if (pPhysicsSystem->pBroadphase == nullptr) {
        result = to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
        goto INITIALIZATION_FAILED;
    }

    pPhysicsSystem->pCollisionConfig = new (std::nothrow) btDefaultCollisionConfiguration;
    if (pPhysicsSystem->pCollisionConfig == nullptr) {
        result = to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
        goto INITIALIZATION_FAILED;
    }

    pPhysicsSystem->pCollisionDispatcher =
        new (std::nothrow) btCollisionDispatcher{pPhysicsSystem->pCollisionConfig};
    if (pPhysicsSystem->pCollisionDispatcher == nullptr) {
        result = to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
        goto INITIALIZATION_FAILED;
    }

    pPhysicsSystem->pSolver =
        (btSequentialImpulseConstraintSolver *)malloc(sizeof(btSequentialImpulseConstraintSolver));
    if (pPhysicsSystem->pSolver == nullptr) {
        result = to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
        goto INITIALIZATION_FAILED;
    }
    new (pPhysicsSystem->pSolver) btSequentialImpulseConstraintSolver;

    pPhysicsSystem->pWorld = (btDiscreteDynamicsWorld *)malloc(sizeof(btDiscreteDynamicsWorld));
    if (pPhysicsSystem->pWorld == nullptr) {
        result = to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
        goto INITIALIZATION_FAILED;
    }
    new (pPhysicsSystem->pWorld)
        btDiscreteDynamicsWorld{pPhysicsSystem->pCollisionDispatcher, pPhysicsSystem->pBroadphase,
                                pPhysicsSystem->pSolver, pPhysicsSystem->pCollisionConfig};

    pPhysicsSystem->pWorld->setGravity(btVector3{0, -9.8, 0});

    // As we're initializing, we need to go through and process any already
    // available data, so that per-tick processing can be streamlined to operate
    // more quickly using deltas
    { // Iterate through active rigid bodies, the primary for this system
        foeEntityID const *pID = foeEcsComponentPoolIdPtr(rigidBodyPool);
        foeEntityID const *const pEndID = pID + foeEcsComponentPoolSize(rigidBodyPool);
        foeRigidBody *pData = (foeRigidBody *)foeEcsComponentPoolDataPtr(rigidBodyPool);

        for (; pID != pEndID; ++pID, ++pData) {
            result = addWorldObject(pPhysicsSystem, *pID, pData, nullptr, nullptr);
            if (result.value != FOE_SUCCESS)
                goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foePhysicsDeinitializeSystem(physicsSystem);

    return to_foeResult(FOE_PHYSICS_SUCCESS);
}

extern "C" void foePhysicsDeinitializeSystem(foePhysicsSystem physicsSystem) {
    PhysicsSystem *pPhysicsSystem = physics_system_from_handle(physicsSystem);

    // Go through and clear any items awaiting resources being loaded
    pPhysicsSystem->awaitingLoadingResources.clear();

    // Remove objects from the world
    if (pPhysicsSystem->rigidBodyPool) {
        { // Iterate through 'active' rigid bodies, the primary for this system
            foeEntityID const *pID = foeEcsComponentPoolIdPtr(pPhysicsSystem->rigidBodyPool);
            foeEntityID const *const pEndID =
                pID + foeEcsComponentPoolSize(pPhysicsSystem->rigidBodyPool);

            for (; pID != pEndID; ++pID) {
                removeWorldObject(pPhysicsSystem, *pID);
            }
        }

        { // Iterate through 'removed' rigid bodies, the primary for this system
            foeEntityID const *pID = foeEcsComponentPoolRemovedIdPtr(pPhysicsSystem->rigidBodyPool);
            foeEntityID const *const pEndID =
                pID + foeEcsComponentPoolRemoved(pPhysicsSystem->rigidBodyPool);

            for (; pID != pEndID; ++pID) {
                removeWorldObject(pPhysicsSystem, *pID);
            }
        }
    }

    // Physics World
    if (pPhysicsSystem->pWorld != nullptr) {
        pPhysicsSystem->pWorld->~btDiscreteDynamicsWorld();
        free(pPhysicsSystem->pWorld);
    }
    pPhysicsSystem->pWorld = nullptr;

    if (pPhysicsSystem->pSolver != nullptr) {
        pPhysicsSystem->pSolver->~btSequentialImpulseConstraintSolver();
        free(pPhysicsSystem->pSolver);
    }
    pPhysicsSystem->pSolver = nullptr;

    if (pPhysicsSystem->pCollisionDispatcher != nullptr)
        delete pPhysicsSystem->pCollisionDispatcher;
    pPhysicsSystem->pCollisionDispatcher = nullptr;

    if (pPhysicsSystem->pCollisionConfig != nullptr)
        delete pPhysicsSystem->pCollisionConfig;
    pPhysicsSystem->pCollisionConfig = nullptr;

    if (pPhysicsSystem->pBroadphase != nullptr)
        delete pPhysicsSystem->pBroadphase;
    pPhysicsSystem->pBroadphase = nullptr;

    // Entity List
    if (pPhysicsSystem->positionModifiedEntityList != FOE_NULL_HANDLE) {
        foeEcsComponentPoolRemoveEntityList(pPhysicsSystem->positionPool,
                                            pPhysicsSystem->positionModifiedEntityList);
        foeEcsDestroyEntityList(pPhysicsSystem->positionModifiedEntityList);
        pPhysicsSystem->positionModifiedEntityList = FOE_NULL_HANDLE;
    }

    // Clear external data
    pPhysicsSystem->positionPool = FOE_NULL_HANDLE;
    pPhysicsSystem->rigidBodyPool = FOE_NULL_HANDLE;
    pPhysicsSystem->resourcePool = FOE_NULL_HANDLE;
}

extern "C" foeResultSet foePhysicsProcessSystem(foePhysicsSystem physicsSystem, float timeElapsed) {
    PhysicsSystem *pPhysicsSystem = physics_system_from_handle(physicsSystem);
    foeResultSet result = to_foeResult(FOE_PHYSICS_SUCCESS);

    // Any previously attempted items that were waiting for external resources to be
    // loaded
    auto awaitingResources = std::move(pPhysicsSystem->awaitingLoadingResources);
    for (auto const &it : awaitingResources) {
        result = addWorldObject(pPhysicsSystem, it, nullptr, nullptr, nullptr);
        if (result.value != FOE_SUCCESS)
            return result;
    }

    { // Removed RigidBody
        foeEntityID const *pID = foeEcsComponentPoolRemovedIdPtr(pPhysicsSystem->rigidBodyPool);
        foeEntityID const *const pEndID =
            pID + foeEcsComponentPoolRemoved(pPhysicsSystem->rigidBodyPool);

        for (; pID != pEndID; ++pID) {
            removeWorldObject(pPhysicsSystem, *pID);
        }
    }

    { // Removed Position
        foeEntityID const *pID = foeEcsComponentPoolRemovedIdPtr(pPhysicsSystem->positionPool);
        foeEntityID const *const pEndID =
            pID + foeEcsComponentPoolRemoved(pPhysicsSystem->positionPool);

        for (; pID != pEndID; ++pID) {
            removeWorldObject(pPhysicsSystem, *pID);
        }
    }

    { // Modified RigidBody
        size_t const entityListCount =
            foeEcsComponentPoolEntityListSize(pPhysicsSystem->rigidBodyPool);
        foeEcsEntityList const *pLists =
            foeEcsComponentPoolEntityLists(pPhysicsSystem->rigidBodyPool);

        for (size_t i = 0; i < entityListCount; ++i) {
            foeEcsEntityList entityList = pLists[i];

            foeEntityID const *pModifiedID = foeEcsEntityListPtr(entityList);
            foeEntityID const *const pEndModifiedID =
                pModifiedID + foeEcsEntityListSize(entityList);

            for (; pModifiedID != pEndModifiedID; ++pModifiedID) {
                removeWorldObject(pPhysicsSystem, *pModifiedID);
                result =
                    addWorldObject(pPhysicsSystem, *pModifiedID, nullptr, nullptr, FOE_NULL_HANDLE);
                if (result.value != FOE_SUCCESS)
                    return result;
            }
        }
    }

    { // Modified Position
        size_t const entityListCount =
            foeEcsComponentPoolEntityListSize(pPhysicsSystem->rigidBodyPool);
        foeEcsEntityList const *pLists =
            foeEcsComponentPoolEntityLists(pPhysicsSystem->rigidBodyPool);

        for (size_t i = 0; i < entityListCount; ++i) {
            foeEcsEntityList entityList = pLists[i];

            // Make sure not to process items modified by this same system
            if (entityList == pPhysicsSystem->positionModifiedEntityList)
                continue;

            foeEntityID const *pModifiedID = foeEcsEntityListPtr(entityList);
            foeEntityID const *const pEndModifiedID =
                pModifiedID + foeEcsEntityListSize(entityList);

            for (; pModifiedID != pEndModifiedID; ++pModifiedID) {
                removeWorldObject(pPhysicsSystem, *pModifiedID);
                result =
                    addWorldObject(pPhysicsSystem, *pModifiedID, nullptr, nullptr, FOE_NULL_HANDLE);
                if (result.value != FOE_SUCCESS)
                    return result;
            }
        }
    }

    { // Inserted RigidBody
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(pPhysicsSystem->rigidBodyPool);
        foeRigidBody *const pStartData =
            (foeRigidBody *const)foeEcsComponentPoolDataPtr(pPhysicsSystem->rigidBodyPool);

        size_t const *pOffset = foeEcsComponentPoolInsertedOffsetPtr(pPhysicsSystem->rigidBodyPool);
        size_t const *const pEndOffset =
            pOffset + foeEcsComponentPoolInserted(pPhysicsSystem->rigidBodyPool);

        for (; pOffset != pEndOffset; ++pOffset) {
            result = addWorldObject(pPhysicsSystem, pStartID[*pOffset], pStartData + *pOffset,
                                    nullptr, nullptr);
            if (result.value != FOE_SUCCESS)
                return result;
        }
    }

    { // Inserted Position
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(pPhysicsSystem->positionPool);
        foePosition3d **const pStartData =
            (foePosition3d **)foeEcsComponentPoolDataPtr(pPhysicsSystem->positionPool);

        size_t const *pOffset = foeEcsComponentPoolInsertedOffsetPtr(pPhysicsSystem->positionPool);
        size_t const *const pEndOffset =
            pOffset + foeEcsComponentPoolInserted(pPhysicsSystem->positionPool);

        for (; pOffset != pEndOffset; ++pOffset) {
            result = addWorldObject(pPhysicsSystem, pStartID[*pOffset], nullptr,
                                    pStartData[*pOffset], nullptr);
            if (result.value != FOE_SUCCESS)
                return result;
        }
    }

    // Actual step the world forward by the amount of elapsed time
    pPhysicsSystem->pWorld->stepSimulation(timeElapsed);

    { // Copy position data to foePosition3d objects
        std::vector<foeEntityID> modifiedIDs;
        modifiedIDs.reserve(pPhysicsSystem->activeWorldObjects.size());

        foeEntityID const *pPositionID = foeEcsComponentPoolIdPtr(pPhysicsSystem->positionPool);

        foeEntityID const *const pStartPositionID = pPositionID;
        foeEntityID const *const pEndPositionID =
            pStartPositionID + foeEcsComponentPoolSize(pPhysicsSystem->positionPool);
        foePosition3d **const pStartPositionData =
            (foePosition3d **)foeEcsComponentPoolDataPtr(pPhysicsSystem->positionPool);

        for (auto const &activeWorldObject : pPhysicsSystem->activeWorldObjects) {
            pPositionID = std::lower_bound(pPositionID, pEndPositionID, activeWorldObject.entity);
            if (pPositionID == pEndPositionID)
                break;
            if (*pPositionID != activeWorldObject.entity)
                continue;

            foePosition3d *pPosition = pStartPositionData[pPositionID - pStartPositionID];

            glm::mat4 transform = btToGlmMat4(activeWorldObject.pRigidBody->getWorldTransform());
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform, scale, rotation, translation, skew, perspective);
            pPosition->position = translation;
            pPosition->orientation = rotation;

            modifiedIDs.emplace_back(*pPositionID);
        }

        // Tell other systems about what positions were modified here
        uint32_t listCount = modifiedIDs.size();
        foeEntityID *pEntityList = modifiedIDs.data();
        foeEcsResetEntityList(pPhysicsSystem->positionModifiedEntityList, 1, &listCount,
                              &pEntityList);
    }

    return to_foeResult(FOE_PHYSICS_SUCCESS);
}