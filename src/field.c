#include "../include/ecc/field.h"

const ecc_point_affine NULL_POINT_AFFINE = { .x = {0}, .y = {0}, .inf = true };
const ecc_point_projective NULL_POINT_PROJECTIVE = { .x = {0}, .y = {0}, .z = {0}, .inf = true };


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
    ecc_int p = curve->F.p;
    
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
    ecc_int p = curve->F.p;
    
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
    
    ecc_int p = curve->F.p;
    ecc_int left = mul(P->y, P->y, p);
    ecc_int right = sum(
        mul(mul(P->x, P->x, p), P->x, p),
        sum(mul(curve->a, P->x, p), curve->b, p),
        p
    );
    return equal(left, right);
}

void affine_to_projective(ecc_point_projective *proj, const ecc_point_affine *aff) {
    proj->x = aff->x;
    proj->y = aff->y;
    proj->z = from_u64(1);
    proj->inf = false;
}

void projective_to_affine(ecc_point_affine *aff, const ecc_point_projective *proj, ecc_int p) {
    if (proj->inf || equal(proj->z, from_u64(0))) {
        aff->inf = true;
        aff->x = from_u64(0);
        aff->y = from_u64(0);
        return;
    }
    
    aff->inf = false;
    ecc_int z_inv = inv(proj->z, p);
    aff->x = mul(proj->x, z_inv, p);
    aff->y = mul(proj->y, z_inv, p);
}

bool is_on_curve_projective(const ecc_point_projective *P, const ecc_curve *curve) {
    if (P->inf || equal(P->z, from_u64(0))) return true;
    
    ecc_int p = curve->F.p;
    
    // Y^2 * Z
    ecc_int left = mul(mul(P->y, P->y, p), P->z, p);
    
    // X^3
    ecc_int x_sq = mul(P->x, P->x, p);
    ecc_int x_cu = mul(x_sq, P->x, p);
    
    // a * X * Z^2
    ecc_int z_sq = mul(P->z, P->z, p);
    ecc_int a_x_z2 = mul(mul(curve->a, P->x, p), z_sq, p);
    
    // b * Z^3
    ecc_int b_z3 = mul(curve->b, mul(z_sq, P->z, p), p);
    
    // right = X^3 + a*X*Z^2 + b*Z^3
    ecc_int right = sum(sum(x_cu, a_x_z2, p), b_z3, p);
    
    return equal(left, right);
}

void double_projective(ecc_point_projective *R, const ecc_point_projective *P, const ecc_curve *curve) {
    ecc_int p = curve->F.p;
    
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
    
    ecc_int new_x = mul(mul(H, from_u64(2), p), S, p);
    
    ecc_int Y_sq_S_sq = mul(mul(P->y, P->y, p), mul(S, S, p), p);
    ecc_int new_y = sub(
        mul(W, sub(mul(B, from_u64(4), p), H, p), p),
        mul(Y_sq_S_sq, from_u64(8), p),
        p
    );
    
    ecc_int new_z = mul(mul(S, mul(S, S, p), p), from_u64(8), p);
    R->inf = false;
    R->x = new_x; R->y = new_y; R->z = new_z;
}

