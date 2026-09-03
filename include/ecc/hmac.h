#ifndef HMAC_H
#define HMAC_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void (*hash_func_t)(const uint8_t *data, size_t len, uint8_t *out);

uint8_t* make_padding(const uint8_t *key, size_t key_len,
                    size_t block_size, size_t hash_size,
                    hash_func_t hash
);
int hmac_compute(const uint8_t *key, size_t key_len,
                 const uint8_t *msg, size_t msg_len,
                 size_t block_size, size_t hash_size,
                 hash_func_t hash,
                 uint8_t *out
);

#endif
