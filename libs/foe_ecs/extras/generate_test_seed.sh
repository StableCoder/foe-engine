#!/usr/bin/env sh

COUNT=$1
OUT_FILE=test_seed.hpp

printf "" >$OUT_FILE

printf "#ifndef TEST_SEED_HPP\n" >>$OUT_FILE
printf "#define TEST_SEED_HPP\n\n" >>$OUT_FILE

printf "#include <array>\n" >>$OUT_FILE
printf "#include <cstdint>\n\n" >>$OUT_FILE

printf "std::array<uint_least32_t, $COUNT> cTestSeed = {\n" >>$OUT_FILE

for i in $(seq $COUNT); do
    printf "0x$(hexdump -n4 -v -e '4/1 "%02X" "\n"' /dev/urandom),\n" >>$OUT_FILE
done

printf "};\n\n" >>$OUT_FILE

printf "#endif // TEST_SEED_HPP\n" >>$OUT_FILE

clang-format -i $OUT_FILE
