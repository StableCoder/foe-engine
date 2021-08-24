/*
    Copyright (C) 2020-2021 George Cave.

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

#ifndef FOE_WSI_WINDOW_HPP
#define FOE_WSI_WINDOW_HPP

#include <foe/handle.h>
#include <foe/wsi/export.h>

#include <system_error>

struct foeKeyboard;
struct foeMouse;

FOE_DEFINE_HANDLE(foeWsiWindow)

/** Creates a window for use
 * @param width Width of the new window
 * @param height Height of the new window
 * @param title The title for the new window
 * @param pWindow [out] Window handle will be put here on success
 * @return FOE_WSI_SUCCESS on success, an appropriate error code otherwise
 * @return True if initialization successful, false otherwise, typically due to the window already
 * being active
 */
FOE_WSI_EXPORT auto foeWsiCreateWindow(
    int width, int height, const char *pTitle, bool hide, foeWsiWindow *pWindow) -> std::error_code;

/// Destroys the current window
FOE_WSI_EXPORT void foeWsiDestroyWindow(foeWsiWindow window);

FOE_WSI_EXPORT const char *foeWsiWindowGetTitle(foeWsiWindow window);
FOE_WSI_EXPORT void foeWsiWindowSetTitle(foeWsiWindow window, const char *pTitle);

FOE_WSI_EXPORT bool foeWsiWindowGetShouldClose(foeWsiWindow window);
FOE_WSI_EXPORT void foeWsiWindowSetShouldClose(foeWsiWindow window, bool terminate);

FOE_WSI_EXPORT void foeWsiWindowGetSize(foeWsiWindow window, int *pWidth, int *pHeight);
FOE_WSI_EXPORT bool foeWsiWindowResized(foeWsiWindow window);
FOE_WSI_EXPORT void foeWsiWindowResize(foeWsiWindow window, int width, int height);

FOE_WSI_EXPORT void foeWsiWindowHide(foeWsiWindow window);
FOE_WSI_EXPORT void foeWsiWindowShow(foeWsiWindow window);

/** @brief
 * @param pScaleX X scaling factor requested for content
 * @param pScaleY Y scaling factor requested for content
 *
 * For macOS and Windows, content can be requested to be scaled independent of window sizing for
 * HiDPI screens.
 */
FOE_WSI_EXPORT void foeWsiWindowGetContentScale(foeWsiWindow window,
                                                float *pScaleX,
                                                float *pScaleY);

FOE_WSI_EXPORT const foeKeyboard *foeWsiGetKeyboard(foeWsiWindow window);
FOE_WSI_EXPORT const foeMouse *foeWsiGetMouse(foeWsiWindow window);

#endif // FOE_WSI_WINDOW_HPP