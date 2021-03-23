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
#include <foe/resource/mesh.hpp>

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <system_error>
#include <vector>

struct foeMeshCreateInfo {
    virtual ~foeMeshCreateInfo() = default;
};

struct foeMeshFromFileCreateInfo : public foeMeshCreateInfo {
    std::string fileName;
    std::string meshName;
};

struct foeMeshGenerateCubeCreateInfo : public foeMeshCreateInfo {};

struct foeMeshGenerateIcosphereCreateInfo : public foeMeshCreateInfo {
    int recursion;
};

class foeMeshLoader {
  public:
    FOE_RES_EXPORT ~foeMeshLoader();

    FOE_RES_EXPORT std::error_code initialize(
        foeGfxSession session,
        std::function<bool(std::string_view, std::unique_ptr<foeMeshCreateInfo> &)> importFunction,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void processLoadRequests();
    FOE_RES_EXPORT void processUnloadRequests();

    FOE_RES_EXPORT void requestResourceLoad(foeMesh *pMesh);
    FOE_RES_EXPORT void requestResourceUnload(foeMesh *pMesh);

  private:
    struct MeshUpload {
        foeMesh *pMesh;
        foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
        foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
        foeMesh::Data data{};
    };

    void startUpload(foeMesh *pMesh);

    // If pMesh is nullptr, then once the upload is complete, the data is just discarded
    void processUpload(foeMesh *pMesh,
                       foeGfxUploadRequest uploadRequest,
                       foeGfxUploadBuffer uploadBuffer,
                       foeMesh::Data data);

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};
    foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    std::function<bool(std::string_view, std::unique_ptr<foeMeshCreateInfo> &)> mImportFunction;
    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;

    std::mutex mUploadingSync{};
    std::vector<MeshUpload> mUploadingData;
    std::atomic_int mActiveUploads;

    std::mutex mUnloadSync{};
    std::array<std::vector<foeMesh::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    std::vector<foeMesh::Data> *mCurrentUnloadRequests{&mUnloadRequestLists[0]};
};

#endif // FOE_RESOURCE_MESH_LOADER_HPP