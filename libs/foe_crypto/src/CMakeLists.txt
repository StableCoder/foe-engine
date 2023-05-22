# Copyright (C) 2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_crypto
  PRIVATE aes_gcm.c
          ed25519.c
          key.c
          memory.c
          random.c
          result.c
          sha256.c
          sha512.c
          x25519.c
          xchacha20_poly1305.c)
