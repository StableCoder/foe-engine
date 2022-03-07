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

#include <foe/graphics/resource/mesh_loader.hpp>

#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/mesh.hpp>
#include <foe/graphics/vk/model.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/model/assimp/importer.hpp>
#include <foe/model/cube.hpp>
#include <foe/model/ico_sphere.hpp>
#include <vk_error_code.hpp>
#include <vulkan/vulkan.h>

#include "error_code.hpp"
#include "log.hpp"

auto foeMeshLoader::initialize(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn)
    -> std::error_code {
    if (!externalFileSearchFn)
        return FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_INITIALIZATION_FAILED;

    mExternalFileSearchFn = externalFileSearchFn;

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

void foeMeshLoader::deinitialize() { mExternalFileSearchFn = {}; }

bool foeMeshLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

auto foeMeshLoader::initializeGraphics(foeGfxSession gfxSession) -> std::error_code {
    if (!initialized())
        return FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_NOT_INITIALIZED;

    // External
    mGfxSession = gfxSession;

    // Internal
    std::error_code errC = foeGfxCreateUploadContext(gfxSession, &mGfxUploadContext);
    if (errC)
        deinitializeGraphics();

    return errC;
}

void foeMeshLoader::deinitializeGraphics() {
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
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }

    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);

    for (auto const &it : toDestroy) {
        if (it.gfxData != FOE_NULL_HANDLE)
            foeGfxDestroyMesh(mGfxSession, it.gfxData);
    }

    // Unloads
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.pResource, it.iteration, true);
        it.pResource->decrementRefCount();
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
            if (it.pResource != nullptr) {
                it.pResource->modifySync.lock();

                if (it.pResource->data.pUnloadFn != nullptr) {
                    it.pResource->data.pUnloadFn(it.pResource->data.pUnloadContext, it.pResource,
                                                 it.pResource->iteration, true);
                }

                ++it.pResource->iteration;
                it.pResource->data = std::move(it.data);
                it.pPostLoadFn(it.pResource, {});

                it.pResource->modifySync.unlock();
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

            it.pPostLoadFn(it.pResource, FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED);
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

bool foeMeshLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeMeshCreateInfo *>(pCreateInfo) != nullptr;
}

void foeMeshLoader::load(void *pLoader,
                         void *pResource,
                         std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                         void (*pPostLoadFn)(void *, std::error_code)) {
    reinterpret_cast<foeMeshLoader *>(pLoader)->load(pResource, pCreateInfo, pPostLoadFn);
}

