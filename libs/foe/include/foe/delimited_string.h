// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_DELIMITED_STRING_H
#define FOE_DELIMITED_STRING_H

#include <foe/export.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Converts sets of individual strings into a single larger one
 * @param srcCount Number of source strings in ppSrc
 * @param ppSrc Points to a set of strings to be converted
 * @param[out] pDstLength is a pointer to an integer related to the character array size required
 * to combine the provided source strings
 * @param[out] pDst is either NULL or a pointer to a character array
 * @return true on success or false if pDst was not large enough to fit the whole source string
 *
 * This function goes through all the provided source strings, and will compact them all into one
 * single larger allocation where each string is delimited using the NUL character.
 *
 * Strings of no size are skipped/ignored.
 *
 * If the total length of the strings is zero, then no buffer is created and ppDst is not modified.
 */
FOE_EXPORT
bool foeCreateDelimitedString(
    uint32_t srcCount, char const *const *ppSrc, char delimiter, uint32_t *pDstLength, char *pDst);

/** @brief Returns data in the given destination if possible, or returns the buffer size required.
 * @param srcLength is the total length of the original delimited string
 * @param pSrc is a pointer to the delimited string
 * @param pDstLength is a pointer to an integer related to the number of characters required to
 * return the delimited string
 * @param pDst is either NULL or a pointer to a set of char*
 * @return true on success or false if pDst was not large enough to fit the whole source string
 *
 * If pDst is NULL then the number of characters required to return all of the delimited source
 * string is returned in pDstLength. Otherwise pDstLength *must* point to a variable set by the user
 * to the size of pDst, and on return the variable is overwritten with the size of data actually
 * written to pDst.
 *
 * If pDstLength is less than the space required to copy the source string, at most pDstLength
 * characters will be written and true will be returned instead of false to indicate that not all
 * data was returned.
 *
 * Data written to the provided destination is delimited by NULL characters and the returned length
 * counts the last character which will be NULL.
 */
FOE_EXPORT
bool foeCopyDelimitedString(
    uint32_t srcLength, char const *pSrc, char delimiter, uint32_t *pDstLength, char *pDst);

/** @brief Returns the start and length of the string inside the larger delimited string
 * @param srcLength is the total length of the original delimited string
 * @param pSrc is a pointer to the delimited string
 * @param index is the index of the string to get
 * @param pStr is a pointer to a char const* that will point to the start of the indexed string. If
 * false is returned then this is not modified.
 * @return true if the requested string index was found and pStr was thus modified, otherwise false
 */
FOE_EXPORT
bool foeIndexedDelimitedString(uint32_t srcLength,
                               char const *pSrc,
                               uint32_t index,
                               char delimiter,
                               uint32_t *pStrLength,
                               char const **ppStr);

#ifdef __cplusplus
}
#endif

#endif // FOE_DELIMITED_STRING_H