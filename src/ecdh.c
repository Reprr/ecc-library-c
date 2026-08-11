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

void free_curve (ecc_curve *curve) {
    if (curve->F) free(curve->F);
    if (curve->G_affine) free(curve->G_affine);
    if (curve->G_projective) free(curve->G_projective);
}

ecc_status_code init_curve(char *curve_ident, ecc_curve *curve) {
    if (strcmp(curve_ident, "secp256k1") == 0) {
        // y^2 = x^3 + 7 mod p
        curve->a = from_u64(0); 
        curve->b = from_u64(7);
        
        curve->F = (ecc_field *) malloc(sizeof(ecc_field));
        
        // Prime p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFC2F
        curve->F->p.d[0] = 0xFFFFFC2F;
        curve->F->p.d[1] = 0xFFFFFFFE;
        curve->F->p.d[2] = 0xFFFFFFFF;
        curve->F->p.d[3] = 0xFFFFFFFF;
        curve->F->p.d[4] = 0xFFFFFFFF;
        curve->F->p.d[5] = 0xFFFFFFFF;
        curve->F->p.d[6] = 0xFFFFFFFF;
        curve->F->p.d[7] = 0xFFFFFFFF;

        curve->G_affine = (ecc_point_affine *) malloc(sizeof(ecc_point_affine));
        curve->G_affine->inf = false;

        // Gx = 79BE667E F9DCBBAC 55A06295 CE870B07 029BFCDB 2DCE28D9 59F2815B 16F81798
        curve->G_affine->x.d[0] = 0x16F81798;
        curve->G_affine->x.d[1] = 0x59F2815B;
        curve->G_affine->x.d[2] = 0x2DCE28D9;
        curve->G_affine->x.d[3] = 0x029BFCDB;
        curve->G_affine->x.d[4] = 0xCE870B07;
        curve->G_affine->x.d[5] = 0x55A06295;
        curve->G_affine->x.d[6] = 0xF9DCBBAC;
        curve->G_affine->x.d[7] = 0x79BE667E;

        // Gy = 483ADA77 26A3C465 5DA4FBFC 0E1108A8 FD17B448 A6855419 9C47D08F FB10D4B8
        curve->G_affine->y.d[0] = 0xFB10D4B8;
        curve->G_affine->y.d[1] = 0x9C47D08F;
        curve->G_affine->y.d[2] = 0xA6855419;
        curve->G_affine->y.d[3] = 0xFD17B448;
        curve->G_affine->y.d[4] = 0x0E1108A8;
        curve->G_affine->y.d[5] = 0x5DA4FBFC;
        curve->G_affine->y.d[6] = 0x26A3C465;
        curve->G_affine->y.d[7] = 0x483ADA77;

        // Order N = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE BAAEDCE6 AF48A03B BFD25E8C D0364141
        curve->N.d[0] = 0xD0364141;
        curve->N.d[1] = 0xBFD25E8C;
        curve->N.d[2] = 0xAF48A03B;
        curve->N.d[3] = 0xBAAEDCE6;
        curve->N.d[4] = 0xFFFFFFFE;
        curve->N.d[5] = 0xFFFFFFFF;
        curve->N.d[6] = 0xFFFFFFFF;
        curve->N.d[7] = 0xFFFFFFFF;

        curve->h = from_u64(1); // Cofactor

        return ECC_OK;
    } else {
        return ECC_INVALID_PARAMS;
    }
}

ecc_status_code generate_private_key(ecc_private_key *pr_k, const ecc_curve *curve) {

    while (1) {
        ssize_t res = getrandom(pr_k->n->d, sizeof(pr_k->n->d), 0); // 0 - /dev/urandom

        if (res != sizeof (pr_k->n->d)) {
            return ECC_FAIL;
        }

        if (cmp(*(pr_k->n, curve->N) < 0) && !equal(pr_k->n, from_u64(0))) {
            return ECC_OK;
        }
    }

    return ECC_OK;
}

ecc_status_code calculate_public_key (ecc_private_key *pr_k, ecc_curve *curve, ecc_public_key *pb_k) {
    *pb_k->G = mul_scalar_affine(*curve->G_affine, *pr_k->n, (*curve));
    return ECC_OK;
}

ecc_status_code calculate_general_private_key (ecc_private_key *pr_k, ecc_public_key *pb_k, ecc_curve *curve, ecc_general_private_key *gen_pr_k) {
    if (!validate(pb_k, curve)) {
        return ECC_NOT_ON_CURVE;
    }
    
    *gen_pr_k->G = mul_scalar_affine(*pb_k->G, *pr_k->n, (*curve));

    return ECC_OK;
}

bool validate_public_key (ecc_public_key *pb_k, ecc_curve *curve) {
    return !is_on_curve(*pb_k->G, *curve);
}



