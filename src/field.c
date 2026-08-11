#include "../include/ecc/field.h"

ecc_int bin_pow(ecc_int a, ecc_int n, ecc_int p) {
    ecc_int result = from_u64(1);
    ecc_int base = a;
    
    while (cmp(n, from_u64(0)) > 0) {
        if (n.d[0] & 1) {
            result = mul(result, base, p);
        }
        base = mul(base, base, p);
        n = shift_right_1(n);
    }

    return result;
}

ecc_int inv(ecc_int a, ecc_int p) {
    return bin_pow(a, _sub(p, from_u64(2)), p);
}

// ecc_int calc_by_mod(ecc_int a, ecc_int p) {
//     return (a % p + p) % p;
// }

ecc_int calc_lambda(ecc_point_affine P, ecc_point_affine Q, ecc_curve curve) {
    ecc_int p = curve.F->p;
    if (equal(P.x, Q.x) && equal(P.y, Q.y)) {
        ecc_int num = sum(mul_scalar(mul(P.x, P.x, p), from_u64(3), p), curve.a, p);
        ecc_int den = mul_scalar(P.y, from_u64(2), p);
        return mul(num, inv(den, p), p);
    } else {
        ecc_int num = sub(Q.y, P.y, p);
        ecc_int den = sub(Q.x, P.x, p);
        return mul(num, inv(den, p), p);
    }
}

ecc_point_affine sum_affine(ecc_point_affine P, ecc_point_affine Q, ecc_curve curve) {
    ecc_int p = curve.F->p;
    if (P.inf) return Q;
    if (Q.inf) return P;

    if (equal(P.x, Q.x) && equal(sum(P.y, Q.y, p), from_u64(0))) // P == -Q
        return NULL_POINT_AFFINE;

    if (equal(P.x, Q.x) && equal(P.y, Q.y) && equal(P.y, from_u64(0))) // P == Q == O
        return NULL_POINT_AFFINE;

    ecc_int lambda = calc_lambda(P, Q, curve);
    ecc_point_affine R;
    R.inf = false;
    // R.x = calc_by_mod(lambda * lambda - P.x - Q.x, p);
    R.x = sub(
        mul(lambda, lambda, p),
        sum(P.x, Q.x, p),
        p
    );
    // R.y = calc_by_mod(lambda * (P.x - R.x) - P.y, p);
    R.y = sub(
        mul(lambda, sub(P.x, R.x, p), p),
        P.y,
        p
    );
    return R;
}

// NOT SECURE
ecc_point_affine mul_scalar_affine(ecc_point_affine P, ecc_int n, ecc_curve curve) {
    if (equal(n, from_u64(0)) || P.inf)
        return NULL_POINT_AFFINE;

    ecc_point_affine result = NULL_POINT_AFFINE;
    ecc_point_affine base = P;

    while (cmp(n, from_u64(0))) {
        if (n.d[0] & 1)
            result = sum_affine(result, base, curve);
        base = sum_affine(base, base, curve);
        // n >>= 1;
        n = shift_right_1(n);
    }
    return result;
}

bool is_on_curve(ecc_point_affine P, ecc_curve curve) {
    if (P.inf) return true;
    ecc_int p = curve.F->p;
    ecc_int left = mul(P.y, P.y, p);
    // ecc_int right = calc_by_mod(P.x * P.x * P.x + curve.a * P.x + curve.b, p);
    ecc_int right = sum(
        mul(
            mul(P.x, P.x, p),
            P.x,
            p
        ),
        sum(
            mul(curve.a, P.x, p),
            curve.b,
            p
        ),
        p
    );
    return equal(left, right);
}