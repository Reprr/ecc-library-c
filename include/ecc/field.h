#ifndef ECC_FIELD_H
#define ECC_FIELD_H

#include "types.h"
#include <stdlib.h>
#include "big_int.h"

static ecc_point_affine NULL_POINT_AFFINE = { .x = {0}, .y = {0}, .inf = true };
static ecc_point_projective NULL_POINT_PROJECTIVE = { .x = {0}, .y = {1}, .z = {0}, .inf = true};

ecc_int bin_pow(ecc_int a, ecc_int n, ecc_int p);
ecc_int inv(ecc_int a, ecc_int p);
ecc_int calc_by_mod(ecc_int a, ecc_int p);
ecc_int calc_lambda(
    const ecc_point_affine *P,
    const ecc_point_affine *Q,
    const ecc_curve *curve
);
void sum_affine(
    ecc_point_affine *R,
    const ecc_point_affine *P,
    const ecc_point_affine *Q,
    const ecc_curve *curve
);
void mul_scalar_affine(
    ecc_point_affine *R,
    const ecc_point_affine *P,
    ecc_int n,
    const ecc_curve *curve
);
bool is_on_curve(const ecc_point_affine *P, const ecc_curve *curve);
void affine_to_projective(ecc_point_projective *proj, const ecc_point_affine *aff);
void projective_to_affine(ecc_point_affine *aff, const ecc_point_projective *proj, ecc_int p);
bool is_on_curve_projective(const ecc_point_projective *P, const ecc_curve *curve);
void double_projective(
    ecc_point_projective *R,
    const ecc_point_projective *P,
    const ecc_curve *curve
);
void sum_projective_neq(
    ecc_point_projective *R,
    const ecc_point_projective *P,
    const ecc_point_projective *Q,
    const ecc_curve *curve
);
void sum_projective(
    ecc_point_projective *R,
    const ecc_point_projective *P,
    const ecc_point_projective *Q,
    const ecc_curve *curve
);

void mul_scalar_projective(
    ecc_point_projective *R,
    const ecc_point_projective *P,
    ecc_int n,
    const ecc_curve *curve
);

void init_null_points();
void init();

#endif
