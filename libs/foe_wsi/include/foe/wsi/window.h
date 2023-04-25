// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_WSI_WINDOW_H
#define FOE_WSI_WINDOW_H

#include <foe/handle.h>
#include <foe/result.h>
#include <foe/wsi/export.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeWsiKeyboard;
struct foeWsiMouse;

FOE_DEFINE_HANDLE(foeWsiWindow)

/** @brief Performs any required global-WSI per-tick processing required
 * @warning Must be called after foeWsiWindowProcessing
 *
 * Some backend WSI implementations have some operations that must be performed across all windows
 * at once instead of individually, and as such, this call exists to perform those operations as
 * necessary.
 */
FOE_WSI_EXPORT
void foeWsiGlobalProcessing();

/** Creates a window for use
 * @param width Width of the new window
 * @param height Height of the new window
 * @param pTitle The title for the new window
 * @param visible Whether the window starts visible or not
 * @param pWindow [out] Window handle will be put here on success
 * @return FOE_WSI_SUCCESS on success, an appropriate error code otherwise
 */
FOE_WSI_EXPORT
foeResultSet foeWsiCreateWindow(
    int width, int height, char const *pTitle, bool visible, foeWsiWindow *pWindow);

/// Destroys the given window
FOE_WSI_EXPORT
void foeWsiDestroyWindow(foeWsiWindow window);

/** @brief Performs any required per-window per-tick processing required
 * @param window Window to process
 * @warning Must be called before foeWsiGlobalProcessing
 *
 * Some backend implementations run processing on a per-window basis each tick, and this function
 * would be used to run those as necessary.
 */
FOE_WSI_EXPORT
void foeWsiWindowProcessing(foeWsiWindow window);

FOE_WSI_EXPORT
char const *foeWsiWindowGetTitle(foeWsiWindow window);
FOE_WSI_EXPORT
void foeWsiWindowSetTitle(foeWsiWindow window, char const *pTitle);

FOE_WSI_EXPORT
bool foeWsiWindowGetShouldClose(foeWsiWindow window);
FOE_WSI_EXPORT
void foeWsiWindowSetShouldClose(foeWsiWindow window, bool terminate);

FOE_WSI_EXPORT
void foeWsiWindowGetSize(foeWsiWindow window, int *pWidth, int *pHeight);
FOE_WSI_EXPORT
bool foeWsiWindowResized(foeWsiWindow window);
FOE_WSI_EXPORT
void foeWsiWindowResize(foeWsiWindow window, int width, int height);

FOE_WSI_EXPORT
bool foeWsiWindowVisible(foeWsiWindow window);
FOE_WSI_EXPORT
void foeWsiWindowHide(foeWsiWindow window);
FOE_WSI_EXPORT
void foeWsiWindowShow(foeWsiWindow window);

/** @brief
 * @param pScaleX X scaling factor requested for content
 * @param pScaleY Y scaling factor requested for content
 *
 * For macOS and Windows, content can be requested to be scaled independent of window sizing for
 * HiDPI screens.
 */
FOE_WSI_EXPORT
void foeWsiWindowGetContentScale(foeWsiWindow window, float *pScaleX, float *pScaleY);

FOE_WSI_EXPORT
foeWsiKeyboard const *foeWsiGetKeyboard(foeWsiWindow window);
FOE_WSI_EXPORT
foeWsiMouse const *foeWsiGetMouse(foeWsiWindow window);

#ifdef __cplusplus
}
#endif

#endif // FOE_WSI_WINDOW_H