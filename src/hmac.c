#include "../include/ecc/hmac.h"

uint8_t* make_padding(const uint8_t *key, size_t key_len,
                            size_t block_size, size_t hash_size,
                            hash_func_t hash
) {
    uint8_t *k0 = malloc(block_size);
    if (!k0) return NULL;

    if (key_len == block_size) {
        memcpy(k0, key, block_size);
    }
    else if (key_len < block_size) {
       memcpy(k0, key, key_len);
        memset(k0 + key_len, 0, block_size - key_len);
    }
    else {
        uint8_t *hashed = malloc(hash_size);
        if (!hashed) {
            free(k0);
            return NULL;
        }
        hash(key, key_len, hashed);
        memcpy(k0, hashed, hash_size);
        memset(k0 + hash_size, 0, block_size - hash_size);
        free(hashed);
    }
    return k0;
}

int hmac_compute(const uint8_t *key, size_t key_len,
                 const uint8_t *msg, size_t msg_len,
                 size_t block_size, size_t hash_size,
                 hash_func_t hash,
                 uint8_t *out
) {
    if (!key || !msg || !hash || !out) {
        fprintf(stderr, "eror: invalid params\n");
        return -1;
    }

    uint8_t *k0 = make_padding(key, key_len, block_size, hash_size, hash);
    if (!k0) {
        fprintf(stderr, "eror: make_padding failed\n");
        return -1;
    }

    uint8_t *inner_pad = malloc(block_size);
    uint8_t *outer_pad = malloc(block_size);
    if (!inner_pad || !outer_pad) {
        free(k0);
        free(inner_pad);
        free(outer_pad);
        fprintf(stderr, "eror: bad malloc\n");
        return -1;
    }
    for (size_t i = 0; i < block_size; ++i) {
        inner_pad[i] = k0[i] ^ 0x36;
        outer_pad[i] = k0[i] ^ 0x5C;
    }
    free(k0);

    uint8_t *inner_input = malloc(block_size + msg_len);
    if (!inner_input) {
        free(inner_pad);
        free(outer_pad);
        fprintf(stderr, "eror: bad malloc\n");
        return -1;
    }
    memcpy(inner_input, inner_pad, block_size);
    memcpy(inner_input + block_size, msg, msg_len);
    free(inner_pad);

    uint8_t *inner_hash = malloc(hash_size);
    if (!inner_hash) {
        free(inner_input);
        free(outer_pad);
        fprintf(stderr, "eror: bad malloc\n");
        return -1;
    }
    hash(inner_input, block_size + msg_len, inner_hash);
    free(inner_input);

    uint8_t *outer_input = malloc(block_size + hash_size);
    if (!outer_input) {
        free(inner_hash);
        free(outer_pad);
        fprintf(stderr, "eror: bad malloc\n");
        return -1;
    }
    memcpy(outer_input, outer_pad, block_size);
    memcpy(outer_input + block_size, inner_hash, hash_size);
    free(outer_pad);
    free(inner_hash);

    hash(outer_input, block_size + hash_size, out);
    free(outer_input);

    return 0;
}
