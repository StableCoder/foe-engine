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

#ifndef FOE_GRAPHICS_RESOURCE_MESH_LOADER_HPP
#define FOE_GRAPHICS_RESOURCE_MESH_LOADER_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_buffer.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/upload_request.hpp>
#include <foe/simulation/core/create_info.hpp>

#include <array>
#include <filesystem>
#include <functional>
#include <vector>

struct foeMeshSource {
    virtual ~foeMeshSource() = default;
};

struct foeMeshFileSource : public foeMeshSource {
    std::string fileName;
    std::string meshName;
    unsigned int postProcessFlags;
};

struct foeMeshCubeSource : public foeMeshSource {};

struct foeMeshIcosphereSource : public foeMeshSource {
    int recursion;
};

struct foeMeshCreateInfo : public foeResourceCreateInfoBase {
    std::unique_ptr<foeMeshSource> source;
};

class FOE_GFX_RES_EXPORT foeMeshLoader {
  public:
    auto initialize(
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn)
        -> std::error_code;
    void deinitialize();
    bool initialized() const noexcept;

    auto initializeGraphics(foeGfxSession gfxSession) -> std::error_code;
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

    void gfxMaintenance();

    static bool canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo);
    static void load(void *pLoader,
                     void *pResource,
                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                     void (*pPostLoadFn)(void *, std::error_code));

  private:
    static void unloadResource(void *pContext,
                               void *pResource,
                               uint32_t resourceIteration,
                               bool immediateUnload);

    void load(void *pResource,
              std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
              void (*pPostLoadFn)(void *, std::error_code));

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};
    std::function<std::filesystem::path(std::filesystem::path)> mExternalFileSearchFn;

    foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    struct LoadData {
        foeMesh *pResource;
        void (*pPostLoadFn)(void *, std::error_code);
        foeMesh::Data data;

        foeGfxUploadRequest uploadRequest;
        foeGfxUploadBuffer uploadBuffer;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeMesh *pResource;
        uint32_t iteration;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;

    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeMesh::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mDataDestroyLists{};
};

#endif // FOE_GRAPHICS_RESOURCE_MESH_LOADER_HPP