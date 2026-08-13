#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "types.h"
#include "big_int.h"
// #include ""

#define PUB_COMPRESSED_SIZE     4 * NUM_LIMBS + 1
#define PUB_UNCOMPRESSED_SIZE   8 * NUM_LIMBS + 1
#define PRIV_SIZE               2 * NUM_LIMBS

typedef struct {
    uint8_t data[PUB_COMPRESSED_SIZE];
} serialized_pub_key_compressed;

typedef struct {
    uint8_t data[PUB_UNCOMPRESSED_SIZE];
} serialized_pub_key_uncompressed;

typedef struct {
    uint8_t data[PRIV_SIZE];
} serialized_priv_key;

typedef struct {
    uint8_t data[PRIV_SIZE];
} serialized_shared_priv_key;

void serialize_uncompressed_pub_key_sec1(
    const ecc_public_key *pb_k,
    serialized_pub_key_uncompressed *out,
    const ecc_curve *curve
);
void serialize_compressed_pub_key_sec1(
    const ecc_public_key *pb_k,
    serialized_pub_key_compressed *out,
    const ecc_curve *curve
);
void serialize_priv_key_sec1(
    const ecc_private_key *pb_k,
    serialized_pub_key_uncompressed *out,
    const ecc_curve *curve
);
void serialize_shared_priv_key_sec1(
    const ecc_private_key *pb_k,
    serialized_shared_priv_key *out,
    const ecc_curve *curve
);

#endif