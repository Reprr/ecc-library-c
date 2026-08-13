#include "../include/ecc/serialize.h"
#include <stdlib.h>

ecc_status_code serialize_uncompressed_pub_key_sec1(
    const ecc_public_key *pb_k,
    serialized_pub_key_uncompressed *out,
    const ecc_curve *curve
) {

    if (!pb_k || !pb_k->G || !out || !curve) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));
    if (pb_k->G->inf || equal(pb_k->G->z, from_u64(0))) {
        out->data[0] = 0x00;
        return ECC_OK;
    }

    ecc_point_affine P_aff;
    projective_to_affine(&P_aff, pb_k->G, curve->F->p);
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
    if (!pb_k || !pb_k->G || !out || !curve) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));
    if (pb_k->G->inf || equal(pb_k->G->z, from_u64(0))) {
        out->data[0] = 0x00;
        return ECC_OK;
    }

    ecc_point_affine P_aff;
    projective_to_affine(&P_aff, pb_k->G, curve->F->p);
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
    if (!pr_k || !pr_k->n || !out || !curve || !curve->N) return ECC_INVALID_PARAMS;
    if (cmp(pr_k->n, from_u64(1)) < 0 || cmp(pr_k->n, curve->N) > 0) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));

    for (int i = NUM_LIMBS-1, idx = 0; i >= 0; --i) {
        for (int j = 3; j >= 0; --j, ++idx) {
            out->data[idx] =  ((pr_k->n->d[i] >> (8 * j)) & 0xFF);
        }
    }
    return ECC_OK;
}

ecc_status_code serialize_shared_priv_key_sec1(
    const ecc_shared_private_key *shared_pr_k,
    serialized_shared_priv_key *out,
    const ecc_curve *curve
) {
    if (!shared_pr_k || !shared_pr_k->G || !out || !curve) return ECC_INVALID_PARAMS;

    memset(out->data, 0, sizeof(out->data));
    if (shared_pr_k->G->inf || equal(shared_pr_k->G->z, from_u64(0))) {
        out->data[0] = 0x00;
        return ECC_INVALID_PARAMS;
    }

    ecc_point_affine P_aff;
    projective_to_affine(&P_aff, shared_pr_k->G, curve->F->p);
    if (!is_on_curve(&P_aff, curve)) return ECC_INVALID_PARAMS;

    for (int i = NUM_LIMBS-1, idx = 0; i >= 0; --i) {
        for (int j = 3; j >= 0; --j, ++idx) {
            out->data[idx] =  ((P_aff.x.d[i] >> (8 * j)) & 0xFF);
        }
    }
    return ECC_OK;
}

