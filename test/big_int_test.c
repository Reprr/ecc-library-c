#include "unity.h"
#include "../include/ecc/big_int.h"
#include "../include/ecc/types.h"

ecc_int p;
ecc_int p_large;

void setUp(void) {
    p = from_u64(17);

    p_large.d[0] = 0xFFFFFC2F;
    p_large.d[1] = 0xFFFFFFFE;
    p_large.d[2] = 0xFFFFFFFF;
    p_large.d[3] = 0xFFFFFFFF;
    p_large.d[4] = 0xFFFFFFFF;
    p_large.d[5] = 0xFFFFFFFF;
    p_large.d[6] = 0xFFFFFFFF;
    p_large.d[7] = 0xFFFFFFFF;
}

void tearDown(void) {

}

void test_sum_mod(void) {
    ecc_int a = from_u64(1);
    ecc_int b = from_u64(16);
    ecc_int sm = sum(a, b, p);
    ecc_int expected = from_u64(0);
    TEST_ASSERT_TRUE(equal(sm, expected));
}

void test_mul_mod_small(void) {
    // p=17, 5*3 = 15 mod 17 = 15
    ecc_int a = from_u64(5);
    ecc_int b = from_u64(3);
    ecc_int res = mul(a, b, p);
    ecc_int expected = from_u64(15);
    TEST_ASSERT_TRUE(equal(res, expected));
}

void test_mul_mod_wrap(void) {
    // p=17, 4*5 = 20 mod 17 = 3
    ecc_int a = from_u64(4);
    ecc_int b = from_u64(5);
    ecc_int res = mul(a, b, p);
    ecc_int expected = from_u64(3);
    TEST_ASSERT_TRUE(equal(res, expected));
}

void test_mul_mod_zero(void) {
    ecc_int a = from_u64(7);
    ecc_int b = from_u64(0);
    ecc_int res = mul(a, b, p);
    TEST_ASSERT_TRUE(equal(res, from_u64(0)));
}

void test_mul_mod_large(void) {
    // a = p-1, b = p-1 => (p-1)^2 mod p = 1

    ecc_int a = sub(p_large, from_u64(1), p_large); // p-1
    ecc_int b = sub(p_large, from_u64(1), p_large);
    ecc_int res = mul(a, b, p_large);
    ecc_int expected = from_u64(1);
    TEST_ASSERT_TRUE(equal(res, expected));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sum_mod);
    RUN_TEST(test_mul_mod_small);
    RUN_TEST(test_mul_mod_wrap);
    RUN_TEST(test_mul_mod_zero);
    RUN_TEST(test_mul_mod_large);

    return UNITY_END();
}