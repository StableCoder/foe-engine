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

#ifndef FOE_RESOURCE_MESH_LOADER_HPP
#define FOE_RESOURCE_MESH_LOADER_HPP

#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_buffer.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/upload_request.hpp>
#include <foe/resource/export.h>
#include <foe/resource/loader_base.hpp>
#include <foe/resource/mesh.hpp>

#include <array>
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <system_error>
#include <vector>

class FOE_RES_EXPORT foeMeshLoader : public foeResourceLoaderBase {
  public:
    ~foeMeshLoader();

    std::error_code initialize(
        foeGfxSession session,
        std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
        std::function<void(std::function<void()>)> asynchronousJobs);
    void deinitialize();
    bool initialized() const noexcept;

    void processLoadRequests();
    void processUnloadRequests();

    void requestResourceLoad(foeMesh *pMesh);
    void requestResourceUnload(foeMesh *pMesh);

  private:
    FOE_RESOURCE_NO_EXPORT struct MeshUpload {
        foeMesh *pMesh;
        foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
        foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
        foeMesh::Data data{};
    };

    FOE_RESOURCE_NO_EXPORT void startUpload(foeMesh *pMesh);

    // If pMesh is nullptr, then once the upload is complete, the data is just discarded
    FOE_RESOURCE_NO_EXPORT void processUpload(foeMesh *pMesh,
                                              foeGfxUploadRequest uploadRequest,
                                              foeGfxUploadBuffer uploadBuffer,
                                              foeMesh::Data data);

    FOE_RESOURCE_NO_EXPORT foeGfxSession mGfxSession{FOE_NULL_HANDLE};
    FOE_RESOURCE_NO_EXPORT foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    FOE_RESOURCE_NO_EXPORT std::function<foeResourceCreateInfoBase *(foeId)> mImportFunction;
    FOE_RESOURCE_NO_EXPORT std::function<std::filesystem::path(std::filesystem::path)>
        mExternalFileSearchFn;
    FOE_RESOURCE_NO_EXPORT std::function<void(std::function<void()>)> mAsyncJobs;
    FOE_RESOURCE_NO_EXPORT std::atomic_int mActiveJobs;

    FOE_RESOURCE_NO_EXPORT std::mutex mUploadingSync{};
    FOE_RESOURCE_NO_EXPORT std::vector<MeshUpload> mUploadingData;
    FOE_RESOURCE_NO_EXPORT std::atomic_int mActiveUploads;

    FOE_RESOURCE_NO_EXPORT std::mutex mUnloadSync{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeMesh::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeMesh::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>::iterator
        mCurrentUnloadRequests{&mUnloadRequestLists[0]};
};

#endif // FOE_RESOURCE_MESH_LOADER_HPP