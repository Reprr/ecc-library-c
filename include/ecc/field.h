#ifndef ECC_FIELD_H
#define ECC_FIELD_H

#include "types.h"
#include <stdlib.h>

static const ecc_point_affine NULL_POINT_AFFINE = { .x = 0, .y = 0, .inf = true };

ecc_int bin_pow (ecc_int a, ecc_int n, ecc_int p);
ecc_int inv (ecc_int a, ecc_int p);
ecc_int calc_by_mod (ecc_int a, ecc_int p);
ecc_int calc_lambda (const ecc_point_affine P, const ecc_point_affine Q, ecc_curve curve);
ecc_point_affine sum_affine(const ecc_point_affine P, const ecc_point_affine Q, ecc_curve curve);
ecc_point_affine mul_scalar_affine(const ecc_point_affine P, ecc_int n, ecc_curve curve);
bool is_on_curve (const ecc_point_affine P, ecc_curve curve);
void init_null_points();
void init();

#endif