#ifndef BIG_INT_H 
#define BIG_INT_H

#include "types.h"
#include <string.h>

ecc_int from_u64(uint64_t val);
bool equal(ecc_int a, ecc_int b);
int cmp(ecc_int a, ecc_int b);
int cmp_512(ecc_int_512 a, ecc_int_512 b);
ecc_int _sum(ecc_int a, ecc_int b);
ecc_int _sub(ecc_int a, ecc_int b);
ecc_int_512 _sub_512(ecc_int_512 a, ecc_int_512 b);
ecc_int_512 _mul(ecc_int a, ecc_int b);
ecc_int mul_scalar(ecc_int a, ecc_int n, ecc_int p);
ecc_int sum(ecc_int a, ecc_int b, ecc_int p);
ecc_int sub(ecc_int a, ecc_int b, ecc_int p);
ecc_int mod(ecc_int_512 a, ecc_int p);
ecc_int mul(ecc_int a, ecc_int b, ecc_int p);
ecc_int shift_right_1(ecc_int x);

#endif