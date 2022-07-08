// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_STORAGE_SINGLE_ALLOC_HPP
#define FOE_STORAGE_SINGLE_ALLOC_HPP

#include <array>
#include <cstddef>
#include <cstdlib>
#include <tuple>

template <typename... Types>
class foeSingleAllocStorage {
  public:
    foeSingleAllocStorage(size_t capacity = 0);
    ~foeSingleAllocStorage();

    foeSingleAllocStorage(foeSingleAllocStorage const &) = delete;
    foeSingleAllocStorage &operator=(foeSingleAllocStorage const &) = delete;

    foeSingleAllocStorage(foeSingleAllocStorage &&);
    auto operator=(foeSingleAllocStorage &&) -> foeSingleAllocStorage &;

    template <int Index = 0>
    auto get() noexcept -> typename std::tuple_element<Index, std::tuple<Types...>>::type * {
        return static_cast<typename std::tuple_element<Index, std::tuple<Types...>>::type *>(
            mBuffers[Index]);
    }

    template <int Index = 0>
    auto get() const noexcept ->
        typename std::tuple_element<Index, std::tuple<Types...>>::type const * {
        return static_cast<typename std::tuple_element<Index, std::tuple<Types...>>::type const *>(
            mBuffers[Index]);
    }

    size_t capacity() const noexcept { return mCapacity; }

  private:
    template <int Index = 0>
    auto buildOffsetArray(size_t capacity) const noexcept
        -> std::tuple<size_t, std::array<size_t, sizeof...(Types)>>;

    template <int Index = 0>
    size_t allocSize(size_t capacity,
                     std::array<size_t, sizeof...(Types)> &allocSizeArray) const noexcept;

    size_t mCapacity;
    std::array<void *, sizeof...(Types)> mBuffers;
};

template <typename... Types>
foeSingleAllocStorage<Types...>::foeSingleAllocStorage(size_t capacity) : mCapacity{capacity} {
    if (mCapacity == 0) {
        mBuffers = {};
        return;
    }

    auto [allocSize, offsetArray] = buildOffsetArray(capacity);

    mBuffers[0] = malloc(allocSize);
    for (size_t i = 1; i < mBuffers.size(); ++i) {
        mBuffers[i] = static_cast<unsigned char *>(mBuffers[0]) + offsetArray[i];
    }
}

template <typename... Types>
foeSingleAllocStorage<Types...>::~foeSingleAllocStorage() {
    free(mBuffers[0]);
}

template <typename... Types>
foeSingleAllocStorage<Types...>::foeSingleAllocStorage(foeSingleAllocStorage &&other) :
    mCapacity{other.mCapacity}, mBuffers{other.mBuffers} {
    // Clear other
    other.mCapacity = 0;
    other.mBuffers = {};
}

template <typename... Types>
auto foeSingleAllocStorage<Types...>::operator=(foeSingleAllocStorage &&other)
    -> foeSingleAllocStorage & {
    if (this != &other) {
        // Clear this one, if active
        if (mCapacity != 0) {
            free(mBuffers[0]);
        }

        // Transfer over
        mCapacity = other.mCapacity;
        mBuffers = other.mBuffers;

        other.mCapacity = 0;
        other.mBuffers = {};
    }

    return *this;
}

template <typename... Types>
template <int Index>
auto foeSingleAllocStorage<Types...>::buildOffsetArray(size_t capacity) const noexcept
    -> std::tuple<size_t, std::array<size_t, sizeof...(Types)>> {
    std::array<size_t, sizeof...(Types)> offsetArray{};

    return std::make_tuple(allocSize(capacity, offsetArray), offsetArray);
}

template <typename... Types>
template <int Index>
size_t foeSingleAllocStorage<Types...>::allocSize(
    size_t capacity, std::array<size_t, sizeof...(Types)> &offsetArray) const noexcept {
    using IndexType = typename std::tuple_element<Index, std::tuple<Types...>>::type;

    size_t currentSize = offsetArray[Index];
    size_t fullSize = currentSize + sizeof(IndexType) * capacity;

    if constexpr (Index != sizeof...(Types) - 1) {
        // Can go another element deeper
        using NextType = typename std::tuple_element<Index + 1, std::tuple<Types...>>::type;

        size_t lessAlignment = fullSize / alignof(NextType);
        size_t moreAlignment = (lessAlignment + 1) * alignof(NextType);
        if ((moreAlignment - fullSize) == alignof(NextType)) {
            // The full alloc size so far fits alignment perfectly
            offsetArray[Index + 1] = fullSize;
        } else {
            // Not quite, need to add some buffer for better alignment
            offsetArray[Index + 1] = moreAlignment;
        }
        return allocSize<Index + 1>(capacity, offsetArray);
    } else {
        return fullSize;
    }
}

#endif // FOE_STORAGE_SINGLE_ALLOC_HPP