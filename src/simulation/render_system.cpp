// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_system.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/position/component/3d.hpp>

#include "../log.hpp"
#include "../result.h"
#include "../vk_result.h"
#include "armature.hpp"
#include "render_system_armature.hpp"
#include "render_system_position.hpp"

#include <algorithm>
#include <vector>

namespace {

struct RenderSystem {
    // Non Graphics
    foeResourcePool resourcePool;
    foeRenderStatePool renderStatePool;
    foePosition3dPool positionPool;
    foeAnimatedBoneStatePool animatedBoneStatePool;

    std::vector<RenderDataSet> awaitingLoading;

    std::vector<RenderDataSet> renderData;

    // Graphics
    foeGfxSession gfxSession;
    uint32_t minBufferAlignment;

    // Subsystems
    RenderSystemPositionData positionData;
    RenderSystemArmatureData armatureData;
};

FOE_DEFINE_HANDLE_CASTS(render_system, RenderSystem, foeRenderSystem)

void clearRenderData(RenderResources const *pRenderData) {
    if (pRenderData->mesh != FOE_NULL_HANDLE) {
        foeResourceDecrementUseCount(pRenderData->mesh);
        foeResourceDecrementRefCount(pRenderData->mesh);
    }

    if (pRenderData->material != FOE_NULL_HANDLE) {
        foeResourceDecrementUseCount(pRenderData->material);
        foeResourceDecrementRefCount(pRenderData->material);
    }

    if (pRenderData->bonedVertexDescriptor != FOE_NULL_HANDLE) {
        foeResourceDecrementUseCount(pRenderData->bonedVertexDescriptor);
        foeResourceDecrementRefCount(pRenderData->bonedVertexDescriptor);
    }

    if (pRenderData->vertexDescriptor != FOE_NULL_HANDLE) {
        foeResourceDecrementUseCount(pRenderData->vertexDescriptor);
        foeResourceDecrementRefCount(pRenderData->vertexDescriptor);
    }
}

bool getResourceData(foeResourcePool resourcePool,
                     foeResourceID resourceID,
                     foeResourceType resourceType,
                     size_t resourceTypeSize,
                     foeResource oldResource,
                     foeResource &newResource) {
    if (oldResource != FOE_NULL_HANDLE) {
        if (foeResourceType type = foeResourceGetType(oldResource);
            foeResourceGetID(oldResource) == resourceID &&
            (type == resourceType || type == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED ||
             type == FOE_RESOURCE_RESOURCE_TYPE_REPLACED ||
             foeResourceHasType(oldResource, resourceType))) {
            // The old resource is the ID and type we still want, just re-use it
            newResource = oldResource;
            return true;
        }

        // Can't re-use old resource, decrement it since we no longer want it
        foeResourceDecrementUseCount(oldResource);
        foeResourceDecrementRefCount(oldResource);
    }

    // Check if we even want a resource
    if (resourceID == FOE_INVALID_ID)
        // No actual resource to acquire here, successful return
        return true;

    // If here, need to try to acquire the resource
    do {
        newResource = foeResourcePoolFind(resourcePool, resourceID);

        if (newResource == FOE_NULL_HANDLE)
            newResource = foeResourcePoolAdd(resourcePool, resourceID);
    } while (newResource == FOE_NULL_HANDLE);

    // Make sure retrieved resource is the correct type, or could become the desired type
    if (foeResourceType type = foeResourceGetType(newResource);
        type != resourceType && type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
        type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
        !foeResourceHasType(newResource, resourceType))
        return false;

    return true;
}

bool processResourceReplacement(foeResource *pResource) {
    bool replaced = false;

    if (*pResource == FOE_NULL_HANDLE)
        return false;

    while (foeResourceGetType(*pResource) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED) {
        foeResource replacement = foeResourceGetReplacement(*pResource);

        foeResourceIncrementUseCount(replacement);

        foeResourceDecrementUseCount(*pResource);
        foeResourceDecrementRefCount(*pResource);

        *pResource = replacement;
        replaced = true;
    }

    return replaced;
}

foeResourceStateFlags processResourceLoadState(foeResource *pResource,
                                               foeResourceType resourceType,
                                               foeResourceStateFlags overallState) {
    if (*pResource == FOE_NULL_HANDLE)
        return overallState;

    foeResourceStateFlags resourceState;

    do {
        resourceState = foeResourceGetState(*pResource);
    } while (processResourceReplacement(pResource));

    if (foeResourceType type = foeResourceGetType(*pResource);
        type != resourceType && type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
        type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
        !foeResourceHasType(*pResource, resourceType)) {
        overallState |= FOE_RESOURCE_STATE_FAILED_BIT;
    }

    if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
        overallState |= FOE_RESOURCE_STATE_FAILED_BIT;
    } else if ((resourceState & FOE_RESOURCE_STATE_LOADED_BIT) == 0) {
        // This resource is not loaded, therefore remove the overall LOADED flag
        overallState &= ~FOE_RESOURCE_STATE_LOADED_BIT;

        if ((resourceState & FOE_RESOURCE_STATE_LOADING_BIT) == 0) {
            // Resource is not LOADED and not LOADING, request load
            foeResourceLoadData(*pResource);
            overallState |= FOE_RESOURCE_STATE_LOADING_BIT;
        }
    }
    if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT)
        overallState |= FOE_RESOURCE_STATE_LOADING_BIT;

    return overallState;
}

foeResourceStateFlags loadResourceData(foeResource oldResource,
                                       foeResource &newResource,
                                       foeResourceType resourceType,
                                       foeResourceStateFlags overallState) {
    if (newResource == FOE_NULL_HANDLE)
        // No resource to even deal with, no change to overall load state
        return overallState;

    foeResourceStateFlags resourceState = foeResourceGetState(newResource);

    if (oldResource != newResource) {
        // A new resource, need to increment it's use and maybe start the loading process
        foeResourceIncrementUseCount(newResource);
    }

    return processResourceLoadState(&newResource, resourceType, overallState);
}

