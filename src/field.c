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

ecc_int calc_lambda(const ecc_point_affine *P, const ecc_point_affine *Q, const ecc_curve *curve) {
    ecc_int p = curve->F->p;
    
    if (equal(P->x, Q->x) && equal(P->y, Q->y) && P->inf == Q->inf) {
        // lambda = (3x^2 + a) / (2y)
        ecc_int num = sum(mul_scalar(mul(P->x, P->x, p), from_u64(3), p), curve->a, p);
        ecc_int den = mul_scalar(P->y, from_u64(2), p);
        return mul(num, inv(den, p), p);
    } else {
        // lambda = (y2 - y1) / (x2 - x1)
        ecc_int num = sub(Q->y, P->y, p);
        ecc_int den = sub(Q->x, P->x, p);
        return mul(num, inv(den, p), p);
    }
}

void sum_affine(ecc_point_affine *R, const ecc_point_affine *P, const ecc_point_affine *Q, const ecc_curve *curve) {
    ecc_int p = curve->F->p;
    
    if (P->inf) {
        *R = *Q;
        return;
    }
    if (Q->inf) {
        *R = *P;
        return;
    }

    // P + (-P) = O
    if (equal(P->x, Q->x) && equal(sum(P->y, Q->y, p), from_u64(0))) {
        *R = NULL_POINT_AFFINE;
        return;
    }

    // P == Q == O
    if (equal(P->x, Q->x) && equal(P->y, Q->y) && equal(P->y, from_u64(0))) {
        *R = NULL_POINT_AFFINE;
        return;
    }

    ecc_int lambda = calc_lambda(P, Q, curve);
    
    R->inf = false;
    R->x = sub(mul(lambda, lambda, p), sum(P->x, Q->x, p), p);
    R->y = sub(mul(lambda, sub(P->x, R->x, p), p), P->y, p);
}

void mul_scalar_affine(ecc_point_affine *R, const ecc_point_affine *P, ecc_int n, const ecc_curve *curve) {
    if (equal(n, from_u64(0)) || P->inf) {
        *R = NULL_POINT_AFFINE;
        return;
    }

    ecc_point_affine result = NULL_POINT_AFFINE;
    ecc_point_affine base = *P;
    ecc_int temp = n;

    while (cmp(temp, from_u64(0)) > 0) {
        if (temp.d[0] & 1) {
            ecc_point_affine tmp;
            sum_affine(&tmp, &result, &base, curve);
            result = tmp;
        }
        ecc_point_affine tmp;
        sum_affine(&tmp, &base, &base, curve);
        base = tmp;
        temp = shift_right_1(temp);
    }
    
    *R = result;
}

bool is_on_curve(const ecc_point_affine *P, const ecc_curve *curve) {
    if (P->inf) return true;
    
    ecc_int p = curve->F->p;
    ecc_int left = mul(P->y, P->y, p);
    ecc_int right = sum(
        mul(mul(P->x, P->x, p), P->x, p),
        sum(mul(curve->a, P->x, p), curve->b, p),
        p
    );
    return equal(left, right);
}

void double_projective(ecc_point_projective *R, const ecc_point_projective *P, const ecc_curve *curve) {
    ecc_int p = curve->F->p;
    
    // W = 3X^2 + aZ^2
    ecc_int W = sum(
        mul(mul(P->x, P->x, p), from_u64(3), p),
        mul(curve->a, mul(P->z, P->z, p), p),
        p
    );
    
    // S = Y * Z
    ecc_int S = mul(P->y, P->z, p);
    
    // B = X * Y * S = X * Y^2 * Z
    ecc_int B = mul(P->x, mul(P->y, S, p), p);
    
    // H = W^2 - 8B
    ecc_int H = sub(mul(W, W, p), mul(B, from_u64(8), p), p);
    
    // X3 = 2 * H * S
    R->x = mul(mul(H, from_u64(2), p), S, p);
    
    // Y3 = W * (4B - H) - 8 * Y^2 * S^2
    ecc_int Y_sq_S_sq = mul(mul(P->y, P->y, p), mul(S, S, p), p);
    R->y = sub(
        mul(W, sub(mul(B, from_u64(4), p), H, p), p),
        mul(Y_sq_S_sq, from_u64(8), p),
        p
    );
    
    // Z3 = 8 * S^3
    R->z = mul(mul(S, mul(S, S, p), p), from_u64(8), p);
}

void sum_projective_neq(ecc_point_projective *R, const ecc_point_projective *P, const ecc_point_projective *Q, const ecc_curve *curve) {
    
}


void init_null_points() {
}

void init() {
    init_null_points();
}