// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_IMGUI_REGISTRAR_HPP
#define FOE_SIMULATION_IMGUI_REGISTRAR_HPP

#include <foe/ecs/id.h>
#include <foe/resource/create_info.h>
#include <foe/resource/resource.h>
#include <foe/simulation/imgui/export.h>

#include <functional>
#include <mutex>
#include <vector>

struct foeSimulation;
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
    using ComponentFn = void (*)(foeEntityID, foeSimulation const *);
    using ResourceFn = void (*)(foeResourceBase const *);
    using ResourceCreateInfoFn = void (*)(foeResourceCreateInfo);
    using LoaderFn = void (*)(foeResourceID, foeResourceLoaderBase **, size_t);

    /** @brief Registers a set of ImGui element rendering functions
     * @param componentFn Function for component types
     * @param resourceFn Function for various resource types
     * @param resourceCreateInfoFn is the function that would be called to display
     * foeResourceCreateInfo types
     * @param loaderFn Function for various loader types
     * @return An appropriate error code based on the success or failure of the call
     */
    FOE_SIM_IMGUI_EXPORT
    foeResultSet registerElements(ComponentFn componentFn,
                                  ResourceFn resourceFn,
                                  ResourceCreateInfoFn resourceCreateInfoFn,
                                  LoaderFn loaderFn);

    /** @brief Deregisters a set of ImGui element rendering functions
     * @param componentFn Function for component types
     * @param resourceFn Function for various resource types
     * @param resourceCreateInfoFn is the function that would be called to display
     * foeResourceCreateInfo types
     * @param loaderFn Function for various loader types
     * @return An appropriate error code based on the success or failure of the call
     */
    FOE_SIM_IMGUI_EXPORT
    foeResultSet deregisterElements(ComponentFn componentFn,
                                    ResourceFn resourceFn,
                                    ResourceCreateInfoFn resourceCreateInfoFn,
                                    LoaderFn loaderFn);

    /** @brief Attempts to render components associated with the entity
     * @param entity ID to find associated components to
     * @param pSimulation is a pointer to the simulation to search for the entity
     */
    FOE_SIM_IMGUI_EXPORT
    void displayEntity(foeEntityID entity, foeSimulation const *pSimulation);

    /** @brief Attempts to render resources associated with the ID
     * @param pResourceData The resource data to be displayed
     */
    FOE_SIM_IMGUI_EXPORT
    void displayResource(foeResourceBase const *pResourceData);

    /** @brief Attempts to render resources associated with the ID
     * @param createInfo is the handle of the resource to attempt to display
     */
    FOE_SIM_IMGUI_EXPORT
    void displayResourceCreateInfo(foeResourceCreateInfo createInfo);

  private:
    struct DisplayFns {
        ComponentFn componentFn;
        ResourceFn resourceFn;
        ResourceCreateInfoFn resourceCreateInfoFn;
        LoaderFn loaderFn;
    };

    bool matchFunctionList(ComponentFn componentFn,
                           ResourceFn resourceFn,
                           ResourceCreateInfoFn resourceCreateInfoFn,
                           LoaderFn loaderFn,
                           DisplayFns const *pDisplayFns) const;

    /// Used to synchronize access to mFnLists
    std::recursive_mutex mSync;
    /// List of registered functions to display different types
    std::vector<DisplayFns> mFnLists;
};

#endif // FOE_SIMULATION_IMGUI_REGISTRAR_HPP