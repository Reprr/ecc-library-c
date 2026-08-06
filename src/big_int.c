#include "../include/ecc/big_int.h"
#include <string.h>

ecc_int from_u64(uint64_t val) {
    ecc_int res = {0};
    res.d[0] = (uint32_t)val;
    res.d[1] = (uint32_t)(val >> 32);
    return res;
}

bool equal(ecc_int a, ecc_int b) {
    return memcmp(a.d, b.d, sizeof(ecc_int)) == 0;
}

int cmp(ecc_int a, ecc_int b) {
    for (int i = NUM_LIMBS - 1; i >= 0; --i) {
        if (a.d[i] > b.d[i]) return 1;
        if (a.d[i] < b.d[i]) return -1;
    }
    return 0;
}

int cmp_512(ecc_int_512 a, ecc_int_512 b) {
    int c = cmp(a.high, b.high);
    if (c != 0) return c;
    return cmp(a.low, b.low);
}

ecc_int _sum(ecc_int a, ecc_int b) {
    uint64_t carry = 0;
    ecc_int res = from_u64(0);
    for (int i = 0; i < NUM_LIMBS; ++i) {
        uint64_t sum = (uint64_t)a.d[i] + b.d[i] + carry;
        res.d[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
    return res;
}

ecc_int _sub(ecc_int a, ecc_int b) {
    int64_t borrow = 0;
    ecc_int res = from_u64(0);
    for (int i = 0; i < NUM_LIMBS; ++i) {
        int64_t diff = (int64_t)a.d[i] - b.d[i] - borrow;
        borrow = (diff < 0);
        diff += (1ULL << 32) * borrow;
        res.d[i] = (uint32_t)diff;
    }
    return res;
}

ecc_int sum(ecc_int a, ecc_int b, ecc_int p) {
    ecc_int res = _sum(a, b);
    if (cmp(res, p) >= 0)
        res = _sub(res, p);
    return res;
}

ecc_int sub(ecc_int a, ecc_int b, ecc_int p) {
    if (cmp(a, b) >= 0)
        return _sub(a, b);
    ecc_int diff = _sub(b, a);
    return _sub(p, diff);
}

ecc_int_512 _mul(ecc_int a, ecc_int b) {
    ecc_int_512 res = {0};

    for (int i = 0; i < NUM_LIMBS; ++i) {
        uint64_t carry = 0;
        for (int j = 0; j < NUM_LIMBS; ++j) {
            uint64_t product = (uint64_t)a.d[i] * b.d[j] +
                               ((uint32_t*)&res)[i + j] + carry;
            ((uint32_t*)&res)[i + j] = (uint32_t)product;
            carry = product >> 32;
        }
        int idx = i + NUM_LIMBS;
        while (carry && idx < 2 * NUM_LIMBS) {
            uint64_t sum = (uint64_t)((uint32_t*)&res)[idx] + carry;
            ((uint32_t*)&res)[idx] = (uint32_t)sum;
            carry = sum >> 32;
            idx++;
        }
    }
    return res;
}

ecc_int_512 sub_512(ecc_int_512 a, ecc_int_512 b) {
    ecc_int_512 res = {0};
    int64_t borrow = 0;
    for (int i = 0; i < 2 * NUM_LIMBS; ++i) {
        int64_t diff = (int64_t)((uint32_t*)&a)[i] - (int64_t)((uint32_t*)&b)[i] - borrow;
        borrow = (diff < 0);
        diff += (1LL << 32) * borrow;
        ((uint32_t*)&res)[i] = (uint32_t)diff;
    }
    return res;
}

ecc_int shift_right_1(ecc_int x) {
    ecc_int res = from_u64(0);
    uint64_t carry = 0;
    for (int i = NUM_LIMBS - 1; i >= 0; --i) {
        uint64_t val = (carry << 32) | x.d[i];
        res.d[i] = (uint32_t)(val >> 1);
        carry = val & 1;
    }
    return res;
}

ecc_int mod(ecc_int_512 a, ecc_int p) {
    if (cmp(a.high, from_u64(0)) == 0 && cmp(a.low, p) < 0) {
        return a.low;
    }

    ecc_int left = from_u64(0);
    ecc_int right = from_u64(0);
    for (int i = 0; i < NUM_LIMBS; ++i)
        right.d[i] = 0xFFFFFFFF; // 2^256 - 1

    while (cmp(left, right) <= 0) {
        ecc_int diff = _sub(right, left);
        ecc_int half = shift_right_1(diff);
        ecc_int mid = _sum(left, half);

        ecc_int_512 prod = _mul(p, mid);

        if (cmp_512(prod, a) <= 0) {
            left = _sum(mid, from_u64(1));
        } else {
            right = _sub(mid, from_u64(1));
        }
    }

    ecc_int_512 qp = _mul(p, right);
    ecc_int_512 rem = sub_512(a, qp);
    return rem.low;
}

ecc_int mul(ecc_int a, ecc_int b, ecc_int p) {
    return mod(_mul(a, b), p);
}

ecc_int mul_scalar(ecc_int a, ecc_int n, ecc_int p) {
    if (equal(n, from_u64(0))) return NULL_ECC_INT;
    if (equal(n, from_u64(1))) return a;
    if (n.d[0] == 1) {
        return sum(a, mul_scalar(a, _sub(n, from_u64(1)), p), p);
    }
    ecc_int b = mul_scalar(a, shift_right_1(n), p);
    return sum(b, b, p);
}
