# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

#!/usr/bin/env sh
set -e

if ! [ -d Vulkan-Docs ]; then
    git clone https://github.com/KhronosGroup/Vulkan-Docs
fi
pushd Vulkan-Docs >/dev/null
git fetch -p
TAG=$(git tag | grep -e "^v[0-9]*\.[0-9]*\.[0-9]*$" | sort -t '.' -k3nr | head -n1)
echo "Checking out Vulkan-Docs @ $TAG"
git checkout $TAG >/dev/null
popd >/dev/null

../../../../../external/vulkan_mini_libs/vulkan-mini-libs-2/tools/parse_vk_doc.py -i Vulkan-Docs/xml/vk.xml -w .gen_cache.xml

mkdir -p fuzz_corpus
rm -rf fuzz_corpus/*
./generate_corpus.py -x .gen_cache.xml -y structs.yaml -o fuzz_corpus/

mkdir -p fuzz_build
rm -rf fuzz_build/*
cmake ../../../../../ -B fuzz_build -DAFL=ON -DBUILD_TESTS=ON -DAFL_MODE=LTO
make -C fuzz_build -j $(nproc) fuzz_foe_graphics_vk_yaml sort_fuzzed_foe_graphics_vk_yaml

for STRUCT in $(cat structs.yaml | grep -e '^Vk.*:$'); do
    STRUCT="${STRUCT::-1}"
    mkdir -p fuzz_output/$STRUCT/
    AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -i fuzz_corpus/$STRUCT/ -o fuzz_output/$STRUCT/ -m none -d -V 60 -- fuzz_build/libs/foe_graphics_vk/libs/yaml/test/fuzz/fuzz_foe_graphics_vk_yaml $STRUCT @@
done

set +e
echo ">> Sorting Fuzz Output"
mkdir -p ../data/fuzzed_test_data/
rm -rf ../data/fuzzed_test_data/*
for STRUCT in $(cat structs.yaml | grep -e '^Vk.*:$'); do
    STRUCT="${STRUCT::-1}"
    COUNT=0

    mkdir ../data/fuzzed_test_data/$STRUCT

    # Sort initial corpus
    for FILE in fuzz_corpus/$STRUCT/*; do
        if [ ! -f $FILE ]; then
            continue
        fi
        echo "> FILE: $FILE"

        RESULT=$(fuzz_build/libs/foe_graphics_vk/libs/yaml/test/fuzz/sort_fuzzed_foe_graphics_vk_yaml $STRUCT $FILE)
        if [ ! -z $RESULT ]; then
            cp $FILE ../data/fuzzed_test_data/$STRUCT/$COUNT-$RESULT.yaml
            COUNT=$((COUNT + 1))
        fi
    done
    # Sort Fuzz Output
    for FILE in fuzz_output/$STRUCT/default/queue/*; do
        if [ ! -f $FILE ]; then
            continue
        fi
        echo "> FILE: $FILE"
        RESULT=$(fuzz_build/libs/foe_graphics_vk/libs/yaml/test/fuzz/sort_fuzzed_foe_graphics_vk_yaml $STRUCT $FILE)
        if [ ! -z $RESULT ]; then
            cp $FILE ../data/fuzzed_test_data/$STRUCT/$COUNT-$RESULT.yaml
            COUNT=$((COUNT + 1))
        fi
    done
done

echo ">> Done"
