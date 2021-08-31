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

#ifndef FOE_ALGORITHM_HPP
#define FOE_ALGORITHM_HPP

#include <algorithm>
#include <utility>

namespace foe {

/** @brief Removes duplicate elements in the defined range
 * @tparam ForwardIt The type of iterator that denotes the beginning/end of a range
 * @param first The iterator to the first element
 * @param last The iterator denoting the end of the search area
 * @return Iterator to the new end-element of the given array
 */
template <class ForwardIt>
constexpr ForwardIt remove_duplicates(ForwardIt first, ForwardIt last) {
    auto end_unique = last;
    for (auto iter = first; iter != end_unique; ++iter) {
        end_unique = std::remove(iter + 1, end_unique, *iter);
    }

    return end_unique;
}

/** @brief Removes side-by-side same elements, leaving the last from any sequence
 * @tparam ForwardIt Iterator type that at least moves forward
 * @param first Iterator to the first element
 * @param last Iterator to the end of the range
 * @return Iterator to the new end-element of the given array
 *
 * Goes through the indicated range of objects and for any objects that are together in a asection
 * that equate to the same, removed the earlier items and leaves the last one.
 *
 * Essentially the same as std::unique but leaves the last instead of the first item of a sequence.
 */
template <class ForwardIt>
constexpr ForwardIt unique_last(ForwardIt first, ForwardIt last) {
    if (first == last)
        return last;

    ForwardIt result = first;
    while (++first != last) {
        if (*result == *first) {
            // Result doesn't move
            *result = std::move(*first);
        } else if (++result != first) {
            // Result moved forward from if statement
            // If we found a match, the move this mismatch element down.
            *result = std::move(*first);
        }
    }

    return ++result;
}

/** @brief Removes side-by-side same elements, leaving the last from any sequence
 * @tparam ForwardIt Iterator type that at least moves forward
 * @tparam BinaryPredicate Predicate to use to evaluate two items
 * @param first Iterator to the first element
 * @param last Iterator to the end of the range
 * @param p Predicate to use for comparison
 * @return Iterator to the new end-element of the given array
 *
 * Goes through the indicated range of objects and for any objects that are together in a asection
 * that equate to the same, removed the earlier items and leaves the last one.
 *
 * Essentially the same as std::unique but leaves the last instead of the first item of a sequence.
 */
template <class ForwardIt, class BinaryPredicate>
constexpr ForwardIt unique_last(ForwardIt first, ForwardIt last, BinaryPredicate p) {
    if (first == last)
        return last;

    ForwardIt result = first;
    while (++first != last) {
        if (p(*result, *first)) {
            // Result doesn't move
            *result = std::move(*first);
        } else if (++result != first) {
            // Result moved forward from if statement
            // If we found a match, the move this mismatch element down.
            *result = std::move(*first);
        }
    }

    return ++result;
}

} // namespace foe

#endif // FOE_ALGORITHM_HPP