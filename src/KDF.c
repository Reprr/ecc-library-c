#include "../include/ecc/KDF.h"

ecc_status_code ansi_x963_kdf_sha256(
    uint8_t *K, size_t key_data_len,
    const uint8_t *Z, size_t z_len,
    const uint8_t *shared_info, size_t si_len
) {
    if (z_len + si_len + 4 >= SHA256_MAXLEN) return ECC_INVALID_PARAMS;
    if (key_data_len >= (size_t)SHA256_DIGEST_SIZE * 0xFFFFFFFFULL) return ECC_INVALID_PARAMS;

    uint32_t counter = 1;
    size_t K_idx = 0;
    size_t iterations = (key_data_len + SHA256_DIGEST_SIZE - 1) / SHA256_DIGEST_SIZE;

    for (size_t i = 0; i < iterations; ++i) {
        uint8_t cnt_be[4] = {
            (uint8_t)(counter >> 24), (uint8_t)(counter >> 16),
            (uint8_t)(counter >> 8),  (uint8_t)counter
        };

        sha256_ctx ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, Z, z_len);
        sha256_update(&ctx, cnt_be, 4);
        if (si_len > 0) sha256_update(&ctx, shared_info, si_len);

        uint8_t hash[SHA256_DIGEST_SIZE];
        sha256_finalize(&ctx, hash);

        size_t to_copy = key_data_len - K_idx;
        if (to_copy > SHA256_DIGEST_SIZE) to_copy = SHA256_DIGEST_SIZE;
        memcpy(K + K_idx, hash, to_copy);
        K_idx += to_copy;

        ++counter;
    }
    return ECC_OK;
}


