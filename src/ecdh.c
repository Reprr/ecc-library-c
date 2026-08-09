#include "../include/ecc/ecdh.h"

void print_ecc_status_code (ecc_status_code code) {
    if (code == ECC_OK) {
        fprintf (stdout, "ECC_OK\n");
    } else if (code == ECC_INVALID_PARAMS) {
        fprintf (stdout, "ECC_INVALID_PARAMS\n");
    } else if (code == ECC_FAIL) {
        fprintf (stdout, "ECC_FAIL\n");
    } else {
        fprintf (stdout, "Unknown code\n");
    }
}

ecc_status_code init_curve (char *curve_ident, ecc_curve *curve) {
    if (strcmp(curve_ident, "secp256k1") == 0) {
        // y^2 = x^3 + 7 mod p;
        curve->a = from_u64(0); curve->b = from_u64(7);
        curve->F = (ecc_field *) malloc (sizeof(ecc_field));
        curve->F->p.d[0] = 0xFFFFFC2F;
        curve->F->p.d[1] = 0xFFFFFFFE;
        for (int i = 2; i < NUM_LIMBS; ++i)
            curve->F->p.d[i] = 0xFFFFFFFF;

        curve->G_affine = (ecc_point_affine *) malloc (sizeof (ecc_point_affine));

        curve->G_affine->x.d[0] = 0x16F81798; curve->G_affine->x.d[1] = 0x59F2815B;
        curve->G_affine->x.d[2] = 0x2DCE28D9; curve->G_affine->x.d[3] = 0x029BFCDB;
        curve->G_affine->x.d[4] = 0xCE870B07; curve->G_affine->x.d[5] = 0x55A06295;
        curve->G_affine->x.d[6] = 0xF9DCBBAC; curve->G_affine->x.d[7] = 0x79BE667E;

        curve->G_affine->y.d[0] = 0xFB10D4B8; curve->G_affine->y.d[1] = 0x9C47D048;
        // curve->G_affine.X.d[2] = ;

        return ECC_OK;
    } else {
        return ECC_INVALID_PARAMS;
    }
}

ecc_status_code generate_private_key(ecc_private_key *pr_k) {
    for (int i = 0; i < NUM_LIMBS; ++i) {
        ssize_t result = getrandom(&(pr_k->n->d[i]), sizeof(pr_k->n), 0); // 0 - /dev/urandom
        if (result < 0) {
            // fprintf(stderr, "/dev/urandom failed...\n");
            // exit(-1);
            return ECC_FAIL;
        }
    }
    return ECC_OK;
}

ecc_status_code calculate_public_key (ecc_private_key *pr_k, ecc_curve *curve, ecc_public_key *pb_k) {
    *pb_k->G = mul_scalar_affine(*curve->G_affine, *pr_k->n, (*curve));
    return ECC_OK;
}

// ecc_status_code calculate_general_private_key (ecc_private_key *pr_k, ecc_public_key *pb_k, ecc_curve *curve, ecc_general_private_key *gen_pr_k) {
//     gen_pr_k->G = mul_scalar_affine(pb_k->G, pr_k->n, (*curve));
//     return ECC_OK;
// }
