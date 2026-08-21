#include "unity.h"
#include "../include/ecc/big_int.h"
#include "../include/ecc/types.h"

ecc_int p;
ecc_int p_large;

void setUp(void) {
    p = from_u64(17);
    
    // secp256k1
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


void test_equal_same_values(void) {
    ecc_int a = from_u64(42);
    ecc_int b = from_u64(42);
    TEST_ASSERT_TRUE(equal(a, b));
}

void test_equal_different_values(void) {
    ecc_int a = from_u64(42);
    ecc_int b = from_u64(43);
    TEST_ASSERT_FALSE(equal(a, b));
}

void test_cmp_less(void) {
    ecc_int a = from_u64(5);
    ecc_int b = from_u64(10);
    TEST_ASSERT_LESS_THAN(0, cmp(a, b));
}

void test_cmp_greater(void) {
    ecc_int a = from_u64(10);
    ecc_int b = from_u64(5);
    TEST_ASSERT_GREATER_THAN(0, cmp(a, b));
}

void test_cmp_equal(void) {
    ecc_int a = from_u64(10);
    ecc_int b = from_u64(10);
    TEST_ASSERT_EQUAL(0, cmp(a, b));
}


void test_sum_no_overflow(void) {
    ecc_int a = from_u64(5);
    ecc_int b = from_u64(7);
    ecc_int result = sum(a, b, p);
    ecc_int expected = from_u64(12);
    TEST_ASSERT_TRUE(equal(result, expected));
}

void test_sum_with_overflow(void) {
    ecc_int a = from_u64(10);
    ecc_int b = from_u64(10);
    ecc_int result = sum(a, b, p);
    ecc_int expected = from_u64(3); // 20 mod 17 = 3
    TEST_ASSERT_TRUE(equal(result, expected));
}


void test_sub_no_borrow(void) {
    ecc_int a = from_u64(10);
    ecc_int b = from_u64(3);
    ecc_int result = sub(a, b, p);
    ecc_int expected = from_u64(7);
    TEST_ASSERT_TRUE(equal(result, expected));
}

void test_sub_with_borrow(void) {
    ecc_int a = from_u64(3);
    ecc_int b = from_u64(10);
    ecc_int result = sub(a, b, p);
    ecc_int expected = from_u64(10); // 3 - 10 + 17 = 10
    TEST_ASSERT_TRUE(equal(result, expected));
}


void test_shift_right_1_even(void) {
    ecc_int a = from_u64(10);
    ecc_int result = shift_right_1(a);
    ecc_int expected = from_u64(5);
    TEST_ASSERT_TRUE(equal(result, expected));
}

void test_shift_right_1_odd(void) {
    ecc_int a = from_u64(11);
    ecc_int result = shift_right_1(a);
    ecc_int expected = from_u64(5); // 11 >> 1 = 5
    TEST_ASSERT_TRUE(equal(result, expected));
}


void test_mul_scalar_basic(void) {
    ecc_int a = from_u64(3);
    ecc_int n = from_u64(5);
    ecc_int result = mul_scalar(a, n, p);
    ecc_int expected = from_u64(15);
    TEST_ASSERT_TRUE(equal(result, expected));
}

void test_mul_scalar_with_mod(void) {
    ecc_int a = from_u64(3);
    ecc_int n = from_u64(7);
    ecc_int result = mul_scalar(a, n, p);
    ecc_int expected = from_u64(4); // 21 mod 17 = 4
    TEST_ASSERT_TRUE(equal(result, expected));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_equal_same_values);
    RUN_TEST(test_equal_different_values);
    RUN_TEST(test_cmp_less);
    RUN_TEST(test_cmp_greater);
    RUN_TEST(test_cmp_equal);
    RUN_TEST(test_sum_no_overflow);
    RUN_TEST(test_sum_with_overflow);
    RUN_TEST(test_sub_no_borrow);
    RUN_TEST(test_sub_with_borrow);
    RUN_TEST(test_shift_right_1_even);
    RUN_TEST(test_shift_right_1_odd);
    RUN_TEST(test_mul_scalar_basic);
    RUN_TEST(test_mul_scalar_with_mod);
    
    return UNITY_END();
}