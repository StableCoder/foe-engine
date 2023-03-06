// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.h>
#include <foe/resource/result.h>

TEST_CASE("foeResource - Attempting to do a resource replacement with a defined type fails") {
    foeResource resource{FOE_NULL_HANDLE};
    foeResourceFns fns{};
    foeResultSet result;

    result = foeCreateResource(0, 0, &fns, sizeof(foeResourceBase), &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // Initially there is no replacement resource, as this object is 'unloaded'
    CHECK(foeResourceGetReplacement(resource) == FOE_NULL_HANDLE);
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);

    // Create the resource to act as the 'replacement' (type does not matter)
    foeResource replacementResource{FOE_NULL_HANDLE};
    result = foeCreateUndefinedResource(0, &fns, &replacementResource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // Replacement will fail with specific error
    result = foeResourceReplace(resource, replacementResource);
    REQUIRE(result.value == FOE_RESOURCE_ERROR_RESOURCE_NOT_UNDEFINED);

    foeResourceDecrementRefCount(replacementResource);
    foeResourceDecrementRefCount(resource);
}

TEST_CASE("foeResource - Replacement of undefined resource works as expected") {
    foeResource resource{FOE_NULL_HANDLE};
    foeResourceFns fns{};
    foeResultSet result;

    result = foeCreateUndefinedResource(0, &fns, &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // Initially there is no replacement resource, as this object is 'unloaded'
    CHECK(foeResourceGetReplacement(resource) == FOE_NULL_HANDLE);
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);

    // Create the resource to act as the 'replacement' (type does not matter)
    foeResource replacementResource{FOE_NULL_HANDLE};
    result = foeCreateUndefinedResource(0, &fns, &replacementResource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // The ref-count of the original resource should be 1, denoting the returned handle
    CHECK(foeResourceGetRefCount(replacementResource) == 1);

    SECTION("Replace with original resource having non-zero 'use' count") {
        CHECK(foeResourceIncrementUseCount(resource) == 1);

        // Perform the replacement
        result = foeResourceReplace(resource, replacementResource);
        REQUIRE(result.value == FOE_SUCCESS);

        // The old resource should now be considered as 'loaded', and the replacement resource
        // should be accurate
        CHECK(foeResourceGetReplacement(resource) == replacementResource);
        CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);

        // The replacement resource should now have 3 references counted:
        // 1. Reference returned when created
        // 2. Reference from the original (replaced) resource
        // 3. Reference returned from the original resource via `foeResourceGetReplacement` above
        CHECK(foeResourceGetRefCount(replacementResource) == 3);
        // As the original resource was 'used', this one is also marks as such
        CHECK(foeResourceGetUseCount(replacementResource) == 1);

        SECTION("Decrementing use count before ref-count of original resource") {
            // Decrementing the original resource 'use' count to zero also decrements the
            // replacement's count by 1
            CHECK(foeResourceDecrementUseCount(resource) == 0);
            CHECK(foeResourceGetUseCount(replacementResource) == 0);
        }
        SECTION("Not decrementing the use count beforehand")

        // The original resource still has a reference count of 1, and decrementing/destroying it
        // should also remove it's reference from the replacement resource, and the 'use' from
        // it
        CHECK(foeResourceGetRefCount(resource) == 1);
        CHECK(foeResourceDecrementRefCount(resource) == 0);
        CHECK(foeResourceGetRefCount(replacementResource) == 2);

        // If the use cound was not decremented by the original resource having it's use cound
        // decremented, then the ref-count would have decremented it instead
        CHECK(foeResourceGetUseCount(replacementResource) == 0);
    }
    SECTION("Replace with zero 'use' count") {
        // Perform the replacement
        result = foeResourceReplace(resource, replacementResource);
        REQUIRE(result.value == FOE_SUCCESS);

        // The old resource should now be considered as 'loaded', and the replacement resource
        // should be accurate
        CHECK(foeResourceGetReplacement(resource) == replacementResource);
        CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);

        // The replacement resource should now have 3 references counted
        // 1. Reference returned when created
        // 2. Reference from the original (replaced) resource
        // 3. Reference returned from the original resource via `foeResourceGetReplacement` above
        CHECK(foeResourceGetRefCount(replacementResource) == 3);
        // The replacement resource does not have a 'use' from the older resource
        CHECK(foeResourceGetUseCount(replacementResource) == 0);

        // The original resource still has a reference count of 1, and decrementing/destroying it
        // should also remove it's reference from the replacement resource
        CHECK(foeResourceGetRefCount(resource) == 1);
        CHECK(foeResourceDecrementRefCount(resource) == 0);
        CHECK(foeResourceGetRefCount(replacementResource) == 2);
    }

    // Cleanup
    CHECK(foeResourceDecrementRefCount(replacementResource) == 1);
    CHECK(foeResourceDecrementRefCount(replacementResource) == 0);
}