// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh_loader.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/mesh.h>
#include <foe/graphics/vk/model.h>
#include <foe/graphics/vk/session.h>
#include <foe/model/assimp/importer.hpp>
#include <foe/model/cube.hpp>
#include <foe/model/ico_sphere.hpp>
#include <vulkan/vulkan.h>

#include "log.hpp"
#include "result.h"

foeResultSet foeMeshLoader::initialize(
    foeResourcePool resourcePool,
    std::function<foeResultSet(char const *, foeManagedMemory *)> externalFileSearchFn) {
    if (resourcePool == FOE_NULL_HANDLE || !externalFileSearchFn)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_INITIALIZATION_FAILED);

    mResourcePool = resourcePool;
    mExternalFileSearchFn = externalFileSearchFn;

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeMeshLoader::deinitialize() {
    mExternalFileSearchFn = {};
    mResourcePool = FOE_NULL_HANDLE;
}

bool foeMeshLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

foeResultSet foeMeshLoader::initializeGraphics(foeGfxSession gfxSession) {
    if (!initialized())
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_NOT_INITIALIZED);

    // External
    mGfxSession = gfxSession;

    // Internal
    foeResultSet result = foeGfxCreateUploadContext(gfxSession, &mGfxUploadContext);
    if (result.value != FOE_SUCCESS)
        deinitializeGraphics();

    return result;
}

void foeMeshLoader::deinitializeGraphics() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork =
            foeResourcePoolUnloadType(mResourcePool, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH) > 0;

        gfxMaintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();

        mDestroySync.lock();
        for (auto const &it : mDataDestroyLists) {
            upcomingWork |= !it.empty();
        }
        mDestroySync.unlock();
    } while (upcomingWork);

    // Internal
    if (mGfxUploadContext != FOE_NULL_HANDLE)
        foeGfxDestroyUploadContext(mGfxUploadContext);
    mGfxUploadContext = FOE_NULL_HANDLE;

    // External
    mGfxSession = FOE_NULL_HANDLE;
}

bool foeMeshLoader::initializedGraphics() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

void foeMeshLoader::gfxMaintenance() {
    // Delayed Destruction
    mDestroySync.lock();
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }
    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);
    mDestroySync.unlock();

    for (auto const &it : toDestroy) {
        if (it.gfxData != FOE_NULL_HANDLE)
            foeGfxDestroyMesh(mGfxSession, it.gfxData);
    }

    // Unloads
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Loads
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        auto requestStatus = foeGfxGetUploadRequestStatus(it.uploadRequest);
        if (requestStatus == FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE) {
            if (it.uploadBuffer != FOE_NULL_HANDLE)
                foeGfxDestroyUploadBuffer(mGfxUploadContext, it.uploadBuffer);
            foeGfxDestroyUploadRequest(mGfxUploadContext, it.uploadRequest);

            // If we're uploading to something, swap the data in now
            if (it.resource != FOE_NULL_HANDLE) {
                auto moveFn = [](void *pSrc, void *pDst) {
                    auto *pSrcData = (foeMesh *)pSrc;

                    new (pDst) foeMesh(std::move(*pSrcData));
                };

                it.pPostLoadFn(it.resource, to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS), &it.data,
                               moveFn, this, foeMeshLoader::unloadResource);
            } else {
                // No target resource, this is to be discarded
                if (it.data.gfxData != FOE_NULL_HANDLE)
                    foeGfxDestroyMesh(mGfxSession, it.data.gfxData);
            }
        } else if (requestStatus != FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE) {
            // Some sort of faileure occurred during the upload
            if (it.uploadBuffer != FOE_NULL_HANDLE)
                foeGfxDestroyUploadBuffer(mGfxUploadContext, it.uploadBuffer);
            foeGfxDestroyUploadRequest(mGfxUploadContext, it.uploadRequest);

            it.pPostLoadFn(it.resource,
                           to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED), nullptr,
                           nullptr, nullptr, nullptr);
        } else {
            // Still uploading, requeue
            stillLoading.emplace_back(std::move(it));
        }
    }

    // If there's items still loading then requeue them
    if (!stillLoading.empty()) {
        mLoadSync.lock();

        mLoadRequests.reserve(mLoadRequests.size() + stillLoading.size());
        for (auto &it : stillLoading) {
            mLoadRequests.emplace_back(std::move(it));
        }

        mLoadSync.unlock();
    }
}

bool foeMeshLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    auto type = foeResourceCreateInfoGetType(createInfo);
    return type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO ||
           type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO ||
           type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO;
}

