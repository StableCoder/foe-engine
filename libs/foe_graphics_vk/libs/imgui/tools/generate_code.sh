# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

#!/usr/bin/env sh
set -e

# Variables
START=72 # Prior to v72, vk.xml was not published, so that's the default minimum.
END=
SKIP_PARSE=0

help_blurb() {
    echo " -s, --start <INT> The starting version of Vulkan to generate for (default: 72)"
    echo " -e, --end <INT>   The ending version of Vulkan to generate for (default: none)"
    echo " --skip-parse      Skips generating new XML cache, just generate header files"
}

# Command-line parsing
while [[ $# -gt 0 ]]; do
    key="$1"

    case $key in
    -s | --start)
        START="$2"
        shift
        shift
        ;;
    -e | --end)
        END="$2"
        shift
        shift
        ;;
    --skip-parse)
        SKIP_PARSE=1
        shift
        ;;
    -h | --help)
        help_blurb
        exit 0
        ;;
    *)
        exit 1
        ;;
    esac
done

if [ $SKIP_PARSE -eq 0 ]; then
    if [ -f .gen_cache.xml ]; then
        rm .gen_cache.xml
    fi

    if ! [ -d Vulkan-Docs ]; then
        git clone https://github.com/KhronosGroup/Vulkan-Docs
    fi
    pushd Vulkan-Docs >/dev/null
    git fetch -p

    FIRST=1
    for TAG in $(git tag | grep -e "^v[0-9]*\.[0-9]*\.[0-9]*$" | sort -t '.' -k3nr); do
        EXTRA_OPTS=
        VER=$(echo $TAG | cut -d'.' -f3)
        if [[ $VER -lt $START ]]; then
            continue
        elif [ "$END" != "" ] && [[ $VER -gt $END ]]; then
            continue
        fi
        git checkout $TAG

        ../../../../../../external/vulkan_mini_libs/vulkan-mini-libs-2/tools/parse_vk_doc.py -i xml/vk.xml -w ../.gen_cache.xml
        FIRST=0
    done
    popd >/dev/null
fi

./generate_struct_source.py -x .gen_cache.xml -y structs.yaml

clang-format -i ../include/foe/graphics/vk/imgui/vk_struct.hpp
clang-format -i ../src/vk_struct.cpp