void getResourceCleanup(foeResource oldResource, foeResource newResource) {
    if (newResource == FOE_NULL_HANDLE)
        // oldResource would have been decremented by 'getResourceData' already
        return;

    if (oldResource == newResource) {
        // Since the resources match, then the resources' use has been previously incremented,
        // need to decrement it here
        foeResourceDecrementUseCount(newResource);
    }

    foeResourceDecrementRefCount(newResource);
}

[[nodiscard]] foeResourceStateFlags getRenderData(foeResourcePool resourcePool,
                                                  foeRenderState const *pRenderState,
                                                  RenderResources *pRenderData) {
    RenderResources newResourceData = {};
    bool good = true;
    foeResourceStateFlags overallResourceState = FOE_RESOURCE_STATE_FAILED_BIT;

    good = good && getResourceData(resourcePool, pRenderState->vertexDescriptor,
                                   FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
                                   sizeof(foeVertexDescriptor), pRenderData->vertexDescriptor,
                                   newResourceData.vertexDescriptor);

    good = good && getResourceData(resourcePool, pRenderState->bonedVertexDescriptor,
                                   FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
                                   sizeof(foeVertexDescriptor), pRenderData->bonedVertexDescriptor,
                                   newResourceData.bonedVertexDescriptor);

    good =
        good && getResourceData(resourcePool, pRenderState->material,
                                FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL, sizeof(foeMaterial),
                                pRenderData->material, newResourceData.material);

    good = good && getResourceData(resourcePool, pRenderState->mesh,
                                   FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH, sizeof(foeMesh),
                                   pRenderData->mesh, newResourceData.mesh);

    if (good) {
        // Things are in a good state, so increment the usage of each of the new resources, and
        // start loading them if necessary

        // We only try to increment use and load all together after checking that all desired
        // resources exist and are the correct types so we don't start doing expensive resource
        // loading when we're missing things we didn;'t have anyways

        // Set to good state initially
        overallResourceState = FOE_RESOURCE_STATE_LOADED_BIT;

        overallResourceState = loadResourceData(
            pRenderData->vertexDescriptor, newResourceData.vertexDescriptor,
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR, overallResourceState);
        overallResourceState = loadResourceData(
            pRenderData->bonedVertexDescriptor, newResourceData.bonedVertexDescriptor,
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR, overallResourceState);
        overallResourceState =
            loadResourceData(pRenderData->material, newResourceData.material,
                             FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL, overallResourceState);
        overallResourceState =
            loadResourceData(pRenderData->mesh, newResourceData.mesh,
                             FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH, overallResourceState);
    }

    if ((overallResourceState & FOE_RESOURCE_STATE_FAILED_BIT) == 0) {
        *pRenderData = newResourceData;
        return overallResourceState;
    }

    // We failed somewhere, clean up
    getResourceCleanup(pRenderData->vertexDescriptor, newResourceData.vertexDescriptor);
    getResourceCleanup(pRenderData->bonedVertexDescriptor, newResourceData.bonedVertexDescriptor);
    getResourceCleanup(pRenderData->material, newResourceData.material);
    getResourceCleanup(pRenderData->mesh, newResourceData.mesh);

    return overallResourceState;
}

} // namespace

extern "C" foeResultSet foeCreateRenderSystem(foeRenderSystem *pRenderSystem) {
    RenderSystem *pNewRenderSystem = new (std::nothrow) RenderSystem{};
    if (pNewRenderSystem == nullptr)
        return to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);

    *pRenderSystem = render_system_to_handle(pNewRenderSystem);
    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

extern "C" void foeDestroyRenderSystem(foeRenderSystem renderSystem) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);

    delete pRenderSystem;
}