void foeMeshLoader::load(void *pResource,
                         const std::shared_ptr<foeResourceCreateInfoBase> &pCreateInfo,
                         void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pMesh = reinterpret_cast<foeMesh *>(pResource);
    auto *pMeshCI = dynamic_cast<foeMeshCreateInfo *>(pCreateInfo.get());

    if (pMeshCI == nullptr) {
        pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO);
        return;
    }

    std::error_code errC;
    VkResult vkRes{VK_SUCCESS};

    foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
    foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
    foeMesh::Data data{
        .pUnloadContext = this,
        .pUnloadFn = unloadResource,
        .pCreateInfo = pCreateInfo,
    };

    if (auto pCI = dynamic_cast<foeMeshFileSource *>(pMeshCI->source.get()); pCI) {
        std::filesystem::path filePath = mExternalFileSearchFn(pCI->fileName);

        auto modelLoader = std::make_unique<foeModelAssimpImporter>(filePath.string().c_str(),
                                                                    pCI->postProcessFlags);
        assert(modelLoader->loaded());

        unsigned int meshIndex{UINT32_MAX};
        for (unsigned int i = 0; i < modelLoader->getNumMeshes(); ++i) {
            if (modelLoader->getMeshName(i) == pCI->meshName) {
                meshIndex = i;
                break;
            }
        }
        if (meshIndex == UINT32_MAX) {
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
        errC = foeGfxVkCreateMesh(mGfxSession, vertexDataSize + vertexWeightDataSize,
                                  static_cast<uint32_t>(indexData.size()) * sizeof(uint32_t),
                                  static_cast<uint32_t>(indexData.size()), VK_INDEX_TYPE_UINT32,
                                  vertexDataSize, &hostVisible, &data.gfxData);
        if (errC) {
            goto LOAD_FAILED;
        }
        auto [vertexBuffer, vertexAlloc] = foeGfxVkGetMeshVertexData(data.gfxData);
        auto [indexBuffer, indexAlloc] = foeGfxVkGetMeshIndexData(data.gfxData);

        if (!hostVisible) {
            errC = foeGfxCreateUploadBuffer(mGfxUploadContext,
                                            vertexDataSize + vertexWeightDataSize +
                                                (indexData.size() * sizeof(uint32_t)),
                                            &uploadBuffer);
            if (errC) {
                goto LOAD_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        errC = mapModelBuffers(foeGfxVkGetAllocator(mGfxSession),
                               vertexDataSize + vertexWeightDataSize, vertexAlloc, indexAlloc,
                               mGfxUploadContext, uploadBuffer, &pVertexData, &pIndexData);
        if (errC) {
            goto LOAD_FAILED;
        }

        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        if (vertexWeightDataSize != 0) { // Only copy weight/bone data if it exists
            memcpy(reinterpret_cast<uint8_t *>(pVertexData) + vertexWeightDataSize,
                   weightData.get(), vertexWeightDataSize);
        }
        memcpy(pIndexData, indexData.data(), indexData.size() * sizeof(uint32_t));

        unmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                          mGfxUploadContext, uploadBuffer);

        vkRes = recordModelUploadCommands(
            mGfxUploadContext, vertexBuffer, vertexDataSize + vertexWeightDataSize, indexBuffer,
            indexData.size() * sizeof(uint32_t), uploadBuffer, &uploadRequest);
        if (vkRes != VK_SUCCESS) {
            goto LOAD_FAILED;
        }

        errC = foeSubmitUploadDataCommands(mGfxUploadContext, uploadRequest);
        if (errC) {
            goto LOAD_FAILED;
        }

        data.perVertexBoneWeights = perVertexBoneWeights;
        data.gfxBones = std::move(meshBones);
        data.gfxVertexComponent = components;
    } else if (auto pCI = dynamic_cast<foeMeshCubeSource *>(pMeshCI->source.get()); pCI) {
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

        errC = foeGfxVkCreateMesh(mGfxSession, vertexDataSize, indexDataSize, indexData.size(),
                                  VK_INDEX_TYPE_UINT16, 0, &hostVisible, &data.gfxData);
        if (errC) {
            goto LOAD_FAILED;
        }

        auto [vertexBuffer, vertexAlloc] = foeGfxVkGetMeshVertexData(data.gfxData);
        auto [indexBuffer, indexAlloc] = foeGfxVkGetMeshIndexData(data.gfxData);

        if (!hostVisible) {
            errC = foeGfxCreateUploadBuffer(mGfxUploadContext, vertexDataSize + indexDataSize,
                                            &uploadBuffer);
            if (errC) {
                goto LOAD_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        errC =
            mapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexDataSize, vertexAlloc,
                            indexAlloc, mGfxUploadContext, uploadBuffer, &pVertexData, &pIndexData);
        if (errC) {
            goto LOAD_FAILED;
        }
        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        memcpy(pIndexData, indexData.data(), indexDataSize);

        unmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                          mGfxUploadContext, uploadBuffer);

        vkRes = recordModelUploadCommands(mGfxUploadContext, vertexBuffer, vertexDataSize,
                                          indexBuffer, indexDataSize, uploadBuffer, &uploadRequest);
        if (vkRes != VK_SUCCESS) {
            goto LOAD_FAILED;
        }

        errC = foeSubmitUploadDataCommands(mGfxUploadContext, uploadRequest);
        if (errC) {
            goto LOAD_FAILED;
        }

        data.perVertexBoneWeights = 0;
        data.gfxVertexComponent = std::move(components);
    } else if (auto pCI = dynamic_cast<foeMeshIcosphereSource *>(pMeshCI->source.get()); pCI) {
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

        errC = foeGfxVkCreateMesh(mGfxSession, vertexDataSize, indexDataSize,
                                  static_cast<uint32_t>(indexData.size()), VK_INDEX_TYPE_UINT16, 0,
                                  &hostVisible, &data.gfxData);
        if (errC) {
            goto LOAD_FAILED;
        }

        auto [vertexBuffer, vertexAlloc] = foeGfxVkGetMeshVertexData(data.gfxData);
        auto [indexBuffer, indexAlloc] = foeGfxVkGetMeshIndexData(data.gfxData);

        if (!hostVisible) {
            errC = foeGfxCreateUploadBuffer(mGfxUploadContext, vertexDataSize + indexDataSize,
                                            &uploadBuffer);
            if (errC) {
                goto LOAD_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        errC =
            mapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexDataSize, vertexAlloc,
                            indexAlloc, mGfxUploadContext, uploadBuffer, &pVertexData, &pIndexData);
        if (errC) {
            goto LOAD_FAILED;
        }
        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        memcpy(pIndexData, indexData.data(), indexDataSize);

        unmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                          mGfxUploadContext, uploadBuffer);

        vkRes = recordModelUploadCommands(mGfxUploadContext, vertexBuffer, vertexDataSize,
                                          indexBuffer, indexDataSize, uploadBuffer, &uploadRequest);
        if (vkRes != VK_SUCCESS) {
            goto LOAD_FAILED;
        }

        errC = foeSubmitUploadDataCommands(mGfxUploadContext, uploadRequest);
        if (vkRes) {
            goto LOAD_FAILED;
        }

        data.perVertexBoneWeights = 0;
        data.gfxVertexComponent = std::move(components);
    }

LOAD_FAILED:
    if (!errC && vkRes != VK_SUCCESS)
        errC = vkRes;

    if (errC) {
        // Failed at some point
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeMesh {} with error {}:{}",
                foeIdToString(pMesh->getID()), errC.value(), errC.message())
        pPostLoadFn(pMesh, errC);

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
            .pResource = pMesh,
            .pPostLoadFn = pPostLoadFn,
            .data = std::move(data),
            .uploadRequest = uploadRequest,
            .uploadBuffer = uploadBuffer,
        });

        mLoadSync.unlock();
    }
}

void foeMeshLoader::unloadResource(void *pContext,
                                   void *pResource,
                                   uint32_t resourceIteration,
                                   bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeMeshLoader *>(pContext);
    auto *pMesh = reinterpret_cast<foeMesh *>(pResource);

    if (immediateUnload) {
        pMesh->modifySync.lock();

        if (pMesh->iteration == resourceIteration) {
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(
                std::move(pMesh->data));

            pMesh->data = {};
            pMesh->state = foeResourceState::Unloaded;
            ++pMesh->iteration;
        }

        pMesh->modifySync.unlock();
    } else {
        pMesh->incrementRefCount();
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pResource = pMesh,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadSync.unlock();
    }
}