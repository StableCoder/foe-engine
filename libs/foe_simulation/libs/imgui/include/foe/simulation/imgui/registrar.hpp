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

#ifndef FOE_SIMULATION_IMGUI_REGISTRAR_HPP
#define FOE_SIMULATION_IMGUI_REGISTRAR_HPP

#include <foe/ecs/id.hpp>
#include <foe/simulation/imgui/export.h>

#include <mutex>
#include <system_error>
#include <vector>

struct foeSimulationState;
struct foeComponentPoolBase;
struct foeResourceLoaderBase;

/** @brief Object where arbitrary functions to display information via ImGui is registered
 *
 * In order to facilitate the ability to use plugins for functionality, that also brings the
 * unfortunate consequence that having the top-level application reference specific ImGui functions
 * for rendering elements becomes incredibly difficult.
 *
 * However, much like simulation functionality registration, the use of a common registrar for
 * registering ImGui render functionality for arbitrary object types can be used instead.
 *
 * This is the object to which ImGui element rendering can be registered and deregistered to, as
 * well as called to render elements that have been registered.
 */
class foeSimulationImGuiRegistrar {
  public:
    using ComponentFn = void (*)(foeEntityID, foeComponentPoolBase **, size_t);
    using ResourceFn = void (*)(foeResourceID, foeSimulationState const *);
    using LoaderFn = void (*)(foeResourceID, foeResourceLoaderBase **, size_t);

    /** @brief Registers a set of ImGui element rendering functions
     * @param componentFn Function for component types
     * @param resourceFn Function for various resource types
     * @param loaderFn Function for various loader types
     * @return An appropriate error code based on the success or failure of the call
     */
    FOE_SIM_IMGUI_EXPORT auto registerElements(ComponentFn componentFn,
                                               ResourceFn resourceFn,
                                               LoaderFn loaderFn) -> std::error_code;

    /** @brief Deregisters a set of ImGui element rendering functions
     * @param componentFn Function for component types
     * @param resourceFn Function for various resource types
     * @param loaderFn Function for various loader types
     * @return An appropriate error code based on the success or failure of the call
     */
    FOE_SIM_IMGUI_EXPORT auto deregisterElements(ComponentFn componentFn,
                                                 ResourceFn resourceFn,
                                                 LoaderFn loaderFn) -> std::error_code;

    /** @brief Attempts to render components associated with the entity
     * @param entity ID to find associated components to
     * @param ppPools List of pools to search for associated components
     * @param poolCount Number of pools passed in for ppPools
     */
    FOE_SIM_IMGUI_EXPORT void displayEntity(foeEntityID entity,
                                            foeComponentPoolBase **ppPools,
                                            size_t poolCount);

    /** @brief Attempts to render resources associated with the ID
     * @param resource ID to find associated components to
     * @param ppPools List of pools to search for associated components
     * @param poolCount Number of pools passed in for ppPools
     */
    FOE_SIM_IMGUI_EXPORT void displayResource(foeResourceID resource,
                                              foeSimulationState const *pSimulation);

  private:
    struct DisplayFns {
        ComponentFn componentFn;
        ResourceFn resourceFn;
        LoaderFn loaderFn;
    };

    bool matchFunctionList(ComponentFn componentFn,
                           ResourceFn resourceFn,
                           LoaderFn loaderFn,
                           DisplayFns const *pDisplayFns) const;

    std::mutex mSync;

    std::vector<DisplayFns> mFnLists;
};

#endif // FOE_SIMULATION_IMGUI_REGISTRAR_HPP