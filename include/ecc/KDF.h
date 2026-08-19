#ifndef KDF_H
#define KDF_H

#include "sha_256.h"
#include "types.h"

ecc_status_code ansi_x963_kdf_sha256(
    uint8_t *K, size_t key_data_len,
    const uint8_t *Z, size_t z_len,
    const uint8_t *shared_info, size_t si_len
);

#endif