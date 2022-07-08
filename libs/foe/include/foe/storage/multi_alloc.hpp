// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_STORAGE_MULTI_ALLOC_HPP
#define FOE_STORAGE_MULTI_ALLOC_HPP

#include <array>
#include <cstddef>
#include <cstdlib>
#include <tuple>

template <typename... Types>
class foeMultiAllocStorage {
  public:
    foeMultiAllocStorage(size_t capacity = 0);
    ~foeMultiAllocStorage();

    foeMultiAllocStorage(foeMultiAllocStorage const &) = delete;
    foeMultiAllocStorage &operator=(foeMultiAllocStorage const &) = delete;

    foeMultiAllocStorage(foeMultiAllocStorage &&);
    auto operator=(foeMultiAllocStorage &&) -> foeMultiAllocStorage &;

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
    auto buildAllocSizeArray(size_t capacity) const noexcept
        -> std::array<size_t, sizeof...(Types)>;

    template <int Index = 0>
    void allocSize(size_t capacity,
                   std::array<size_t, sizeof...(Types)> &allocSizeArray) const noexcept;

    size_t mCapacity;
    std::array<void *, sizeof...(Types)> mBuffers;
};

template <typename... Types>
foeMultiAllocStorage<Types...>::foeMultiAllocStorage(size_t capacity) : mCapacity{capacity} {
    if (mCapacity == 0) {
        mBuffers = {};
        return;
    }

    auto allocSizeArray = buildAllocSizeArray(capacity);

    for (size_t i = 0; i < mBuffers.size(); ++i) {
        mBuffers[i] = malloc(allocSizeArray[i]);
    }
}

template <typename... Types>
foeMultiAllocStorage<Types...>::~foeMultiAllocStorage() {
    for (auto &it : mBuffers) {
        free(it);
    }
}

template <typename... Types>
foeMultiAllocStorage<Types...>::foeMultiAllocStorage(foeMultiAllocStorage &&other) :
    mCapacity{other.mCapacity}, mBuffers{other.mBuffers} {
    // Clear other
    other.mCapacity = 0;
    other.mBuffers = {};
}

template <typename... Types>
auto foeMultiAllocStorage<Types...>::operator=(foeMultiAllocStorage &&other)
    -> foeMultiAllocStorage & {
    if (this != &other) {
        // Clear this one, if active
        if (mCapacity != 0) {
            for (auto &it : mBuffers) {
                free(it);
            }
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
auto foeMultiAllocStorage<Types...>::buildAllocSizeArray(size_t capacity) const noexcept
    -> std::array<size_t, sizeof...(Types)> {
    std::array<size_t, sizeof...(Types)> retArray;

    allocSize(capacity, retArray);

    return retArray;
}

template <typename... Types>
template <int Index>
void foeMultiAllocStorage<Types...>::allocSize(
    size_t capacity, std::array<size_t, sizeof...(Types)> &allocSizeArray) const noexcept {
    using IndexType = typename std::tuple_element<Index, std::tuple<Types...>>::type;

    allocSizeArray[Index] = capacity * sizeof(IndexType);

    if constexpr (Index != sizeof...(Types) - 1) {
        // Can go another element deeper
        allocSize<Index + 1>(capacity, allocSizeArray);
    }
}

#endif // FOE_STORAGE_MULTI_ALLOC_HPP