// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "animated_bone_system.h"

#include <foe/resource/pool.h>

#include "../result.h"
#include "animated_bone_state.hpp"
#include "armature.hpp"
#include "armature_state.h"
#include "foe/handle.h"
#include "type_defs.h"

#include <algorithm>
#include <vector>

namespace {

struct AwaitingData {
    foeEntityID entity;
    foeResource armature;
};

struct AnimatedBoneSystem {
    foeResourcePool mResourcePool;
    foeArmatureStatePool mArmatureStatePool;
    foeAnimatedBoneStatePool mAnimatedBoneStatePool;

    std::vector<AwaitingData> mAwaitingLoading;
};

FOE_DEFINE_HANDLE_CASTS(animated_bone_system, AnimatedBoneSystem, foeAnimatedBoneSystem)

void originalArmatureNode(foeArmatureNode const *pNode,
                          glm::mat4 const &parentTransform,
                          glm::mat4 *pArmatureTransform) {

    *pArmatureTransform = parentTransform * pNode->transformMatrix;

    for (size_t child = 0; child < pNode->numChildren; ++child) {
        size_t const offset = pNode->childrenOffset + child;

        originalArmatureNode(pNode + offset, *pArmatureTransform, pArmatureTransform + offset);
    }
}

void animateArmatureNode(foeArmatureNode const *pNode,
                         std::vector<foeNodeAnimationChannel> const &animationChannels,
                         double const animationTick,
                         glm::mat4 const &parentTransform,
                         glm::mat4 *pArmatureTransform) {
    foeNodeAnimationChannel const *pAnimChannel{nullptr};
    for (size_t channel = 0; channel < animationChannels.size(); ++channel) {
        if (animationChannels[channel].nodeName == pNode->name) {
            pAnimChannel = &animationChannels[channel];
        }
    }

    if (pAnimChannel == nullptr ||
        (pAnimChannel->positionKeys.empty() && pAnimChannel->rotationKeys.empty() &&
         pAnimChannel->scalingKeys.empty())) {
        *pArmatureTransform = pNode->transformMatrix;
    } else {
        glm::vec3 posVec = interpolatePosition(animationTick, pAnimChannel);
        glm::mat4 posMat = glm::translate(glm::mat4(1.f), posVec);

        glm::quat rotQuat = interpolateRotation(animationTick, pAnimChannel);
        glm::mat4 rotMat = glm::mat4_cast(rotQuat);

        glm::vec3 scaleVec = interpolateScaling(animationTick, pAnimChannel);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), scaleVec);

        *pArmatureTransform = parentTransform * posMat * rotMat * scaleMat;
    }

    for (size_t child = 0; child < pNode->numChildren; ++child) {
        size_t const offset = pNode->childrenOffset + child;

        animateArmatureNode(pNode + offset, animationChannels, animationTick, *pArmatureTransform,
                            pArmatureTransform + offset);
    }
}

void animateArmature(foeArmature const *pArmature,
                     uint32_t animationIndex,
                     float time,
                     glm::mat4 *pBoneData) {
    if (animationIndex >= pArmature->animations.size()) {
        // The original armature matrices
        originalArmatureNode(&pArmature->armature[0], glm::mat4{1.f}, pBoneData);
    } else {
        // Actual animation
        foeAnimation const &animation = pArmature->animations[animationIndex];

        auto const animationDuration = animation.duration / animation.ticksPerSecond;
        auto animationTime = time;
        while (animationTime > animationDuration) {
            animationTime -= animationDuration;
        }

        animationTime *= animation.ticksPerSecond;

        animateArmatureNode(&pArmature->armature[0], animation.nodeChannels, animationTime,
                            glm::mat4{1.f}, pBoneData);
    }
}

} // namespace

extern "C" foeResultSet foeCreateAnimatedBoneSystem(foeAnimatedBoneSystem *pAnimatedBoneSystem) {
    AnimatedBoneSystem *pNewAnimatedBoneSystem = new (std::nothrow) AnimatedBoneSystem{};
    if (pNewAnimatedBoneSystem == nullptr)
        return to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);

    *pAnimatedBoneSystem = animated_bone_system_to_handle(pNewAnimatedBoneSystem);
    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

