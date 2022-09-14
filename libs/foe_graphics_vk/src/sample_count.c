// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/sample_count.h>

VkSampleCountFlags foeGfxVkGetSampleCountFlags(int sampleCount) {
    switch (sampleCount) {
    case 1:
        return VK_SAMPLE_COUNT_1_BIT;

    case 2:
        return VK_SAMPLE_COUNT_2_BIT;

    case 4:
        return VK_SAMPLE_COUNT_4_BIT;

    case 8:
        return VK_SAMPLE_COUNT_8_BIT;

    case 16:
        return VK_SAMPLE_COUNT_16_BIT;

    case 32:
        return VK_SAMPLE_COUNT_32_BIT;

    case 64:
        return VK_SAMPLE_COUNT_64_BIT;

    default:
        return (VkSampleCountFlags)0;
    }
}

int foeGfxVkGetSampleCount(VkSampleCountFlags flags) {
    switch (flags) {
    case VK_SAMPLE_COUNT_1_BIT:
        return 1;

    case VK_SAMPLE_COUNT_2_BIT:
        return 2;

    case VK_SAMPLE_COUNT_4_BIT:
        return 4;

    case VK_SAMPLE_COUNT_8_BIT:
        return 8;

    case VK_SAMPLE_COUNT_16_BIT:
        return 16;

    case VK_SAMPLE_COUNT_32_BIT:
        return 32;

    case VK_SAMPLE_COUNT_64_BIT:
        return 64;

    default:
        return 0;
    }
}