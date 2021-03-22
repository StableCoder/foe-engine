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

#include <foe/resource/mesh_loader.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeMeshLoader::~foeMeshLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeMeshLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
    if (mActiveUploads > 0) {
        FOE_LOG(foeResource, Fatal,
                "foeMeshLoader being destructed with {} active uploads in progress!",
                mActiveUploads.load());
    }
}

std::error_code foeMeshLoader::initialize(
    foeGfxSession session,
    std::function<bool(std::string_view, std::string &, std::string &)> importFunction,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mGfxSession = session;
    mImportFunction = importFunction;
    mAsyncJobs = asynchronousJobs;

    errC = foeGfxCreateUploadContext(session, &mGfxUploadContext);
    if (errC)
        goto INITIALIZATION_FAILED;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeMeshLoader::deinitialize() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeMeshLoader being deinitialized with {} active jobs!",
                mActiveJobs.load());
    }
    if (mActiveUploads > 0) {
        FOE_LOG(foeResource, Fatal,
                "foeMeshLoader being deinitialized with {} active uploads in progress!",
                mActiveUploads.load());
    }

    if (mGfxUploadContext != FOE_NULL_HANDLE)
        foeGfxDestroyUploadContext(mGfxUploadContext);

    mGfxSession = FOE_NULL_HANDLE;
}

bool foeMeshLoader::initialized() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

void foeMeshLoader::processLoadRequests() {
    mUploadingSync.lock();
    auto uploads = std::move(mUploadingData);
    mUploadingSync.unlock();

    for (auto &it : uploads) {
        processUpload(it.pMesh, it.uploadRequest, it.uploadBuffer, it.data);
    }
}

void foeMeshLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == &mUnloadRequestLists[mUnloadRequestLists.size()]) {
        mCurrentUnloadRequests = &mUnloadRequestLists[0];
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        // Nothing to do, compiles out
        if (data.gfxData != FOE_NULL_HANDLE)
            foeGfxDestroyMesh(mGfxSession, data.gfxData);
    }
}

void foeMeshLoader::requestResourceLoad(foeMesh *pMesh) {
    ++mActiveJobs;
    mAsyncJobs([this, pMesh] {
        startUpload(pMesh);
        --mActiveJobs;
    });
}

void foeMeshLoader::requestResourceUnload(foeMesh *pMesh) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pMesh->dataWriteLock};

    // Only unload if it's 'loaded' and useCount is zero
    if (pMesh->loadState == foeResourceLoadState::Loaded && pMesh->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(std::move(pMesh->data));

        pMesh->data = {};
        pMesh->loadState = foeResourceLoadState::Unloaded;
    }
}

#include <foe/graphics/vk/mesh.hpp>
#include <foe/graphics/vk/model.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/model/file_importer_plugins.hpp>
#include <vk_error_code.hpp>
#include <vulkan/vulkan.h>