extern "C" foeResultSet foeInitializeRenderSystemGraphics(
    foeRenderSystem renderSystem,
    foeGfxSession gfxSession,
    foeResourcePool resourcePool,
    foeRenderStatePool renderStatePool,
    foePosition3dPool positionPool,
    foeAnimatedBoneStatePool animatedBoneStatePool) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    // Checks
    if (gfxSession == FOE_NULL_HANDLE)
        std::abort();
    if (resourcePool == FOE_NULL_HANDLE || renderStatePool == FOE_NULL_HANDLE ||
        positionPool == FOE_NULL_HANDLE || animatedBoneStatePool == FOE_NULL_HANDLE)
        std::abort();

    // Set external data
    pRenderSystem->resourcePool = resourcePool;
    pRenderSystem->renderStatePool = renderStatePool;
    pRenderSystem->positionPool = positionPool;
    pRenderSystem->animatedBoneStatePool = animatedBoneStatePool;
    pRenderSystem->gfxSession = gfxSession;

    // Subsystems
    result = initializePositionData(gfxSession, pRenderSystem->positionData);
    if (result.value != FOE_SUCCESS)
        goto GRAPHICS_INITIALIZATION_FAILED;

    result = initializeArmatureData(gfxSession, pRenderSystem->armatureData);
    if (result.value != FOE_SUCCESS)
        goto GRAPHICS_INITIALIZATION_FAILED;

    { // Compile initial data sets
        pRenderSystem->renderData.reserve(foeEcsComponentPoolSize(renderStatePool));

        foeEntityID const *pRenderStateID = foeEcsComponentPoolIdPtr(renderStatePool);
        foeEntityID const *const pEndRenderStateID =
            pRenderStateID + foeEcsComponentPoolSize(renderStatePool);
        foeRenderState const *pRenderStateData =
            (foeRenderState const *)foeEcsComponentPoolDataPtr(renderStatePool);

        foeEntityID const *pPositionID = foeEcsComponentPoolIdPtr(positionPool);
        foeEntityID const *const pStartPositionID = pPositionID;
        foeEntityID const *const pEndPositionID =
            pStartPositionID + foeEcsComponentPoolSize(positionPool);
        foePosition3d const *const *const ppStartPositionData =
            (foePosition3d const *const *const)foeEcsComponentPoolDataPtr(positionPool);

        foeEntityID const *pAnimatedBoneStateID = foeEcsComponentPoolIdPtr(animatedBoneStatePool);
        foeEntityID const *const pStartAnimatedBoneStateID = pAnimatedBoneStateID;
        foeEntityID const *const pEndAnimatedBoneStateID =
            pStartAnimatedBoneStateID + foeEcsComponentPoolSize(animatedBoneStatePool);
        foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
            (foeAnimatedBoneState const *const)foeEcsComponentPoolDataPtr(animatedBoneStatePool);

        for (; pRenderStateID != pEndRenderStateID; ++pRenderStateID, ++pRenderStateData) {
            // Check if associated position component exists
            pPositionID = std::lower_bound(pPositionID, pEndPositionID, *pRenderStateID);
            if (pPositionID == pEndPositionID)
                // No more possible positions, leave
                break;
            if (*pPositionID != *pRenderStateID)
                // There is no associated position component, continue
                continue;

            RenderDataSet newDataSet{.entity = *pRenderStateID, .armatureIndex = UINT32_MAX};

            foeResourceStateFlags resourceState =
                getRenderData(resourcePool, pRenderStateData, &newDataSet.resources);

            assert(resourceState & (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                                    FOE_RESOURCE_STATE_LOADED_BIT));

            if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
                // Some required resource failed to load
                clearArmatureData(pRenderSystem->armatureData, newDataSet.armatureIndex);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
                // Data is loaded, can add right away
                pAnimatedBoneStateID = std::lower_bound(pAnimatedBoneStateID,
                                                        pEndAnimatedBoneStateID, *pRenderStateID);
                if (pAnimatedBoneStateID != pEndAnimatedBoneStateID &&
                    *pAnimatedBoneStateID == *pRenderStateID) {
                    result = getArmatureData(pRenderSystem->armatureData,
                                             pStartAnimatedBoneStateData +
                                                 (pAnimatedBoneStateID - pStartAnimatedBoneStateID),
                                             newDataSet.resources.mesh, newDataSet.armatureIndex);
                    if (result.value != FOE_SUCCESS)
                        goto GRAPHICS_INITIALIZATION_FAILED;
                }

                result = insertPositionData(
                    pRenderSystem->positionData, pRenderSystem->renderData.size(),
                    *(ppStartPositionData + (pPositionID - pStartPositionID)));
                if (result.value != FOE_SUCCESS)
                    goto GRAPHICS_INITIALIZATION_FAILED;

                pRenderSystem->renderData.emplace_back(newDataSet);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
                // Something is loading
                pRenderSystem->awaitingLoading.emplace_back(newDataSet);
            }
        }
    }

GRAPHICS_INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeDeinitializeRenderSystemGraphics(renderSystem);

    return result;
}

extern "C" void foeDeinitializeRenderSystemGraphics(foeRenderSystem renderSystem) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);

    // Clear subsystem content
    deinitializeArmatureData(pRenderSystem->gfxSession, pRenderSystem->armatureData);
    deinitializePositionData(pRenderSystem->gfxSession, pRenderSystem->positionData);

    // Clear items awaiting loading
    for (auto const &dataSet : pRenderSystem->awaitingLoading) {
        clearRenderData(&dataSet.resources);
    }
    pRenderSystem->awaitingLoading.clear();

    // Decompile data sets
    for (auto const &dataSet : pRenderSystem->renderData) {
        clearRenderData(&dataSet.resources);
    }

    // Clear external data
    pRenderSystem->gfxSession = FOE_NULL_HANDLE;
    pRenderSystem->animatedBoneStatePool = FOE_NULL_HANDLE;
    pRenderSystem->positionPool = FOE_NULL_HANDLE;
    pRenderSystem->renderStatePool = FOE_NULL_HANDLE;
    pRenderSystem->resourcePool = FOE_NULL_HANDLE;
}

