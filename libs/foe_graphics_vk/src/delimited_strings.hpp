/*
    Copyright (C) 2022 George Cave.

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

#ifndef DELIMITED_STRINGS_HPP
#define DELIMITED_STRINGS_HPP

#include <cstdint>
#include <system_error>

/** @brief Convers sets of individual strings into a single larger one
 * @param srcCount Number of source strings in ppSrc
 * @param ppSrc Points to a set of strings to be converted
 * @param[out] pDstLength is a pointer that will return the size of the ending destination buffer
 * @param[out] ppDst returns the final buffer with the delimited string if created.
 *
 * This function goes through all the provided source strings, and will compact them all into one
 * single larger allocation where each string is delimited using the NUL character.
 *
 * Strings of no size are skipped.
 *
 * If the total length of the strings is zero, then no buffer is created and ppDst is not modified.
 */
void convertToDelimitedString(uint32_t srcCount,
                              char const **ppSrc,
                              uint32_t *pDstLength,
                              char **ppDst);

/** @brief Returns data in the given destination if possible, or returns the destinations ize
 * required.
 * @param srcLength is the total length of the original delimited string
 * @param pSrc is a pointer to the delimited string
 * @param pDstLength is a pointer to an integer related to the number of characters required to
 * return the delimited string
 * @param pDst is either NULL or a pointer to a set of char*
 * @return FOE_GRAPHICS_VK_SUCCESS on success or FOE_GRAPHICS_VK_INCOMPLETE if not pDst was not
 * large enought to fit the whole source string.
 *
 * If pDst is NULL then the number of characters required to return all of the delimited source
 * string is returned in pDstLength. Otherwise pDstLength *must* point to a variable set by the user
 * to the size of pDst, and on return the variable is overwritten with the size of data actually
 * written to pDst.
 *
 * If pDstLength is less than the space required to copy the source string, at most pDstLength
 * characters will be written and FOE_GRAPHICS_VK_INCOMPLETE will be returned instead of
 * FOE_GRAPHICS_VK_SUCCESS to indicate that not all data was returned.
 *
 * Data written to the provided destination is delimited by NULL characters and the returned length
 * counts the last character which will be NULL.
 */
std::error_code getDelimitedString(uint32_t srcLength,
                                   char const *pSrc,
                                   uint32_t *pDstLength,
                                   char *pDst);

#endif // DELIMITED_STRINGS_HPP