void foeMeshLoader::load(void *pLoader,
                         foeResource resource,
                         foeResourceCreateInfo createInfo,
                         PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeMeshLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeMeshLoader::load(foeResource resource,
                         foeResourceCreateInfo createInfo,
                         PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeMeshLoader - Cannot load {} as given CreateInfo is incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)),
                foeResourceCreateInfoGetType(createInfo));

        pPostLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                    nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    } else if (foeResourceGetType(resource) != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeMeshLoader - Cannot load {} as it is an incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)), foeResourceGetType(resource));

        pPostLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_RESOURCE_TYPE),
                    nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    }

    auto type = foeResourceCreateInfoGetType(createInfo);

    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
    foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
    foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
    foeManagedMemory managedMemory{FOE_NULL_HANDLE};
    foeMesh data{
        .rType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH,
    };

    if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        foeMeshFileCreateInfo const *pCI =
            (foeMeshFileCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
        result = mExternalFileSearchFn(pCI->pFile, &managedMemory);
        if (result.value != FOE_SUCCESS)
            goto LOAD_FAILED;

        void *pData;
        uint32_t dataSize;
        foeManagedMemoryGetData(managedMemory, &pData, &dataSize);

        auto modelLoader = std::make_unique<foeModelAssimpImporter>(pData, dataSize, pCI->pFile,
                                                                    pCI->postProcessFlags);
        if (!modelLoader->loaded()) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED);
            goto LOAD_FAILED;
        }

        unsigned int meshIndex{UINT32_MAX};
        for (unsigned int i = 0; i < modelLoader->getNumMeshes(); ++i) {
            if (modelLoader->getMeshName(i) == std::string_view{pCI->pMesh}) {
                meshIndex = i;
                break;
            }
        }
        if (meshIndex == UINT32_MAX) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED);
            goto LOAD_FAILED;
        }

        std::vector<foeVertexComponent> components{
            foeVertexComponent::Position, foeVertexComponent::Normal, foeVertexComponent::UV};

        std::vector<float> vertexData;
        uint32_t vertexDataSize = 0;
        uint32_t vertexWeightDataSize = 0;
        std::vector<uint32_t> indexData;
        std::unique_ptr<uint8_t[]> weightData;
        std::vector<foeMeshBone> meshBones;

        uint32_t numVertices = modelLoader->getNumMeshVertices(meshIndex);
        // Vertex Data
        vertexData.resize((foeGetVertexComponentStride(static_cast<uint32_t>(components.size()),
                                                       components.data()) *
                           numVertices) /
                          sizeof(float));
        modelLoader->importMeshVertexData(meshIndex, static_cast<uint32_t>(components.size()),
                                          components.data(), glm::mat4(1.f), vertexData.data());
        vertexDataSize = static_cast<uint32_t>(vertexData.size()) * sizeof(float);

        // Index Data
        indexData.resize(modelLoader->getNumFaces(meshIndex) * 3);
        modelLoader->importMeshIndexData32(meshIndex, 0, indexData.data());

        // Weights
        auto maxWeightsPerVertex = modelLoader->getMeshMaxPerVertexWeights(meshIndex);
        uint32_t perVertexBoneWeights = 4;
        if (maxWeightsPerVertex > 0) {
            auto verticesByWeight = modelLoader->getMeshVerticesByWeight(meshIndex);
            vertexWeightDataSize = modelLoader->getNumMeshVertices(meshIndex) *
                                   perVertexBoneWeights * (sizeof(float) + sizeof(uint32_t));
            weightData.reset(new uint8_t[vertexWeightDataSize]);
            modelLoader->importMeshVertexBoneWeights(meshIndex, perVertexBoneWeights,
                                                     weightData.get());

            // Bones
            meshBones = modelLoader->importMeshBones(meshIndex);
        }

        // Upload to graphics
        bool hostVisible; // If the original buffers are host visible, then no need for staging
        result = foeGfxVkCreateMesh(mGfxSession, vertexDataSize + vertexWeightDataSize,
                                    static_cast<uint32_t>(indexData.size()) * sizeof(uint32_t),
                                    static_cast<uint32_t>(indexData.size()), VK_INDEX_TYPE_UINT32,
                                    vertexDataSize, &hostVisible, &data.gfxData);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        VkBuffer vertexBuffer = VK_NULL_HANDLE, indexBuffer = VK_NULL_HANDLE;
        VmaAllocation vertexAlloc = VK_NULL_HANDLE, indexAlloc = VK_NULL_HANDLE;

        foeGfxVkGetMeshVertexData(data.gfxData, &vertexBuffer, &vertexAlloc);
        foeGfxVkGetMeshIndexData(data.gfxData, &indexBuffer, &indexAlloc);

        if (!hostVisible) {
            result = foeGfxCreateUploadBuffer(mGfxUploadContext,
                                              vertexDataSize + vertexWeightDataSize +
                                                  (indexData.size() * sizeof(uint32_t)),
                                              &uploadBuffer);
            if (result.value != FOE_SUCCESS) {
                goto LOAD_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        result = foeGfxVkMapModelBuffers(
            foeGfxVkGetAllocator(mGfxSession), vertexDataSize + vertexWeightDataSize, vertexAlloc,
            indexAlloc, mGfxUploadContext, uploadBuffer, &pVertexData, &pIndexData);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        if (vertexWeightDataSize != 0) { // Only copy weight/bone data if it exists
            memcpy(reinterpret_cast<uint8_t *>(pVertexData) + vertexWeightDataSize,
                   weightData.get(), vertexWeightDataSize);
        }
        memcpy(pIndexData, indexData.data(), indexData.size() * sizeof(uint32_t));

        foeGfxVkUnmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                                  mGfxUploadContext, uploadBuffer);

        result = foeGfxVkRecordModelUploadCommands(
            mGfxUploadContext, vertexBuffer, vertexDataSize + vertexWeightDataSize, indexBuffer,
            indexData.size() * sizeof(uint32_t), uploadBuffer, &uploadRequest);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        result = foeSubmitUploadDataCommands(mGfxUploadContext, uploadRequest);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        data.perVertexBoneWeights = perVertexBoneWeights;
        data.gfxBones = std::move(meshBones);
        data.gfxVertexComponent = components;
    } else if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO) {
        foeMeshCubeCreateInfo const *pCI =
            (foeMeshCubeCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
        std::vector<foeVertexComponent> components{
            foeVertexComponent::Position, foeVertexComponent::Normal, foeVertexComponent::UV};

        uint32_t vertexDataSize = foeModelCubeNumVertices() *
                                  foeGetVertexComponentStride(
                                      static_cast<uint32_t>(components.size()), components.data());
        std::vector<float> vertexData(vertexDataSize);
        vertexDataSize *= sizeof(float);

        foeModelCubeVertexData(static_cast<uint32_t>(components.size()), components.data(),
                               vertexData.data());

        uint32_t indexDataSize = foeModelCubeNumIndices();
        std::vector<uint16_t> indexData(indexDataSize);
        indexDataSize *= sizeof(uint16_t);

        foeModelCubeIndexData16(0, indexData.data());

        // Graphics Upload
        bool hostVisible; // If the original buffers are host visible, then no need for staging

        result = foeGfxVkCreateMesh(mGfxSession, vertexDataSize, indexDataSize, indexData.size(),
                                    VK_INDEX_TYPE_UINT16, 0, &hostVisible, &data.gfxData);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        VkBuffer vertexBuffer = VK_NULL_HANDLE, indexBuffer = VK_NULL_HANDLE;
        VmaAllocation vertexAlloc = VK_NULL_HANDLE, indexAlloc = VK_NULL_HANDLE;

        foeGfxVkGetMeshVertexData(data.gfxData, &vertexBuffer, &vertexAlloc);
        foeGfxVkGetMeshIndexData(data.gfxData, &indexBuffer, &indexAlloc);

        if (!hostVisible) {
            result = foeGfxCreateUploadBuffer(mGfxUploadContext, vertexDataSize + indexDataSize,
                                              &uploadBuffer);
            if (result.value != FOE_SUCCESS) {
                goto LOAD_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        result = foeGfxVkMapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexDataSize,
                                         vertexAlloc, indexAlloc, mGfxUploadContext, uploadBuffer,
                                         &pVertexData, &pIndexData);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }
        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        memcpy(pIndexData, indexData.data(), indexDataSize);

        foeGfxVkUnmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                                  mGfxUploadContext, uploadBuffer);

        result = foeGfxVkRecordModelUploadCommands(mGfxUploadContext, vertexBuffer, vertexDataSize,
                                                   indexBuffer, indexDataSize, uploadBuffer,
                                                   &uploadRequest);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        result = foeSubmitUploadDataCommands(mGfxUploadContext, uploadRequest);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        data.perVertexBoneWeights = 0;
        data.gfxVertexComponent = std::move(components);
    } else if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO) {
        foeMeshIcosphereCreateInfo const *pCI =
            (foeMeshIcosphereCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
        std::vector<foeVertexComponent> components{
            foeVertexComponent::Position, foeVertexComponent::Normal, foeVertexComponent::UV};

        int numVertices;
        int numIndices;
        foeModelIcoSphereNums(pCI->recursion, &numVertices, &numIndices);

        uint32_t vertexDataSize =
            numVertices * foeGetVertexComponentStride(static_cast<uint32_t>(components.size()),
                                                      components.data());
        std::vector<float> vertexData(vertexDataSize);
        vertexDataSize *= sizeof(float);

        foeModelIcoSphereVertexData(pCI->recursion, static_cast<uint32_t>(components.size()),
                                    components.data(), vertexData.data());

        uint32_t indexDataSize = numIndices;
        std::vector<uint16_t> indexData(indexDataSize);
        indexDataSize *= sizeof(uint16_t);

        foeModelIcoSphereIndexData16(pCI->recursion, 0, indexData.data());

        // Graphics Upload
        bool hostVisible; // If the original buffers are host visible, then no need for staging

        result = foeGfxVkCreateMesh(mGfxSession, vertexDataSize, indexDataSize,
                                    static_cast<uint32_t>(indexData.size()), VK_INDEX_TYPE_UINT16,
                                    0, &hostVisible, &data.gfxData);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        VkBuffer vertexBuffer = VK_NULL_HANDLE, indexBuffer = VK_NULL_HANDLE;
        VmaAllocation vertexAlloc = VK_NULL_HANDLE, indexAlloc = VK_NULL_HANDLE;

        foeGfxVkGetMeshVertexData(data.gfxData, &vertexBuffer, &vertexAlloc);
        foeGfxVkGetMeshIndexData(data.gfxData, &indexBuffer, &indexAlloc);

        if (!hostVisible) {
            result = foeGfxCreateUploadBuffer(mGfxUploadContext, vertexDataSize + indexDataSize,
                                              &uploadBuffer);
            if (result.value != FOE_SUCCESS) {
                goto LOAD_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        result = foeGfxVkMapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexDataSize,
                                         vertexAlloc, indexAlloc, mGfxUploadContext, uploadBuffer,
                                         &pVertexData, &pIndexData);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }
        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        memcpy(pIndexData, indexData.data(), indexDataSize);

        foeGfxVkUnmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                                  mGfxUploadContext, uploadBuffer);

        result = foeGfxVkRecordModelUploadCommands(mGfxUploadContext, vertexBuffer, vertexDataSize,
                                                   indexBuffer, indexDataSize, uploadBuffer,
                                                   &uploadRequest);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        result = foeSubmitUploadDataCommands(mGfxUploadContext, uploadRequest);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        data.perVertexBoneWeights = 0;
        data.gfxVertexComponent = std::move(components);
    }

