#include "../include/ecc/serialize.h"
#include <stdlib.h>

ecc_status_code serialize_uncompressed_pub_key_sec1(
    const ecc_public_key *pb_k,
    serialized_pub_key_uncompressed *out,
    const ecc_curve *curve
) {

    if (!pb_k || !out || !curve) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));
    if (pb_k->G.inf || equal(pb_k->G.z, from_u64(0))) {
        out->data[0] = 0x00;
        return ECC_OK;
    }

    ecc_point_affine P_aff;
    projective_to_affine(&P_aff, &pb_k->G, curve->F.p);
    if (!is_on_curve(&P_aff, curve)) return ECC_INVALID_PARAMS;

    out->data[0] = 0x04;
    for (int i = NUM_LIMBS-1, idx = 1; i >= 0; --i) {
        for (int j = 3; j >= 0; --j, ++idx) {
            out->data[idx] =  ((P_aff.x.d[i] >> (8 * j)) & 0xFF);
            out->data[idx + 4 * NUM_LIMBS] =  ((P_aff.y.d[i] >> (8 * j)) & 0xFF);
        }
    }
    return ECC_OK;
}

ecc_status_code serialize_compressed_pub_key_sec1(
    const ecc_public_key *pb_k,
    serialized_pub_key_compressed *out,
    const ecc_curve *curve
) {
    if (!pb_k || !out || !curve) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));
    if (pb_k->G.inf || equal(pb_k->G.z, from_u64(0))) {
        out->data[0] = 0x00;
        return ECC_OK;
    }

    ecc_point_affine P_aff;
    projective_to_affine(&P_aff, &pb_k->G, curve->F.p);
    if (!is_on_curve(&P_aff, curve)) return ECC_INVALID_PARAMS;

    out->data[0] = 0x02 + (P_aff.y.d[0] & 1);
    for (int i = NUM_LIMBS-1, idx = 1; i >= 0; --i) {
        for (int j = 3; j >= 0; --j, ++idx) {
            out->data[idx] =  ((P_aff.x.d[i] >> (8 * j)) & 0xFF);
        }
    }
    return ECC_OK;
}

ecc_status_code serialize_priv_key_sec1(
    const ecc_private_key *pr_k,
    serialized_priv_key *out,
    const ecc_curve *curve
) {
    if (!pr_k || !out || !curve) return ECC_INVALID_PARAMS;
    if (cmp(pr_k->n, from_u64(1)) < 0 || cmp(pr_k->n, curve->N) >= 0) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));

    for (int i = NUM_LIMBS-1, idx = 0; i >= 0; --i) {
        for (int j = 3; j >= 0; --j, ++idx) {
            out->data[idx] =  ((pr_k->n.d[i] >> (8 * j)) & 0xFF);
        }
    }
    return ECC_OK;
}


ecc_status_code deserialize_uncompressed_pub_key_sec1(
    ecc_public_key *pb_k,
    const uint8_t *in,
    size_t len,
    const ecc_curve *curve
) {
    if (!pb_k || !in || !curve) return ECC_INVALID_PARAMS;

    if (in[0] != 0x04 || len != PUB_UNCOMPRESSED_SIZE) return ECC_INVALID_PARAMS;

    // pb_k->G = (ecc_point_projective *) malloc (sizeof(ecc_point_projective));
    // if (!pb_k->G) return ECC_FAIL;

    int limb_idx = 0;
    for (int i = PUB_UNCOMPRESSED_SIZE - 1; i > PUB_UNCOMPRESSED_SIZE - 1 - NUM_LIMBS * 4; i -= 4)
        pb_k->G.y.d[limb_idx++] = ((uint32_t)in[i] | ((uint32_t)in[i-1] << 8) | ((uint32_t)in[i-2] << 16) | ((uint32_t)in[i-3] << 24));
    limb_idx = 0;
    for (int i = PUB_UNCOMPRESSED_SIZE - 1 - NUM_LIMBS * 4; i > 0; i -= 4)
        pb_k->G.x.d[limb_idx++] = ((uint32_t)in[i] | ((uint32_t)in[i-1] << 8) | ((uint32_t)in[i-2] << 16) | ((uint32_t)in[i-3] << 24));
    pb_k->G.z = from_u64(1);
    pb_k->G.inf = false;

    if (cmp(pb_k->G.x, curve->F.p) >= 0 || cmp(pb_k->G.y, curve->F.p) >= 0 ||
        !is_on_curve_projective(&pb_k->G, curve)) {
        // free(pb_k->G);
        // pb_k->G = NULL;
        return ECC_INVALID_PARAMS;
    }

    return ECC_OK;
}

ecc_status_code deserialize_compressed_pub_key_sec1(
    ecc_public_key *pb_k,
    const uint8_t *in,
    size_t len,
    const ecc_curve *curve
) {
    if (!pb_k || !in || !curve) return ECC_INVALID_PARAMS;

    if ((in[0] != 0x02 && in[0] != 0x03) || len != PUB_COMPRESSED_SIZE) return ECC_INVALID_PARAMS;

    // pb_k->G = (ecc_point_projective *) malloc (sizeof(ecc_point_projective));
    // if (!pb_k->G) return ECC_FAIL;

    // y^2 = x^3 + 7 mod p => y = (x^3+7) ^ (p+1)/4 mod p
    int limb_idx = 0;
    for (int i = PUB_COMPRESSED_SIZE - 1; i > 0; i -= 4)
        pb_k->G.x.d[limb_idx++] = ((uint32_t)in[i] | ((uint32_t)in[i-1] << 8) | ((uint32_t)in[i-2] << 16) | ((uint32_t)in[i-3] << 24));
    
    if (cmp(pb_k->G.x, curve->F.p) >= 0) {
        // free(pb_k->G);
        // pb_k->G = NULL;
        return ECC_INVALID_PARAMS;
    }
    
    ecc_int left = sum(
        mul(pb_k->G.x, mul(pb_k->G.x, pb_k->G.x, curve->F.p), curve->F.p),
        from_u64(7),
        curve->F.p
    );
    pb_k->G.y = bin_pow(left, shift_right_1(shift_right_1(_sum(curve->F.p, from_u64(1)))), curve->F.p);
    pb_k->G.z = from_u64(1);
    pb_k->G.inf = false;
    if ((pb_k->G.y.d[0] & 1) != (in[0] == 0x03)) // 0x02 - even, 0x03 - odd
        pb_k->G.y = sub(curve->F.p, pb_k->G.y, curve->F.p);

    if (
        !equal(mul(pb_k->G.y, pb_k->G.y, curve->F.p), left)
    ) {
        // free(pb_k->G);
        // pb_k->G = NULL;
        return ECC_INVALID_PARAMS;
    }
    
    return ECC_OK;
}

