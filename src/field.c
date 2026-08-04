#include "../include/ecc/field.h"

ecc_int bin_pow(ecc_int a, ecc_int n, ecc_int p) {
    if (n == 0) return 1;
    if (n == 1) return a % p;
    if (n & 1)
        return (a * bin_pow(a, n-1, p)) % p;
    ecc_int b = bin_pow(a, n >> 1, p) % p;
    return (b * b) % p;
}

ecc_int inv(ecc_int a, ecc_int p) {
    return bin_pow(a, p-2, p);
}

ecc_int calc_by_mod(ecc_int a, ecc_int p) {
    return (a % p + p) % p;
}

ecc_int calc_lambda(ecc_point_affine P, ecc_point_affine Q, ecc_curve curve) {
    ecc_int p = curve.F->p;
    if (P.x == Q.x && P.y == Q.y && P.inf == Q.inf) {
        ecc_int num = calc_by_mod(3 * P.x * P.x + curve.a, p);
        ecc_int den = calc_by_mod(2 * P.y, p);
        return calc_by_mod(num * inv(den, p), p);
    } else {
        ecc_int num = calc_by_mod(Q.y - P.y, p);
        ecc_int den = calc_by_mod(Q.x - P.x, p);
        return calc_by_mod(num * inv(den, p), p);
    }
}

ecc_point_affine sum_affine(ecc_point_affine P, ecc_point_affine Q, ecc_curve curve) {
    ecc_int p = curve.F->p;
    if (P.inf) return Q;
    if (Q.inf) return P;

    if (P.x == Q.x && (P.y + Q.y) % p == 0)
        return NULL_POINT_AFFINE;

    if (P.x == Q.x && P.y == Q.y && P.y == 0)
        return NULL_POINT_AFFINE;

    ecc_int lambda = calc_lambda(P, Q, curve);
    ecc_point_affine R;
    R.inf = false;
    R.x = calc_by_mod(lambda * lambda - P.x - Q.x, p);
    R.y = calc_by_mod(lambda * (P.x - R.x) - P.y, p);
    return R;
}

ecc_point_affine mul_scalar_affine(ecc_point_affine P, ecc_int n, ecc_curve curve) {
    if (n == 0 || P.inf)
        return NULL_POINT_AFFINE;

    ecc_point_affine result = NULL_POINT_AFFINE;
    ecc_point_affine base = P;

    while (n > 0) {
        if (n & 1)
            result = sum_affine(result, base, curve);
        base = sum_affine(base, base, curve);
        n >>= 1;
    }
    return result;
}

bool is_on_curve(ecc_point_affine P, ecc_curve curve) {
    if (P.inf) return true;
    ecc_int p = curve.F->p;
    ecc_int left = calc_by_mod(P.y * P.y, p);
    ecc_int right = calc_by_mod(P.x * P.x * P.x + curve.a * P.x + curve.b, p);
    return left == right;
}