#include "../include/ecc/ecdh.h"

void print_ecc_status_code(ecc_status_code code) {
    if (code == ECC_OK) {
        fprintf(stdout, "ECC_OK\n");
    } else if (code == ECC_INVALID_PARAMS) {
        fprintf(stdout, "ECC_INVALID_PARAMS\n");
    } else if (code == ECC_FAIL) {
        fprintf(stdout, "ECC_FAIL\n");
    } else if (code == ECC_NOT_ON_CURVE) {
        fprintf(stdout, "ECC_NOT_ON_CURVE\n");
    } else {
        fprintf(stdout, "Unknown code\n");
    }
}

void free_curve(ecc_curve *curve) {
    if (curve->F) {
        free(curve->F);
        curve->F = NULL;
    }
    if (curve->G_affine) {
        free(curve->G_affine);
        curve->G_affine = NULL;
    }
    if (curve->G_projective) {
        free(curve->G_projective);
        curve->G_projective = NULL;
    }
}

ecc_status_code init_curve(char *curve_ident, ecc_curve *curve) {
    if (strcmp(curve_ident, "secp256k1") == 0) {
        curve->a = from_u64(0); 
        curve->b = from_u64(7);
        
        curve->F = (ecc_field *) malloc(sizeof(ecc_field));
        if (!curve->F) return ECC_FAIL;
        
        // Prime p
        curve->F->p.d[0] = 0xFFFFFC2F;
        curve->F->p.d[1] = 0xFFFFFFFE;
        curve->F->p.d[2] = 0xFFFFFFFF;
        curve->F->p.d[3] = 0xFFFFFFFF;
        curve->F->p.d[4] = 0xFFFFFFFF;
        curve->F->p.d[5] = 0xFFFFFFFF;
        curve->F->p.d[6] = 0xFFFFFFFF;
        curve->F->p.d[7] = 0xFFFFFFFF;

        curve->G_affine = (ecc_point_affine *) malloc(sizeof(ecc_point_affine));
        if (!curve->G_affine) {
            free(curve->F);
            return ECC_FAIL;
        }
        curve->G_affine->inf = false;

        curve->G_affine->x.d[0] = 0x16F81798;
        curve->G_affine->x.d[1] = 0x59F2815B;
        curve->G_affine->x.d[2] = 0x2DCE28D9;
        curve->G_affine->x.d[3] = 0x029BFCDB;
        curve->G_affine->x.d[4] = 0xCE870B07;
        curve->G_affine->x.d[5] = 0x55A06295;
        curve->G_affine->x.d[6] = 0xF9DCBBAC;
        curve->G_affine->x.d[7] = 0x79BE667E;

        curve->G_affine->y.d[0] = 0xFB10D4B8;
        curve->G_affine->y.d[1] = 0x9C47D08F;
        curve->G_affine->y.d[2] = 0xA6855419;
        curve->G_affine->y.d[3] = 0xFD17B448;
        curve->G_affine->y.d[4] = 0x0E1108A8;
        curve->G_affine->y.d[5] = 0x5DA4FBFC;
        curve->G_affine->y.d[6] = 0x26A3C465;
        curve->G_affine->y.d[7] = 0x483ADA77;

        curve->N.d[0] = 0xD0364141;
        curve->N.d[1] = 0xBFD25E8C;
        curve->N.d[2] = 0xAF48A03B;
        curve->N.d[3] = 0xBAAEDCE6;
        curve->N.d[4] = 0xFFFFFFFE;
        curve->N.d[5] = 0xFFFFFFFF;
        curve->N.d[6] = 0xFFFFFFFF;
        curve->N.d[7] = 0xFFFFFFFF;

        curve->h = from_u64(1);
        
        curve->G_projective = (ecc_point_projective *) malloc(sizeof(ecc_point_projective));
        if (!curve->G_projective) {
            free(curve->G_affine);
            free(curve->F);
            return ECC_FAIL;
        }
        curve->G_projective->x = curve->G_affine->x;
        curve->G_projective->y = curve->G_affine->y;
        curve->G_projective->z = from_u64(1);
        curve->G_projective->inf = false;

        return ECC_OK;
    } else {
        return ECC_INVALID_PARAMS;
    }
}

ecc_status_code generate_private_key(ecc_private_key *pr_k, const ecc_curve *curve) {
    ecc_int zero = from_u64(0);

    while (1) {
        ssize_t res = getrandom(pr_k->n->d, sizeof(pr_k->n->d), 0);
        if (res != sizeof(pr_k->n->d)) {
            return ECC_FAIL;
        }

        if (cmp(*(pr_k->n), curve->N) < 0 && !equal(*(pr_k->n), zero)) {
            return ECC_OK;
        }
    }

    return ECC_OK;
}

ecc_status_code calculate_public_key(ecc_private_key *pr_k, ecc_curve *curve, ecc_public_key *pb_k) {
    mul_scalar_projective(pb_k->G, curve->G_projective, *(pr_k->n), curve);
    return ECC_OK;
}

bool validate_public_key(ecc_public_key *pb_k, ecc_curve *curve) {
    if (pb_k->G->inf) return false;
    if (!is_on_curve_projective(pb_k->G, curve)) return false;
    
    return true;
}

ecc_status_code generate_key_pair (ecc_private_key *pr_k, ecc_public_key *pb_k, ecc_curve *E) {
    pr_k->n = (ecc_int *) malloc (sizeof(ecc_int));
    pb_k->G = (ecc_point_projective *) malloc(sizeof(ecc_point_projective));
    ecc_status_code error = generate_private_key(pr_k, E);
    if (error != ECC_OK)
        return error;
    *pb_k->G = NULL_POINT_PROJECTIVE;
    error = calculate_public_key(pr_k, E, pb_k);
    // fprintf (stdout, "here\n");
    if (error != ECC_OK)
        return error;
    if (!validate_public_key(pb_k, E))
        return ECC_NOT_ON_CURVE;
    return ECC_OK;
}

ecc_status_code calculate_general_private_key(ecc_private_key *pr_k, ecc_public_key *pb_k, ecc_curve *curve, ecc_general_private_key *gen_pr_k) {
    if (!validate_public_key(pb_k, curve)) {
        return ECC_NOT_ON_CURVE;
    }
    gen_pr_k->G = (ecc_point_projective *) malloc(sizeof(ecc_point_projective));
    mul_scalar_projective(gen_pr_k->G, pb_k->G, *(pr_k->n), curve);
    return ECC_OK;
}