extern "C" void foeDestroyAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem) {
    AnimatedBoneSystem *pRenderSystem = animated_bone_system_from_handle(animatedBoneSystem);

    delete pRenderSystem;
}

extern "C" foeResultSet foeInitializeAnimatedBoneSystem(
    foeAnimatedBoneSystem animatedBoneSystem,
    foeResourcePool resourcePool,
    foeArmatureStatePool armatureStatePool,
    foeAnimatedBoneStatePool animatedBoneStatePool) {
    AnimatedBoneSystem *pAnimatedBoneSystem = animated_bone_system_from_handle(animatedBoneSystem);

    if (resourcePool == nullptr) {
        return to_foeResult(FOE_BRINGUP_ERROR_NO_ARMATURE_POOL_PROVIDED);
    }
    if (armatureStatePool == nullptr) {
        return to_foeResult(FOE_BRINGUP_ERROR_NO_ARMATURE_STATE_POOL_PROVIDED);
    }

    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    pAnimatedBoneSystem->mResourcePool = resourcePool;
    pAnimatedBoneSystem->mArmatureStatePool = armatureStatePool;
    pAnimatedBoneSystem->mAnimatedBoneStatePool = animatedBoneStatePool;

    // Compile initial set of animated bone states
    { // Inserted ArmatureState
        foeEntityID const *pArmatureStateID = foeEcsComponentPoolIdPtr(armatureStatePool);
        foeEntityID const *const pEndArmatureStateID =
            pArmatureStateID + foeEcsComponentPoolSize(armatureStatePool);
        foeArmatureState const *pArmatureStateData =
            (foeArmatureState const *)foeEcsComponentPoolDataPtr(armatureStatePool);

        for (; pArmatureStateID != pEndArmatureStateID; ++pArmatureStateID, ++pArmatureStateData) {
            foeResource armature = FOE_NULL_HANDLE;
            do {
                armature = foeResourcePoolFind(resourcePool, pArmatureStateData->armatureID);

                if (armature == FOE_NULL_HANDLE) {
                    armature = foeResourcePoolAdd(resourcePool, pArmatureStateData->armatureID,
                                                  FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE,
                                                  sizeof(foeArmature));
                }
            } while (armature == FOE_NULL_HANDLE);

            // Resource ID provided does not match with an Armature type, don't use it
            if (foeResourceGetType(armature) != FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE) {
                foeResourceDecrementRefCount(armature);
                continue;
            }

            foeResourceIncrementUseCount(armature);

            foeResourceLoadState loadState = foeResourceGetState(armature);
            switch (loadState) {
            case FOE_RESOURCE_LOAD_STATE_LOADED: {
                foeArmature const *pArmature = (foeArmature const *)foeResourceGetData(armature);

                foeAnimatedBoneState newData{
                    .armature = armature,
                    .boneCount = (uint32_t)pArmature->armature.size(),
                    .pBones = (glm::mat4 *)malloc(pArmature->armature.size() * sizeof(glm::mat4)),
                };
                if (newData.pBones == nullptr) {
                    result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
                    return result;
                }

                animateArmature(pArmature, pArmatureStateData->animationID,
                                pArmatureStateData->time, newData.pBones);

                result =
                    foeEcsComponentPoolInsert(animatedBoneStatePool, *pArmatureStateID, &newData);
                if (result.value != FOE_SUCCESS) {
                    free(newData.pBones);
                    return result;
                }
            } break;

            case FOE_RESOURCE_LOAD_STATE_FAILED:
                // It failed to load, break out without adding to either the derived component pool
                // or adding to the awating list
                break;

            case FOE_RESOURCE_LOAD_STATE_UNLOADED:
                // Not yet loaded, possibly being loaded
                if (!foeResourceGetIsLoading(armature)) {
                    foeResourceLoadData(armature);
                }
                pAnimatedBoneSystem->mAwaitingLoading.emplace_back(AwaitingData{
                    .entity = *pArmatureStateID,
                    .armature = armature,
                });
                break;
            }
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeDeinitializeAnimatedBoneSystem(animatedBoneSystem);

    return result;
}

extern "C" void foeDeinitializeAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem) {
    AnimatedBoneSystem *pAnimatedBoneSystem = animated_bone_system_from_handle(animatedBoneSystem);

    // Dereference any resources being waited on
    for (auto const &awaited : pAnimatedBoneSystem->mAwaitingLoading) {
        foeResourceDecrementUseCount(awaited.armature);
        foeResourceDecrementRefCount(awaited.armature);
    }
    pAnimatedBoneSystem->mAwaitingLoading.clear();

    // Set AnimatedBoneData components to be cleared next maintenance cycle
    { // Regular AnimatedBoneState
        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mAnimatedBoneStatePool);
        foeEntityID const *const pEndAnimatedBoneStateID =
            pAnimatedBoneStateID +
            foeEcsComponentPoolSize(pAnimatedBoneSystem->mAnimatedBoneStatePool);
        foeAnimatedBoneState const *pAnimatedBoneStateData =
            (foeAnimatedBoneState *)foeEcsComponentPoolDataPtr(
                pAnimatedBoneSystem->mAnimatedBoneStatePool);

        for (; pAnimatedBoneStateID != pEndAnimatedBoneStateID;
             ++pAnimatedBoneStateID, ++pAnimatedBoneStateData) {
            cleanup_foeAnimatedBoneState(pAnimatedBoneStateData);
            memset((void *)pAnimatedBoneStateData, 0, sizeof(foeAnimatedBoneState));

            foeEcsComponentPoolRemove(pAnimatedBoneSystem->mAnimatedBoneStatePool,
                                      *pAnimatedBoneStateID);
        }
    }

    pAnimatedBoneSystem->mAnimatedBoneStatePool = FOE_NULL_HANDLE;
    pAnimatedBoneSystem->mArmatureStatePool = FOE_NULL_HANDLE;
    pAnimatedBoneSystem->mResourcePool = nullptr;
}