void foeMeshLoader::startUpload(foeMesh *pMesh) {
    // First, try to enter the 'loading' state
    auto expected = pMesh->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pMesh->loadState.compare_exchange_weak(expected, foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeMesh {} in parrallel",
                static_cast<void *>(pMesh))
        return;
    }

    std::error_code errC;
    VkResult vkRes{VK_SUCCESS};

    std::string fileName;
    std::string meshName;

    foeGfxUploadRequest gfxUploadRequest;
    foeGfxUploadBuffer gfxUploadBuffer;
    foeMesh::Data meshData{};

    bool read = mImportFunction(pMesh->getName(), fileName, meshName);
    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    {
        auto modelImporterPlugin = foeModelLoadFileImporterPlugin(ASSIMP_PLUGIN_PATH);

        auto modelLoader = modelImporterPlugin->createImporter(fileName.c_str());
        assert(modelLoader->loaded());

        unsigned int meshIndex;
        {
            bool found = false;
            for (unsigned int i = 0; i < modelLoader->getNumMeshes(); ++i) {
                if (modelLoader->getMeshName(i) == meshName) {
                    meshIndex = i;
                    found = true;
                    break;
                }
            }
            if (!found) {
                goto LOADING_FAILED;
            }
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
        vertexData.resize(
            (foeGetVertexComponentStride(components.size(), components.data()) * numVertices) /
            sizeof(float));
        modelLoader->importMeshVertexData(meshIndex, components.size(), components.data(),
                                          glm::mat4(1.f), vertexData.data());
        vertexDataSize = vertexData.size() * sizeof(float);

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
        auto errC = foeGfxVkCreateMesh(mGfxSession, vertexDataSize + vertexWeightDataSize,
                                       indexData.size() * sizeof(uint32_t), indexData.size(),
                                       VK_INDEX_TYPE_UINT32, vertexDataSize, &hostVisible,
                                       &meshData.gfxData);
        if (errC) {
            goto LOADING_FAILED;
        }
        auto [vertexBuffer, vertexAlloc] = foeGfxVkGetMeshVertexData(meshData.gfxData);
        auto [indexBuffer, indexAlloc] = foeGfxVkGetMeshIndexData(meshData.gfxData);

        if (!hostVisible) {
            errC = foeGfxCreateUploadBuffer(mGfxUploadContext,
                                            vertexDataSize + vertexWeightDataSize +
                                                (indexData.size() * sizeof(uint32_t)),
                                            &gfxUploadBuffer);
            if (errC) {
                goto LOADING_FAILED;
            }
        }

        void *pVertexData;
        void *pIndexData;

        errC = mapModelBuffers(foeGfxVkGetAllocator(mGfxSession),
                               vertexDataSize + vertexWeightDataSize, vertexAlloc, indexAlloc,
                               mGfxUploadContext, gfxUploadBuffer, &pVertexData, &pIndexData);
        if (errC) {
            goto LOADING_FAILED;
        }

        memcpy(pVertexData, vertexData.data(), vertexDataSize);
        if (vertexWeightDataSize != 0) { // Only copy weight/bone data if it exists
            memcpy(reinterpret_cast<uint8_t *>(pVertexData) + vertexWeightDataSize,
                   weightData.get(), vertexWeightDataSize);
        }
        memcpy(pIndexData, indexData.data(), indexData.size() * sizeof(uint32_t));

        unmapModelBuffers(foeGfxVkGetAllocator(mGfxSession), vertexAlloc, indexAlloc,
                          mGfxUploadContext, gfxUploadBuffer);

        vkRes = recordModelUploadCommands(
            mGfxUploadContext, vertexBuffer, vertexDataSize + vertexWeightDataSize, indexBuffer,
            indexData.size() * sizeof(uint32_t), gfxUploadBuffer, &gfxUploadRequest);
        if (vkRes != VK_SUCCESS) {
            goto LOADING_FAILED;
        }

        errC = foeSubmitUploadDataCommands(mGfxUploadContext, gfxUploadRequest);
        if (errC) {
            goto LOADING_FAILED;
        }

        meshData.perVertexBoneWeights = perVertexBoneWeights;
        meshData.gfxBones = std::move(meshBones);
        meshData.gfxVertexComponent = components;

        modelImporterPlugin.release();
    }

LOADING_FAILED:
    if (!errC && vkRes != VK_SUCCESS) {
        errC = vkRes;
    }
    if (errC) {
        // Failed at some point, clear all relevant data
        FOE_LOG(foeResource, Error, "Failed to load foeImage {} with error {}:{}",
                static_cast<void *>(pMesh), errC.value(), errC.message())

        pMesh->loadState = foeResourceLoadState::Failed;

        // No longer using the reference, decrement.
        pMesh->decrementRefCount();

        if (gfxUploadRequest != FOE_NULL_HANDLE) {
            ++mActiveUploads;
            // A partial upload success, leave pMesh as nullptr, so the upload completes then the
            // data is discarded
            processUpload(nullptr, gfxUploadRequest, gfxUploadBuffer, meshData);
        } else {
            // No target image, discard all the data
            if (meshData.gfxData != FOE_NULL_HANDLE)
                foeGfxDestroyMesh(mGfxSession, meshData.gfxData);
        }

    } else {
        // Stash the data in a list while we wait for the upload to complete, to later be processed
        // by processUploadRequest
        ++mActiveUploads;

        processUpload(pMesh, gfxUploadRequest, gfxUploadBuffer, meshData);
    }
}

void foeMeshLoader::processUpload(foeMesh *pMesh,
                                  foeGfxUploadRequest uploadRequest,
                                  foeGfxUploadBuffer uploadBuffer,
                                  foeMesh::Data data) {
    auto requestStatus = foeGfxGetUploadRequestStatus(uploadRequest);
    if (requestStatus == FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE) {
        if (uploadBuffer != FOE_NULL_HANDLE)
            foeGfxDestroyUploadBuffer(mGfxUploadContext, uploadBuffer);
        foeGfxDestroyUploadRequest(mGfxUploadContext, uploadRequest);

        // It's done, swap the data in
        if (pMesh != nullptr) {
            std::scoped_lock writeLock{pMesh->dataWriteLock};
            foeMesh::Data oldData = std::move(pMesh->data);
            pMesh->data = std::move(data);
            pMesh->loadState = foeResourceLoadState::Loaded;

            // If there was active old data that we just wrote over, send it to be unloaded
            {
                std::scoped_lock unloadLock{mUnloadSync};
                mCurrentUnloadRequests->emplace_back(std::move(oldData));
            }
        } else {
            // No target Mesh, discard all the data
            if (data.gfxData != FOE_NULL_HANDLE)
                foeGfxDestroyMesh(mGfxSession, data.gfxData);
        }

        --mActiveUploads;
    } else if (requestStatus != FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE) {
        if (uploadBuffer != FOE_NULL_HANDLE)
            foeGfxDestroyUploadBuffer(mGfxUploadContext, uploadBuffer);
        foeGfxDestroyUploadRequest(mGfxUploadContext, uploadRequest);

        // There was an error, this is lost
        if (pMesh != nullptr) {
            pMesh->loadState = foeResourceLoadState::Failed;
        } else {
            // No target Mesh, discard all the data
            if (data.gfxData != FOE_NULL_HANDLE)
                foeGfxDestroyMesh(mGfxSession, data.gfxData);
        }

        --mActiveUploads;
    } else {
        // It's not yet complete, add to the uploading data list
        std::scoped_lock lock{mUploadingSync};

        mUploadingData.emplace_back(MeshUpload{
            .pMesh = pMesh,
            .uploadRequest = uploadRequest,
            .uploadBuffer = uploadBuffer,
            .data = data,
        });
    }
}