void sum_projective_neq(ecc_point_projective *R, const ecc_point_projective *P, const ecc_point_projective *Q, const ecc_curve *curve) {
    ecc_int p = curve->F.p;
    // U = Y2 * Z1 - Y1 * Z2
    // V = X2 * Z1 - X1 * Z2
    ecc_int U = sub(
        mul(Q->y, P->z, p),
        mul(P->y, Q->z, p),
        p
    ) ;
    ecc_int V = sub(
        mul(Q->x, P->z, p),
        mul(P->x, Q->z, p),
        p
    );
    ecc_int V2 = mul(V, V, p);
    ecc_int V3 = mul(V2, V, p);
    ecc_int U2 = mul(U, U, p);
    ecc_int Z1Z2 = mul(P->z, Q->z, p);
    ecc_int A = sub(
        mul(U2, Z1Z2, p),
        mul(
            V2,
            sum(
                mul(P->x, Q->z, p),
                mul(Q->x, P->z, p),
                p
            ),
            p
        ),
        p
    );
    ecc_int new_x = mul(V, A, p);
    ecc_int X1v2Z2 = mul(
        P->x,
        mul(V2, Q->z, p),
        p
    );
    ecc_int new_y = sub(
        mul(
            U,
            sub(
                X1v2Z2,
                A,
                p
            ),
            p
        ),
        mul(
            V3,
            mul(P->y, Q->z, p),
            p
        ),
        p
    );
    ecc_int new_z = mul(V3, Z1Z2, p);
    R->inf = false;
    R->x = new_x; R->y = new_y; R->z = new_z;
}

void sum_projective(
    ecc_point_projective *R,
    const ecc_point_projective *P,
    const ecc_point_projective *Q,
    const ecc_curve *curve
) {
    ecc_int p = curve->F.p;
    
    if (P->inf || equal(P->z, from_u64(0))) { *R = *Q; return; }
    if (Q->inf || equal(Q->z, from_u64(0))) { *R = *P; return; }

    // P == Q iff x1*z2 == x2*z1 и y1*z2 == y2*z1
    ecc_int P_x_Q_z = mul(P->x, Q->z, p);
    ecc_int Q_x_P_z = mul(Q->x, P->z, p);
    
    if (equal(P_x_Q_z, Q_x_P_z)) {
        ecc_int P_y_Q_z = mul(P->y, Q->z, p);
        ecc_int Q_y_P_z = mul(Q->y, P->z, p);
        
        if (equal(P_y_Q_z, Q_y_P_z)) {
            // P == Q
            double_projective(R, P, curve);
            return;
        }
        
        if (equal(sum(P_y_Q_z, Q_y_P_z, p), from_u64(0))) {
            // P == -Q
            *R = NULL_POINT_PROJECTIVE;
            return;
        }
    }
    
    sum_projective_neq(R, P, Q, curve);
}

static int get_nth_bit(ecc_int k, int i) {
    if (i < 0 || i >= NUM_LIMBS * 32) {
        return 0;
    }
    int limb = i / 32;
    int bit = i % 32;
    return (k.d[limb] >> bit) & 1;
}

void mul_scalar_projective(
    ecc_point_projective *R,
    const ecc_point_projective *P,
    ecc_int k,
    const ecc_curve *curve
) {
    if (equal(k, from_u64(0)) || P->inf || equal(P->z, from_u64(0))) {
        *R = NULL_POINT_PROJECTIVE;
        return;
    }

    ecc_point_projective Q[2];

    for (int i = NUM_LIMBS - 1; i >= 0; --i) {
        for (int j = 31; j >= 0; --j) {
            
            int k_i = get_nth_bit(k, j);

            ecc_point_projective sum, dbl;
            sum_projective_neq(&sum, &Q[0], &Q[1], curve);
            double_projective(&dbl, &Q[0], curve);

            if (k_i) {
                for (int l = 0; l < NUM_LIMBS; ++l) {
                    Q[0].x.d[l] = sum.x.d[l];
                    Q[0].y.d[l] = sum.y.d[l];
                    Q[1].x.d[l] = dbl.x.d[l];
                    Q[1].y.d[l] = dbl.y.d[l];
                }
            } else {
                for (int l = 0; l < NUM_LIMBS; ++l) {
                    Q[0].x.d[l] = dbl.x.d[l];
                    Q[0].y.d[l] = dbl.y.d[l];
                    Q[1].x.d[l] = sum.x.d[l];
                    Q[1].y.d[l] = sum.y.d[l];
                }
            }
        }
    }
    *R = Q[0];
}

void init_null_points() {
}

void init() {
    init_null_points();
}