extern "C" foeResultSet foeProcessAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem,
                                                     float timeElapsed) {
    AnimatedBoneSystem *pAnimatedBoneSystem = animated_bone_system_from_handle(animatedBoneSystem);

    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    { // Process 'awaiting' items
        auto dataSets = std::move(pAnimatedBoneSystem->mAwaitingLoading);
        std::sort(dataSets.begin(), dataSets.end(),
                  [](auto const &a, auto const &b) { return a.entity < b.entity; });

        foeEntityID const *pArmatureStateID =
            foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mArmatureStatePool);
        foeEntityID const *const pStartArmatureStateID = pArmatureStateID;
        foeEntityID const *const pEndArmatureStateID =
            pStartArmatureStateID +
            foeEcsComponentPoolSize(pAnimatedBoneSystem->mArmatureStatePool);
        foeArmatureState *const pStartArmatureStateData =
            (foeArmatureState *const)foeEcsComponentPoolDataPtr(
                pAnimatedBoneSystem->mArmatureStatePool);

        auto awaitingIt = dataSets.begin();
        for (; awaitingIt != dataSets.end(); ++awaitingIt) {
            pArmatureStateID =
                std::lower_bound(pArmatureStateID, pEndArmatureStateID, awaitingIt->entity);
            if (pArmatureStateID == pEndArmatureStateID)
                break;
            if (*pArmatureStateID != awaitingIt->entity) {
                foeResourceDecrementUseCount(awaitingIt->armature);
                foeResourceDecrementRefCount(awaitingIt->armature);
                continue;
            }

            // If here, then there is an associated armature state
            foeArmatureState const *const pArmatureStateData =
                pStartArmatureStateData + (pArmatureStateID - pStartArmatureStateID);

            foeResourceLoadState loadState = foeResourceGetState(awaitingIt->armature);
            switch (loadState) {
            case FOE_RESOURCE_LOAD_STATE_LOADED: {
                foeArmature const *pArmature =
                    (foeArmature const *)foeResourceGetData(awaitingIt->armature);

                foeAnimatedBoneState newData{
                    .armature = awaitingIt->armature,
                    .boneCount = (uint32_t)pArmature->armature.size(),
                    .pBones = (glm::mat4 *)malloc(pArmature->armature.size() * sizeof(glm::mat4)),
                };
                if (newData.pBones == nullptr) {
                    result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
                    return result;
                }

                animateArmature(pArmature, pArmatureStateData->animationID,
                                pArmatureStateData->time, newData.pBones);

                result = foeEcsComponentPoolInsert(pAnimatedBoneSystem->mAnimatedBoneStatePool,
                                                   awaitingIt->entity, &newData);
                if (result.value != FOE_SUCCESS) {
                    free(newData.pBones);
                    return result;
                }
            } break;

            case FOE_RESOURCE_LOAD_STATE_FAILED:
                // It failed to load, break out without adding to either the derived
                // component pool or adding to the awating list
                break;

            case FOE_RESOURCE_LOAD_STATE_UNLOADED:
                // Not yet loaded, possibly being loaded
                if (!foeResourceGetIsLoading(awaitingIt->armature)) {
                    foeResourceLoadData(awaitingIt->armature);
                }
                pAnimatedBoneSystem->mAwaitingLoading.emplace_back(*awaitingIt);
                break;
            }
        }

    END_AWAITING_RESOURCE_PROCESSING:
        for (; awaitingIt != dataSets.end(); ++awaitingIt) {
            foeResourceDecrementUseCount(awaitingIt->armature);
            foeResourceDecrementRefCount(awaitingIt->armature);
        }
    }

    { // Removed ArmatureState
        foeEntityID const *pArmatureStateID =
            foeEcsComponentPoolRemovedIdPtr(pAnimatedBoneSystem->mArmatureStatePool);
        foeEntityID const *const pEndArmatureStateID =
            pArmatureStateID + foeEcsComponentPoolRemoved(pAnimatedBoneSystem->mArmatureStatePool);

        for (; pArmatureStateID != pEndArmatureStateID; ++pArmatureStateID) {
            foeEcsComponentPoolRemove(pAnimatedBoneSystem->mAnimatedBoneStatePool,
                                      *pArmatureStateID);
        }
    }

    { // Modified ArmatureState
        size_t const entityListCount =
            foeEcsComponentPoolEntityListSize(pAnimatedBoneSystem->mArmatureStatePool);
        foeEcsEntityList const *pLists =
            foeEcsComponentPoolEntityLists(pAnimatedBoneSystem->mArmatureStatePool);

        for (size_t i = 0; i < entityListCount; ++i) {
            foeEcsEntityList entityList = pLists[i];

            foeEntityID const *pModifiedID = foeEcsEntityListPtr(entityList);
            foeEntityID const *const pEndModifiedID =
                pModifiedID + foeEcsEntityListSize(entityList);

            foeEntityID const *pArmatureStateID =
                foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mArmatureStatePool);
            foeEntityID const *const pStartArmatureStateID = pArmatureStateID;
            foeEntityID const *const pEndArmatureStateID =
                pStartArmatureStateID +
                foeEcsComponentPoolSize(pAnimatedBoneSystem->mArmatureStatePool);
            foeArmatureState *const pStartArmatureStateData =
                (foeArmatureState *const)foeEcsComponentPoolDataPtr(
                    pAnimatedBoneSystem->mArmatureStatePool);

            foeEntityID const *pAnimatedBoneStateID =
                foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mAnimatedBoneStatePool);
            foeEntityID *const pStartAnimatedBoneStateID = (foeEntityID *const)pAnimatedBoneStateID;
            foeEntityID const *const pEndAnimatedBoneStateID =
                pStartAnimatedBoneStateID +
                foeEcsComponentPoolSize(pAnimatedBoneSystem->mAnimatedBoneStatePool);
            foeAnimatedBoneState *const pStartAnimatedBoneStateData =
                (foeAnimatedBoneState *)foeEcsComponentPoolDataPtr(
                    pAnimatedBoneSystem->mAnimatedBoneStatePool);

            for (; pModifiedID != pEndModifiedID; ++pModifiedID) {
                pArmatureStateID =
                    std::lower_bound(pArmatureStateID, pEndArmatureStateID, *pModifiedID);
                if (pArmatureStateID == pEndArmatureStateID)
                    break;
                if (*pArmatureStateID != *pModifiedID)
                    continue;

                foeArmatureState *const pArmatureStateData =
                    pStartArmatureStateData + (pArmatureStateID - pStartArmatureStateID);

                pAnimatedBoneStateID =
                    std::lower_bound(pAnimatedBoneStateID, pEndAnimatedBoneStateID, *pModifiedID);

                if (pAnimatedBoneStateID == pEndAnimatedBoneStateID ||
                    *pAnimatedBoneStateID != *pModifiedID) {
                    // There is no associated AnimatedBoneState, check if we should add it
                    foeResource armature = FOE_NULL_HANDLE;
                    do {
                        armature = foeResourcePoolFind(pAnimatedBoneSystem->mResourcePool,
                                                       pArmatureStateData->armatureID);

                        if (armature == FOE_NULL_HANDLE) {
                            armature = foeResourcePoolAdd(
                                pAnimatedBoneSystem->mResourcePool, pArmatureStateData->armatureID,
                                FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE, sizeof(foeArmature));
                        }
                    } while (armature == FOE_NULL_HANDLE);

                    // Resource ID provided does not match with an Armature type, don't use it
                    if (foeResourceGetType(armature) != FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE) {
                        foeResourceDecrementRefCount(armature);
                        continue;
                    }

                    foeResourceIncrementUseCount(armature);

                    foeResourceLoadState loadState = foeResourceGetState(armature);
                    switch (loadState) {
                    case FOE_RESOURCE_LOAD_STATE_LOADED: {
                        foeArmature const *pArmature =
                            (foeArmature const *)foeResourceGetData(armature);

                        foeAnimatedBoneState newData{
                            .armature = armature,
                            .boneCount = (uint32_t)pArmature->armature.size(),
                            .pBones =
                                (glm::mat4 *)malloc(pArmature->armature.size() * sizeof(glm::mat4)),
                        };
                        if (newData.pBones == nullptr) {
                            result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
                            return result;
                        }

                        animateArmature(pArmature, pArmatureStateData->animationID,
                                        pArmatureStateData->time, newData.pBones);

                        result = foeEcsComponentPoolInsert(
                            pAnimatedBoneSystem->mAnimatedBoneStatePool, *pModifiedID, &newData);
                        if (result.value != FOE_SUCCESS) {
                            free(newData.pBones);
                            return result;
                        }
                    } break;

                    case FOE_RESOURCE_LOAD_STATE_FAILED:
                        // It failed to load, break out without adding to either the derived
                        // component pool or adding to the awating list

                        // Remove component since we can't do animation processing
                        result = foeEcsComponentPoolRemove(
                            pAnimatedBoneSystem->mAnimatedBoneStatePool, *pModifiedID);
                        if (result.value != FOE_SUCCESS)
                            std::abort();
                        break;

                    case FOE_RESOURCE_LOAD_STATE_UNLOADED:
                        // Not yet loaded, possibly being loaded

                        // Remove component until it is loaded since we can't do animation
                        // processing
                        result = foeEcsComponentPoolRemove(
                            pAnimatedBoneSystem->mAnimatedBoneStatePool, *pModifiedID);
                        if (result.value != FOE_SUCCESS)
                            std::abort();

                        if (!foeResourceGetIsLoading(armature)) {
                            foeResourceLoadData(armature);
                        }
                        pAnimatedBoneSystem->mAwaitingLoading.emplace_back(AwaitingData{
                            .entity = *pModifiedID,
                            .armature = armature,
                        });
                        break;
                    }
                } else {
                    // There is already an associated AnimatedBoneState component, see what's
                    // been changed and what, if anything needs to be done
                    foeAnimatedBoneState *pAnimatedBoneStateData =
                        pStartAnimatedBoneStateData +
                        (pAnimatedBoneStateID - pStartAnimatedBoneStateID);

                    if (pArmatureStateData->armatureID !=
                        foeResourceGetID(pAnimatedBoneStateData->armature)) {
                        // The armature has changed
                        foeResource newArmature = FOE_NULL_HANDLE;
                        foeResourceLoadState loadState = FOE_RESOURCE_LOAD_STATE_FAILED;

                        // Acquire new armature resource
                        do {
                            newArmature = foeResourcePoolFind(pAnimatedBoneSystem->mResourcePool,
                                                              pArmatureStateData->armatureID);

                            if (newArmature == FOE_NULL_HANDLE) {
                                newArmature = foeResourcePoolAdd(
                                    pAnimatedBoneSystem->mResourcePool,
                                    pArmatureStateData->armatureID,
                                    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE, sizeof(foeArmature));
                            }
                        } while (newArmature == FOE_NULL_HANDLE);

                        if (foeResourceGetType(newArmature) !=
                            FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE) {
                            // Newly acquired resource is not correct type, don't use it
                            foeResourceDecrementRefCount(newArmature);
                        } else {
                            foeResourceIncrementUseCount(newArmature);
                            loadState = foeResourceGetState(newArmature);
                        }

                        switch (loadState) {
                        case FOE_RESOURCE_LOAD_STATE_LOADED: {
                            // Unreference the old armature
                            foeResourceDecrementUseCount(pAnimatedBoneStateData->armature);
                            foeResourceDecrementRefCount(pAnimatedBoneStateData->armature);

                            // Set the new armature
                            pAnimatedBoneStateData->armature = newArmature;

                            foeArmature const *pArmature =
                                (foeArmature const *)foeResourceGetData(newArmature);

                            // Update the bone allocation if necessary
                            if (pAnimatedBoneStateData->boneCount != pArmature->armature.size()) {
                                glm::mat4 *pNewBoneAlloc = (glm::mat4 *)malloc(
                                    pArmature->armature.size() * sizeof(glm::mat4));
                                if (pAnimatedBoneStateData->pBones == nullptr) {
                                    result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
                                    return result;
                                }

                                free(pAnimatedBoneStateData->pBones);
                                pAnimatedBoneStateData->pBones = pNewBoneAlloc;
                                pAnimatedBoneStateData->boneCount = pArmature->armature.size();
                            }

                            animateArmature(pArmature, pArmatureStateData->animationID,
                                            pArmatureStateData->time,
                                            pAnimatedBoneStateData->pBones);
                        } break;

                        case FOE_RESOURCE_LOAD_STATE_FAILED:
                            // It failed to load, break out without adding to either the derived
                            // component pool or adding to the awating list
                            foeEcsComponentPoolRemove(pAnimatedBoneSystem->mAnimatedBoneStatePool,
                                                      *pArmatureStateID);
                            break;

                        case FOE_RESOURCE_LOAD_STATE_UNLOADED:
                            // Not yet loaded, possibly being loaded
                            foeEcsComponentPoolRemove(pAnimatedBoneSystem->mAnimatedBoneStatePool,
                                                      *pArmatureStateID);

                            if (!foeResourceGetIsLoading(newArmature)) {
                                foeResourceLoadData(newArmature);
                            }
                            pAnimatedBoneSystem->mAwaitingLoading.emplace_back(AwaitingData{
                                .entity = *pModifiedID,
                                .armature = newArmature,
                            });
                            break;
                        }
                    }
                }
            }
        }
    }

    { // Inserted ArmatureState
        foeEntityID const *const pStartArmatureStateID =
            foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mArmatureStatePool);
        foeArmatureState const *const pStartArmatureStateData =
            (foeArmatureState const *)foeEcsComponentPoolDataPtr(
                pAnimatedBoneSystem->mArmatureStatePool);

        size_t const *pOffset =
            foeEcsComponentPoolInsertedOffsetPtr(pAnimatedBoneSystem->mArmatureStatePool);
        size_t const *const pEndOffset =
            pOffset + foeEcsComponentPoolInserted(pAnimatedBoneSystem->mArmatureStatePool);

        for (; pOffset != pEndOffset; ++pOffset) {
            foeEntityID entity = pStartArmatureStateID[*pOffset];
            foeArmatureState const *const pArmatureStateData = pStartArmatureStateData + *pOffset;

            foeResource armature = FOE_NULL_HANDLE;
            do {
                armature = foeResourcePoolFind(pAnimatedBoneSystem->mResourcePool,
                                               pArmatureStateData->armatureID);

                if (armature == FOE_NULL_HANDLE) {
                    armature = foeResourcePoolAdd(
                        pAnimatedBoneSystem->mResourcePool, pArmatureStateData->armatureID,
                        FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE, sizeof(foeArmature));
                }
            } while (armature == FOE_NULL_HANDLE);

            if (foeResourceGetType(armature) != FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE) {
                foeResourceDecrementRefCount(armature);
                continue;
            }

            foeResourceIncrementUseCount(armature);

            foeResourceLoadState loadState = foeResourceGetState(armature);
            switch (loadState) {
            case FOE_RESOURCE_LOAD_STATE_LOADED: {
                foeArmature const *pArmature = (foeArmature const *)foeResourceGetData(armature);

                foeAnimatedBoneState newData{
                    .armature = armature,
                    .boneCount = (uint32_t)pArmature->armature.size(),
                    .pBones = (glm::mat4 *)malloc(pArmature->armature.size() * sizeof(glm::mat4)),
                };
                if (newData.pBones == nullptr) {
                    result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
                    return result;
                }

                animateArmature(pArmature, pArmatureStateData->animationID,
                                pArmatureStateData->time, newData.pBones);

                result = foeEcsComponentPoolInsert(pAnimatedBoneSystem->mAnimatedBoneStatePool,
                                                   entity, &newData);
                if (result.value != FOE_SUCCESS) {
                    free(newData.pBones);
                    return result;
                }
            } break;

            case FOE_RESOURCE_LOAD_STATE_FAILED:
                // It failed to load, break out without adding to either the derived component
                // pool or adding to the awating list
                break;

            case FOE_RESOURCE_LOAD_STATE_UNLOADED:
                // Not yet loaded, possibly being loaded
                if (!foeResourceGetIsLoading(armature)) {
                    foeResourceLoadData(armature);
                }
                pAnimatedBoneSystem->mAwaitingLoading.emplace_back(AwaitingData{
                    .entity = entity,
                    .armature = armature,
                });
                break;
            }
        }
    }

    { // Process all current AnimatedBoneStates
        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mAnimatedBoneStatePool);
        foeEntityID const *const pEndAnimatedBoneStateID =
            pAnimatedBoneStateID +
            foeEcsComponentPoolSize(pAnimatedBoneSystem->mAnimatedBoneStatePool);
        foeAnimatedBoneState *pAnimatedBoneStateData =
            (foeAnimatedBoneState *)foeEcsComponentPoolDataPtr(
                pAnimatedBoneSystem->mAnimatedBoneStatePool);

        foeEntityID const *pArmatureStateID =
            foeEcsComponentPoolIdPtr(pAnimatedBoneSystem->mArmatureStatePool);
        foeEntityID const *const pStartArmatureStateID = pArmatureStateID;
        foeEntityID const *const pEndArmatureStateID =
            pArmatureStateID + foeEcsComponentPoolSize(pAnimatedBoneSystem->mArmatureStatePool);
        foeArmatureState *const pStartArmatureStateData =
            (foeArmatureState *)foeEcsComponentPoolDataPtr(pAnimatedBoneSystem->mArmatureStatePool);

        for (; pAnimatedBoneStateID != pEndAnimatedBoneStateID;
             ++pAnimatedBoneStateID, ++pAnimatedBoneStateData) {
            pArmatureStateID =
                std::lower_bound(pArmatureStateID, pEndArmatureStateID, *pAnimatedBoneStateID);
            if (pArmatureStateID == pEndArmatureStateID)
                break;
            if (*pArmatureStateID != *pAnimatedBoneStateID)
                continue;
            foeArmatureState *pArmatureStateData =
                pStartArmatureStateData + (pArmatureStateID - pStartArmatureStateID);

            pArmatureStateData->time += timeElapsed;

            foeArmature const *pArmature =
                (foeArmature const *)foeResourceGetData(pAnimatedBoneStateData->armature);

            animateArmature(pArmature, pArmatureStateData->animationID, pArmatureStateData->time,
                            pAnimatedBoneStateData->pBones);
        }
    }

    return result;
}