LOAD_FAILED:
    foeResourceCreateInfoDecrementRefCount(createInfo);

    if (managedMemory != FOE_NULL_HANDLE)
        foeManagedMemoryDecrementUse(managedMemory);

    if (result.value != FOE_SUCCESS) {
        // Failed at some point
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to load foeMesh {} with error: {}",
                foeIdToString(foeResourceGetID(resource)), buffer)
        pPostLoadFn(resource, result, nullptr, nullptr, nullptr, nullptr);

        if (uploadRequest != FOE_NULL_HANDLE) {
            // A partial upload success, leave pMesh as nullptr, so the upload completes then
            // the data is discarded
            mLoadSync.lock();

            mLoadRequests.emplace_back(LoadData{
                .data = std::move(data),
                .uploadRequest = uploadRequest,
                .uploadBuffer = uploadBuffer,
            });

            mLoadSync.unlock();
        } else {
            // No target image, discard all data immediately
            if (data.gfxData != FOE_NULL_HANDLE)
                foeGfxDestroyMesh(mGfxSession, data.gfxData);
        }
    } else {
        // Succeeded
        mLoadSync.lock();

        mLoadRequests.emplace_back(LoadData{
            .resource = resource,
            .pPostLoadFn = pPostLoadFn,
            .data = std::move(data),
            .uploadRequest = uploadRequest,
            .uploadBuffer = uploadBuffer,
        });

        mLoadSync.unlock();
    }
}

void foeMeshLoader::unloadResource(void *pContext,
                                   foeResource resource,
                                   uint32_t resourceIteration,
                                   PFN_foeResourceUnloadCall *pUnloadCallFn,
                                   bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeMeshLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeMesh *)pSrc;
            auto *pDstData = (foeMesh *)pDst;

            *pDstData = std::move(*pSrcData);
            pSrcData->~foeMesh();
        };

        foeMesh data{};

        if (pUnloadCallFn(resource, resourceIteration, &data, moveFn)) {
            pLoader->mDestroySync.lock();
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(std::move(data));
            pLoader->mDestroySync.unlock();
        }
    } else {
        foeResourceIncrementRefCount(resource);
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}