extern "C" foeResultSet foeProcessRenderSystem(foeRenderSystem renderSystem) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    { // Check items waiting on loading resources
        std::vector<RenderDataSet> dataSets = std::move(pRenderSystem->awaitingLoading);
        std::sort(
            dataSets.begin(), dataSets.end(),
            [](RenderDataSet const &a, RenderDataSet const &b) { return a.entity < b.entity; });

        foeEntityID const *pRenderStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->renderStatePool);
        foeEntityID const *const pEndRenderStateID =
            pRenderStateID + foeEcsComponentPoolSize(pRenderSystem->renderStatePool);
        foeRenderState const *pRenderStateData =
            (foeRenderState const *)foeEcsComponentPoolDataPtr(pRenderSystem->renderStatePool);

        foeEntityID const *pPositionID = foeEcsComponentPoolIdPtr(pRenderSystem->positionPool);
        foeEntityID const *const pStartPositionID = pPositionID;
        foeEntityID const *const pEndPositionID =
            pStartPositionID + foeEcsComponentPoolSize(pRenderSystem->positionPool);
        foePosition3d const *const *const ppStartPositionData =
            (foePosition3d const *const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->positionPool);

        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->animatedBoneStatePool);
        foeEntityID const *const pStartAnimatedBoneStateID = pAnimatedBoneStateID;
        foeEntityID const *const pEndAnimatedBoneStateID =
            pStartAnimatedBoneStateID +
            foeEcsComponentPoolSize(pRenderSystem->animatedBoneStatePool);
        foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
            (foeAnimatedBoneState const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->animatedBoneStatePool);

        auto renderDataIt = pRenderSystem->renderData.begin();

        auto awaitingIt = dataSets.begin();
        for (; awaitingIt != dataSets.end(); ++awaitingIt) {
            while (*pRenderStateID < awaitingIt->entity) {
                ++pRenderStateID;
                ++pRenderStateData;

                if (pRenderStateID == pEndRenderStateID)
                    goto END_AWAITING_RESOURCE_PROCESSING;
            }

            if (*pRenderStateID != awaitingIt->entity) {
                // Whatever we were loading is no longer in the RenderState component pool,
                // clear the data and continue
                clearArmatureData(pRenderSystem->armatureData, awaitingIt->armatureIndex);
                clearRenderData(&awaitingIt->resources);
                continue;
            }

            pPositionID = std::lower_bound(pPositionID, pEndPositionID, awaitingIt->entity);
            if (pPositionID == pEndPositionID)
                goto END_AWAITING_RESOURCE_PROCESSING;
            if (*pPositionID != awaitingIt->entity) {
                // Whatever we were loading is no longer in the Position component pool, clear
                // the data and continue
                clearArmatureData(pRenderSystem->armatureData, awaitingIt->armatureIndex);
                clearRenderData(&awaitingIt->resources);
                continue;
            }

            foeResourceStateFlags resourceState = getRenderData(
                pRenderSystem->resourcePool, pRenderStateData, &awaitingIt->resources);

            assert(resourceState & (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                                    FOE_RESOURCE_STATE_LOADED_BIT));

            if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
                clearArmatureData(pRenderSystem->armatureData, awaitingIt->armatureIndex);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
                // Loaded, ready to go
                renderDataIt = std::lower_bound(
                    renderDataIt, pRenderSystem->renderData.end(), awaitingIt->entity,
                    [](RenderDataSet const &obj, foeEntityID const entity) {
                        return obj.entity < entity;
                    });

                if (renderDataIt != pRenderSystem->renderData.end() &&
                    renderDataIt->entity == awaitingIt->entity) {
                    // If the item already exists in the render array, clear the awaiting data,
                    // should never happen maybe?
                    clearArmatureData(pRenderSystem->armatureData, awaitingIt->armatureIndex);
                    clearRenderData(&awaitingIt->resources);
                }

                pAnimatedBoneStateID = std::lower_bound(pAnimatedBoneStateID,
                                                        pEndAnimatedBoneStateID, *pRenderStateID);
                if (pAnimatedBoneStateID != pEndAnimatedBoneStateID &&
                    *pAnimatedBoneStateID == *pRenderStateID) {
                    result = getArmatureData(pRenderSystem->armatureData,
                                             pStartAnimatedBoneStateData +
                                                 (pAnimatedBoneStateID - pStartAnimatedBoneStateID),
                                             awaitingIt->resources.mesh, awaitingIt->armatureIndex);
                    if (result.value != FOE_SUCCESS)
                        return result;
                }

                size_t const posIndex = renderDataIt - pRenderSystem->renderData.begin();
                foePosition3d const *const pPositionData =
                    *(ppStartPositionData + (pPositionID - pStartPositionID));
                result = insertPositionData(pRenderSystem->positionData, posIndex, pPositionData);
                if (result.value != FOE_SUCCESS)
                    return result;

                pRenderSystem->renderData.insert(renderDataIt, *awaitingIt);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
                // Still loading
                pRenderSystem->awaitingLoading.emplace_back(*awaitingIt);
            }
        }

    END_AWAITING_RESOURCE_PROCESSING:
        for (; awaitingIt != dataSets.end(); ++awaitingIt) {
            clearArmatureData(pRenderSystem->armatureData, awaitingIt->armatureIndex);
            clearRenderData(&awaitingIt->resources);
        }
    }

    { // AnimatedBoneState removals
        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolRemovedIdPtr(pRenderSystem->animatedBoneStatePool);
        foeEntityID const *const pEndAnimatedBoneStateID =
            pAnimatedBoneStateID + foeEcsComponentPoolRemoved(pRenderSystem->animatedBoneStatePool);

        auto renderDataIt = pRenderSystem->renderData.begin();
        auto const endRenderDataIt = pRenderSystem->renderData.end();

        while (pAnimatedBoneStateID != pEndAnimatedBoneStateID) {
            renderDataIt = std::lower_bound(renderDataIt, endRenderDataIt, *pAnimatedBoneStateID,
                                            [](RenderDataSet const &obj, foeEntityID const entity) {
                                                return obj.entity < entity;
                                            });

            if (renderDataIt == endRenderDataIt)
                // Reached the end of render data, nothing left to do
                break;

            if (renderDataIt->entity == *pAnimatedBoneStateID)
                // If we found a match, then clear any armature data
                clearArmatureData(pRenderSystem->armatureData, renderDataIt->armatureIndex);

            ++pAnimatedBoneStateID;
        }
    }

    { // RenderState removals
        foeEntityID const *pRenderStateID =
            foeEcsComponentPoolRemovedIdPtr(pRenderSystem->renderStatePool);
        foeEntityID const *const pEndRenderStateID =
            pRenderStateID + foeEcsComponentPoolRemoved(pRenderSystem->renderStatePool);

        for (auto renderDataSetIt = pRenderSystem->renderData.begin();
             pRenderStateID != pEndRenderStateID &&
             renderDataSetIt != pRenderSystem->renderData.end();
             ++pRenderStateID) {
            while (*pRenderStateID > renderDataSetIt->entity) {
                ++renderDataSetIt;

                if (renderDataSetIt == pRenderSystem->renderData.end())
                    goto END_RENDER_STATE_REMOVAL_PROCESSING;
            }

            if (*pRenderStateID == renderDataSetIt->entity) {
                clearArmatureData(pRenderSystem->armatureData, renderDataSetIt->armatureIndex);
                clearRenderData(&renderDataSetIt->resources);

                removePositionData(pRenderSystem->positionData,
                                   renderDataSetIt - pRenderSystem->renderData.begin());

                renderDataSetIt = pRenderSystem->renderData.erase(renderDataSetIt);
            }
        }
    END_RENDER_STATE_REMOVAL_PROCESSING:;
    }

    { // Position removals
        foeEntityID const *pPositionID =
            foeEcsComponentPoolRemovedIdPtr(pRenderSystem->positionPool);
        foeEntityID const *const pEndPositionID =
            pPositionID + foeEcsComponentPoolRemoved(pRenderSystem->positionPool);

        for (auto renderDataSetIt = pRenderSystem->renderData.begin();
             pPositionID != pEndPositionID && renderDataSetIt != pRenderSystem->renderData.end();
             ++pPositionID) {
            while (*pPositionID > renderDataSetIt->entity) {
                ++renderDataSetIt;

                if (renderDataSetIt == pRenderSystem->renderData.end())
                    goto END_POSITION_REMOVAL_PROCESSING;
            }

            if (*pPositionID == renderDataSetIt->entity) {
                clearArmatureData(pRenderSystem->armatureData, renderDataSetIt->armatureIndex);
                clearRenderData(&renderDataSetIt->resources);

                removePositionData(pRenderSystem->positionData,
                                   renderDataSetIt - pRenderSystem->renderData.begin());

                renderDataSetIt = pRenderSystem->renderData.erase(renderDataSetIt);
            }
        }
    END_POSITION_REMOVAL_PROCESSING:;
    }

    { // Modified RenderState
        size_t const entityListCount =
            foeEcsComponentPoolEntityListSize(pRenderSystem->renderStatePool);
        foeEcsEntityList const *pLists =
            foeEcsComponentPoolEntityLists(pRenderSystem->renderStatePool);

        for (size_t i = 0; i < entityListCount; ++i) {
            foeEcsEntityList entityList = pLists[i];

            foeEntityID const *pModifiedID = foeEcsEntityListPtr(entityList);
            foeEntityID const *const pEndModifiedID =
                pModifiedID + foeEcsEntityListSize(entityList);

            foeEntityID const *pRenderStateID =
                foeEcsComponentPoolIdPtr(pRenderSystem->renderStatePool);
            foeEntityID const *const pStartRenderStateID = pRenderStateID;
            foeEntityID const *const pEndRenderStateID =
                pStartRenderStateID + foeEcsComponentPoolSize(pRenderSystem->renderStatePool);
            foeRenderState const *const pStartRenderStateData =
                (foeRenderState const *)foeEcsComponentPoolDataPtr(pRenderSystem->renderStatePool);

            foeEntityID const *pAnimatedBoneStateID =
                foeEcsComponentPoolIdPtr(pRenderSystem->animatedBoneStatePool);
            foeEntityID const *const pStartAnimatedBoneStateID = pAnimatedBoneStateID;
            foeEntityID const *const pEndAnimatedBoneStateID =
                pStartAnimatedBoneStateID +
                foeEcsComponentPoolSize(pRenderSystem->animatedBoneStatePool);
            foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
                (foeAnimatedBoneState const *const)foeEcsComponentPoolDataPtr(
                    pRenderSystem->animatedBoneStatePool);

            auto renderDataIt = pRenderSystem->renderData.begin();

            for (; pModifiedID != pEndModifiedID; ++pModifiedID) {
                renderDataIt =
                    std::lower_bound(renderDataIt, pRenderSystem->renderData.end(), *pModifiedID,
                                     [](RenderDataSet const &obj, foeEntityID const entity) {
                                         return obj.entity < entity;
                                     });

                // Reached the end of the held data, nothing else forward can be processed
                if (renderDataIt == pRenderSystem->renderData.end())
                    break;

                // If we didn't find an exact match, increment past and continue
                if (*pModifiedID != renderDataIt->entity)
                    continue;

                pRenderStateID = std::lower_bound(pRenderStateID, pEndRenderStateID, *pModifiedID);
                if (pRenderStateID == pEndRenderStateID)
                    // Reached the end of items in the component pool, leave
                    break;
                if (*pRenderStateID != *pModifiedID)
                    // The render state no longer exists in the pool, continue
                    continue;

                foeRenderState const *pRenderState =
                    pStartRenderStateData + (pRenderStateID - pStartRenderStateID);

                foeResourceStateFlags resourceState = getRenderData(
                    pRenderSystem->resourcePool, pRenderState, &renderDataIt->resources);

                assert(resourceState &
                       (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                        FOE_RESOURCE_STATE_LOADED_BIT));

                if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
                    // Some required resource failed to load
                    clearArmatureData(pRenderSystem->armatureData, renderDataIt->armatureIndex);

                    removePositionData(pRenderSystem->positionData,
                                       renderDataIt - pRenderSystem->renderData.begin());

                    renderDataIt = pRenderSystem->renderData.erase(renderDataIt);
                } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
                    // Everything is still considered loaded and ready to go
                    pAnimatedBoneStateID = std::lower_bound(
                        pAnimatedBoneStateID, pEndAnimatedBoneStateID, *pRenderStateID);
                    if (pAnimatedBoneStateID != pEndAnimatedBoneStateID &&
                        *pAnimatedBoneStateID == *pRenderStateID) {
                        result = getArmatureData(
                            pRenderSystem->armatureData,
                            pStartAnimatedBoneStateData +
                                (pAnimatedBoneStateID - pStartAnimatedBoneStateID),
                            renderDataIt->resources.mesh, renderDataIt->armatureIndex);
                        if (result.value != FOE_SUCCESS)
                            return result;
                    }
                } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
                    // Some required resource is still loading
                    clearArmatureData(pRenderSystem->armatureData, renderDataIt->armatureIndex);
                    pRenderSystem->awaitingLoading.emplace_back(*renderDataIt);

                    removePositionData(pRenderSystem->positionData,
                                       renderDataIt - pRenderSystem->renderData.begin());

                    renderDataIt = pRenderSystem->renderData.erase(renderDataIt);
                }

                ++pRenderStateID;
            }
        }
    }

    { // Position3D modified
        size_t const entityListCount =
            foeEcsComponentPoolEntityListSize(pRenderSystem->positionPool);
        foeEcsEntityList const *pLists =
            foeEcsComponentPoolEntityLists(pRenderSystem->positionPool);

        for (size_t i = 0; i < entityListCount; ++i) {
            foeEcsEntityList entityList = pLists[i];

            foeEntityID const *pModifiedID = foeEcsEntityListPtr(entityList);
            foeEntityID const *const pEndModifiedID =
                pModifiedID + foeEcsEntityListSize(entityList);

            foeEntityID const *pPositionID = foeEcsComponentPoolIdPtr(pRenderSystem->positionPool);
            foeEntityID const *const pStartPositionID = pPositionID;
            foeEntityID const *const pEndPosititonID =
                pStartPositionID + foeEcsComponentPoolSize(pRenderSystem->positionPool);
            foePosition3d *const *const ppStartPositionData =
                (foePosition3d *const *const)foeEcsComponentPoolDataPtr(
                    pRenderSystem->positionPool);

            auto renderDataIt = pRenderSystem->renderData.begin();
            auto const endRenderDataIt = pRenderSystem->renderData.end();

            for (; pModifiedID != pEndModifiedID; ++pModifiedID) {
                // Check if the position component still exists
                pPositionID = std::lower_bound(pPositionID, pEndPosititonID, *pModifiedID);
                if (pPositionID == pEndPosititonID)
                    break;
                if (*pModifiedID != *pPositionID)
                    continue;

                // Check if there is associated render data
                renderDataIt =
                    std::lower_bound(renderDataIt, pRenderSystem->renderData.end(), *pModifiedID,
                                     [](RenderDataSet const &obj, foeEntityID const entity) {
                                         return obj.entity < entity;
                                     });
                if (renderDataIt == endRenderDataIt)
                    break;
                if (*pModifiedID != renderDataIt->entity)
                    continue;

                size_t const index = renderDataIt - pRenderSystem->renderData.begin();

                uint8_t *pData = (uint8_t *)pRenderSystem->positionData.pCpuBuffer;
                pData += index * pRenderSystem->positionData.alignment;

                glm::mat4 *pMatrix = (glm::mat4 *)pData;
                foePosition3d const *pPositionData =
                    *(ppStartPositionData + (pPositionID - pStartPositionID));

                *pMatrix = glm::mat4_cast(pPositionData->orientation) *
                           glm::translate(glm::mat4(1.f), pPositionData->position);
            }
        }
    }

    { // Position insertions
        foeEntityID const *pStartPositionID = foeEcsComponentPoolIdPtr(pRenderSystem->positionPool);
        foePosition3d const *const *const ppStartPositionData =
            (foePosition3d const *const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->positionPool);

        size_t const *pPositionOffset =
            foeEcsComponentPoolInsertedOffsetPtr(pRenderSystem->positionPool);
        size_t const *const pEndPositionOffset =
            pPositionOffset + foeEcsComponentPoolInserted(pRenderSystem->positionPool);

        foeEntityID const *pRenderStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->renderStatePool);
        foeEntityID const *const pStartRenderStateID = pRenderStateID;
        foeEntityID const *const pEndRenderStateID =
            pRenderStateID + foeEcsComponentPoolSize(pRenderSystem->renderStatePool);
        foeRenderState const *pStartRenderStateData =
            (foeRenderState const *)foeEcsComponentPoolDataPtr(pRenderSystem->renderStatePool);

        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->animatedBoneStatePool);
        foeEntityID const *const pStartAnimatedBoneStateID = pAnimatedBoneStateID;
        foeEntityID const *const pEndAnimatedBoneStateID =
            pStartAnimatedBoneStateID +
            foeEcsComponentPoolSize(pRenderSystem->animatedBoneStatePool);
        foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
            (foeAnimatedBoneState const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->animatedBoneStatePool);

        auto renderDataIt = pRenderSystem->renderData.begin();

        for (; pPositionOffset != pEndPositionOffset; ++pPositionOffset) {
            foeEntityID const entity = *(pStartPositionID + *pPositionOffset);

            // Make sure the associated position component exists
            pRenderStateID = std::lower_bound(pRenderStateID, pEndRenderStateID, entity);
            if (pRenderStateID == pEndRenderStateID)
                break;
            if (*pRenderStateID != entity)
                continue;

            foeRenderState const *pRenderStateData =
                pStartRenderStateData + (pRenderStateID - pStartRenderStateID);

            // Check to see if the render data already exists for it
            renderDataIt = std::lower_bound(renderDataIt, pRenderSystem->renderData.end(), entity,
                                            [](RenderDataSet const &obj, foeEntityID const entity) {
                                                return obj.entity < entity;
                                            });
            if (renderDataIt != pRenderSystem->renderData.end() && renderDataIt->entity == entity)
                // Render data already in, continue
                continue;

            RenderDataSet newDataSet{.entity = entity, .armatureIndex = UINT32_MAX};

            foeResourceStateFlags resourceState =
                getRenderData(pRenderSystem->resourcePool, pRenderStateData, &newDataSet.resources);

            assert(resourceState & (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                                    FOE_RESOURCE_STATE_LOADED_BIT));

            if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
                // Some required resource failed to load
                clearArmatureData(pRenderSystem->armatureData, newDataSet.armatureIndex);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
                // Check if we also have armature data available
                pAnimatedBoneStateID =
                    std::lower_bound(pAnimatedBoneStateID, pEndAnimatedBoneStateID, entity);
                if (pAnimatedBoneStateID != pEndAnimatedBoneStateID &&
                    *pAnimatedBoneStateID == entity) {
                    result = getArmatureData(pRenderSystem->armatureData,
                                             pStartAnimatedBoneStateData +
                                                 (pAnimatedBoneStateID - pStartAnimatedBoneStateID),
                                             newDataSet.resources.mesh, newDataSet.armatureIndex);
                    if (result.value != FOE_SUCCESS)
                        return result;
                }

                size_t const posIndex = renderDataIt - pRenderSystem->renderData.begin();
                foePosition3d const *const pPositionData =
                    *(ppStartPositionData + *pPositionOffset);
                result = insertPositionData(pRenderSystem->positionData, posIndex, pPositionData);
                if (result.value != FOE_SUCCESS)
                    return result;

                renderDataIt = pRenderSystem->renderData.insert(renderDataIt, newDataSet);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
                // Some required resource is still loading
                pRenderSystem->awaitingLoading.emplace_back(newDataSet);
            }
        }
    }

    { // RenderState insertions
        foeEntityID const *pStartRenderStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->renderStatePool);
        foeRenderState const *pStartRenderStateData =
            (foeRenderState const *)foeEcsComponentPoolDataPtr(pRenderSystem->renderStatePool);

        size_t const *pRenderStateOffset =
            foeEcsComponentPoolInsertedOffsetPtr(pRenderSystem->renderStatePool);
        size_t const *const pEndRenderStateOffset =
            pRenderStateOffset + foeEcsComponentPoolInserted(pRenderSystem->renderStatePool);

        foeEntityID const *pPositionID = foeEcsComponentPoolIdPtr(pRenderSystem->positionPool);
        foeEntityID const *pStartPositionID = pPositionID;
        foeEntityID const *const pEndPositionID =
            pStartPositionID + foeEcsComponentPoolSize(pRenderSystem->positionPool);
        foePosition3d const *const *const ppStartPositionData =
            (foePosition3d const *const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->positionPool);

        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->animatedBoneStatePool);
        foeEntityID const *const pStartAnimatedBoneStateID = pAnimatedBoneStateID;
        foeEntityID const *const pEndAnimatedBoneStateID =
            pStartAnimatedBoneStateID +
            foeEcsComponentPoolSize(pRenderSystem->animatedBoneStatePool);
        foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
            (foeAnimatedBoneState const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->animatedBoneStatePool);

        auto renderDataIt = pRenderSystem->renderData.begin();

        for (; pRenderStateOffset != pEndRenderStateOffset; ++pRenderStateOffset) {
            foeEntityID const entity = *(pStartRenderStateID + *pRenderStateOffset);
            foeRenderState const *pRenderStateData = pStartRenderStateData + *pRenderStateOffset;

            // Make sure the associated position component exists
            pPositionID = std::lower_bound(pPositionID, pEndPositionID, entity);
            if (pPositionID == pEndPositionID)
                break;
            if (*pPositionID != entity)
                continue;

            // Check to see if the render data already exists for it
            renderDataIt = std::lower_bound(renderDataIt, pRenderSystem->renderData.end(), entity,
                                            [](RenderDataSet const &obj, foeEntityID const entity) {
                                                return obj.entity < entity;
                                            });
            if (renderDataIt != pRenderSystem->renderData.end() && renderDataIt->entity == entity)
                // Render data already in, continue
                continue;

            RenderDataSet newDataSet{.entity = entity, .armatureIndex = UINT32_MAX};

            foeResourceStateFlags resourceState =
                getRenderData(pRenderSystem->resourcePool, pRenderStateData, &newDataSet.resources);

            assert(resourceState & (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                                    FOE_RESOURCE_STATE_LOADED_BIT));

            if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
                // Some required resource failed to load
                clearArmatureData(pRenderSystem->armatureData, newDataSet.armatureIndex);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
                // Check if we also have armature data available
                pAnimatedBoneStateID =
                    std::lower_bound(pAnimatedBoneStateID, pEndAnimatedBoneStateID, entity);
                if (pAnimatedBoneStateID != pEndAnimatedBoneStateID &&
                    *pAnimatedBoneStateID == entity) {
                    result = getArmatureData(pRenderSystem->armatureData,
                                             pStartAnimatedBoneStateData +
                                                 (pAnimatedBoneStateID - pStartAnimatedBoneStateID),
                                             newDataSet.resources.mesh, newDataSet.armatureIndex);
                    if (result.value != FOE_SUCCESS)
                        return result;
                }

                size_t const posIndex = renderDataIt - pRenderSystem->renderData.begin();
                foePosition3d const *const pPositionData =
                    *(ppStartPositionData + (pPositionID - pStartPositionID));
                result = insertPositionData(pRenderSystem->positionData, posIndex, pPositionData);
                if (result.value != FOE_SUCCESS)
                    return result;

                renderDataIt = pRenderSystem->renderData.insert(renderDataIt, newDataSet);
            } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
                // Some required resource is still loading
                pRenderSystem->awaitingLoading.emplace_back(newDataSet);
            }
        }
    }

    { // AnimatedBoneState Insertions
        foeEntityID const *const pStartAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->animatedBoneStatePool);
        foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
            (foeAnimatedBoneState const *)foeEcsComponentPoolDataPtr(
                pRenderSystem->animatedBoneStatePool);

        size_t const *pOffset =
            foeEcsComponentPoolInsertedOffsetPtr(pRenderSystem->animatedBoneStatePool);
        size_t const *const pEndOffset =
            pOffset + foeEcsComponentPoolInserted(pRenderSystem->animatedBoneStatePool);

        auto renderDataIt = pRenderSystem->renderData.begin();
        auto const endRenderDataIt = pRenderSystem->renderData.end();

        while (pOffset != pEndOffset) {
            foeEntityID entity = pStartAnimatedBoneStateID[*pOffset];
            foeAnimatedBoneState const *const pAnimatedBoneStateData =
                pStartAnimatedBoneStateData + *pOffset;

            renderDataIt = std::lower_bound(renderDataIt, endRenderDataIt, entity,
                                            [](RenderDataSet const &obj, foeEntityID const entity) {
                                                return obj.entity < entity;
                                            });

            if (renderDataIt == endRenderDataIt)
                // Reached the end of render data, nothing left to do
                break;

            if (renderDataIt->entity == entity) {
                result = getArmatureData(pRenderSystem->armatureData, pAnimatedBoneStateData,
                                         renderDataIt->resources.mesh, renderDataIt->armatureIndex);
                if (result.value != FOE_SUCCESS)
                    return result;
            }

            ++pOffset;
        }
    }

    { // Armature CPU Buffer
        foeEntityID const *pAnimatedBoneStateID =
            foeEcsComponentPoolIdPtr(pRenderSystem->animatedBoneStatePool);
        foeEntityID const *const pStartAnimatedBoneStateID = pAnimatedBoneStateID;
        foeEntityID const *const pEndAnimatedBoneStateID =
            pStartAnimatedBoneStateID +
            foeEcsComponentPoolSize(pRenderSystem->animatedBoneStatePool);
        foeAnimatedBoneState const *const pStartAnimatedBoneStateData =
            (foeAnimatedBoneState const *const)foeEcsComponentPoolDataPtr(
                pRenderSystem->animatedBoneStatePool);

        auto renderDataIt = pRenderSystem->renderData.begin();
        auto const endRenderDataIt = pRenderSystem->renderData.end();

        for (; renderDataIt != endRenderDataIt; ++renderDataIt) {
            if (renderDataIt->armatureIndex == UINT32_MAX)
                continue;

            RenderSystemArmatureData::ArmatureBoneAlloc *pArmatureEntry =
                pRenderSystem->armatureData.pArmatureAllocations + renderDataIt->armatureIndex;

            glm::mat4 *pBufferData =
                (glm::mat4 *)((uint8_t *)pRenderSystem->armatureData.pBoneData +
                              pArmatureEntry->offset);

            pAnimatedBoneStateID = std::lower_bound(pAnimatedBoneStateID, pEndAnimatedBoneStateID,
                                                    renderDataIt->entity);
            foeAnimatedBoneState const *pAnimatedBoneStateData =
                pStartAnimatedBoneStateData + (pAnimatedBoneStateID - pStartAnimatedBoneStateID);

            foeMesh const *pMesh =
                (foeMesh const *)foeResourceGetData(renderDataIt->resources.mesh);

            // Get Armature Resource
            foeArmature const *pArmature =
                (foeArmature const *)foeResourceGetData(pAnimatedBoneStateData->armature);

            glm::mat4 lastBone = glm::mat4{1.f};
            for (auto const &bone : pMesh->gfxBones) {
                // Find the matching armature node, if it exists
                foeArmatureNode const *pArmatureNode{nullptr};
                size_t armatureNodeIndex;
                for (size_t i = 0; i < pArmature->armature.size(); ++i) {
                    if (pArmature->armature[i].name == bone.name) {
                        pArmatureNode = &pArmature->armature[i];
                        armatureNodeIndex = i;
                        break;
                    }
                }

                if (pArmatureNode == nullptr) {
                    // Didn't find a matching node,
                    lastBone = glm::mat4{1.f};
                } else {
                    lastBone = pAnimatedBoneStateData->pBones[armatureNodeIndex];
                }

                *pBufferData = lastBone * bone.offsetMatrix;
                ++pBufferData;
            }
        }
    }

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

foeResultSet foeProcessRenderSystemGraphics(foeRenderSystem renderSystem, uint32_t frameIndex) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    result =
        preparePositionGpuData(pRenderSystem->positionData, pRenderSystem->gfxSession, frameIndex);
    if (result.value != FOE_SUCCESS)
        return result;

    result =
        prepareArmatureGpuData(pRenderSystem->armatureData, pRenderSystem->gfxSession, frameIndex);
    if (result.value != FOE_SUCCESS)
        return result;

    return result;
}

std::vector<RenderDataSet> const &getRenderDataSets(foeRenderSystem renderSystem) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);

    return pRenderSystem->renderData;
}

VkDescriptorSet const *getPositionDescriptorSets(foeRenderSystem renderSystem,
                                                 uint32_t frameIndex) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);

    return pRenderSystem->positionData.gpuData[frameIndex].pDescriptorSets;
}

VkDescriptorSet const *getArmatureDescriptorSets(foeRenderSystem renderSystem,
                                                 uint32_t frameIndex) {
    RenderSystem *pRenderSystem = render_system_from_handle(renderSystem);

    return pRenderSystem->armatureData.armatureGpuData[frameIndex].pDescriptorSets;
}