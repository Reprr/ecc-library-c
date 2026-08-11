#include "unity.h"
#include "../include/ecc/field.h"
#include "../include/ecc/types.h"
#include "../include/ecc/ecdh.h"
#include <stdio.h>

ecc_curve E;

void setUp(void) {
    ecc_status_code error = init_curve("secp256k1", &E);
    TEST_ASSERT_EQUAL(ECC_OK, error);
}

void tearDown(void) {

}

void test_bin_pow_small(void) {
    ecc_int a = from_u64(2);
    ecc_int n = from_u64(10);
    ecc_int p = from_u64(1000);
    ecc_int result = bin_pow(a, n, p);
    ecc_int expected = from_u64(24); // 2^10 = 1024, 1024 mod 1000 = 24
    TEST_ASSERT_TRUE(equal(result, expected));
}

void test_bin_pow_with_mod(void) {
    ecc_int a = from_u64(3);
    ecc_int n = from_u64(5);
    ecc_int p = from_u64(17);
    ecc_int result = bin_pow(a, n, p);
    ecc_int expected = from_u64(5); // 3^5 = 243, 243 mod 17 = 5
    TEST_ASSERT_TRUE(equal(result, expected));
}

void test_bin_pow_zero_exponent(void) {
    ecc_int a = from_u64(7);
    ecc_int n = from_u64(0);
    ecc_int p = from_u64(17);
    ecc_int result = bin_pow(a, n, p);
    ecc_int expected = from_u64(1);
    TEST_ASSERT_TRUE(equal(result, expected));
}

// Тесты inv (модулярный инверс)
void test_inv_basic(void) {
    ecc_int a = from_u64(3);
    ecc_int p = from_u64(11);
    ecc_int result = inv(a, p);
    // 3 * 4 = 12 ≡ 1 (mod 11)
    ecc_int check = mul(a, result, p);
    ecc_int expected = from_u64(1);
    TEST_ASSERT_TRUE(equal(check, expected));
}

void test_inv_identity(void) {
    ecc_int a = from_u64(5);
    ecc_int p = from_u64(17);
    ecc_int result = inv(a, p);
    ecc_int check = mul(a, result, p);
    ecc_int expected = from_u64(1);
    TEST_ASSERT_TRUE(equal(check, expected));
}

void test_is_on_curve_generator(void) {
    TEST_ASSERT_TRUE(is_on_curve(*(E.G_affine), E));
}

void test_is_on_curve_invalid_point(void) {
    ecc_point_affine P;
    P.x = from_u64(1);
    P.y = from_u64(1);
    P.inf = false;
    TEST_ASSERT_FALSE(is_on_curve(P, E));
}

void test_is_on_curve_null_point(void) {
    ecc_point_affine P = { .x = {0}, .y = {0}, .inf = true };
    TEST_ASSERT_TRUE(is_on_curve(P, E));
}

void test_sum_affine_same_points(void) {
    ecc_point_affine G = *(E.G_affine);
    ecc_point_affine result = sum_affine(G, G, E);
    
    TEST_ASSERT_TRUE(is_on_curve(result, E));
    TEST_ASSERT_FALSE(result.inf);
}

void test_sum_affine_with_null(void) {
    ecc_point_affine G = *(E.G_affine);
    ecc_point_affine null_point = { .x = {0}, .y = {0}, .inf = true };
    
    ecc_point_affine result = sum_affine(G, null_point, E);
    TEST_ASSERT_TRUE(equal(result.x, G.x));
    TEST_ASSERT_TRUE(equal(result.y, G.y));
}

void test_sum_affine_inverse_points(void) {
    ecc_point_affine G = *(E.G_affine);
    
    // Создаем инверсную точку: (x, -y mod p)
    ecc_point_affine G_inv;
    G_inv.x = G.x;
    G_inv.y = sub(from_u64(0), G.y, E.F->p);
    G_inv.inf = false;
    
    ecc_point_affine result = sum_affine(G, G_inv, E);
    TEST_ASSERT_TRUE(result.inf); // Должна получиться бесконечно удаленная точка
}

void test_mul_scalar_affine_identity(void) {
    ecc_point_affine G = *(E.G_affine);
    ecc_int one = from_u64(1);
    ecc_point_affine result = mul_scalar_affine(G, one, E);
    
    TEST_ASSERT_TRUE(equal(result.x, G.x));
    TEST_ASSERT_TRUE(equal(result.y, G.y));
}

void test_mul_scalar_affine_two(void) {
    ecc_point_affine G = *(E.G_affine);
    ecc_int two = from_u64(2);
    ecc_point_affine result = mul_scalar_affine(G, two, E);
    
    TEST_ASSERT_TRUE(is_on_curve(result, E));
    TEST_ASSERT_FALSE(result.inf);
}

void test_mul_scalar_affine_order(void) {
    ecc_point_affine G = *(E.G_affine);
    ecc_int n = E.N;
    ecc_point_affine result = mul_scalar_affine(G, n, E);
    
    TEST_ASSERT_TRUE(result.inf); // n * G = inf
}

void test_mul_scalar_affine_order_plus_one(void) {
    ecc_point_affine G = *(E.G_affine);
    ecc_int n_plus_1 = sum(E.N, from_u64(1), E.N);
    ecc_point_affine result = mul_scalar_affine(G, n_plus_1, E);
    
    TEST_ASSERT_TRUE(equal(result.x, G.x));
    TEST_ASSERT_TRUE(equal(result.y, G.y));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bin_pow_small);
    RUN_TEST(test_bin_pow_with_mod);
    RUN_TEST(test_bin_pow_zero_exponent);
    RUN_TEST(test_inv_basic);
    RUN_TEST(test_inv_identity);
    RUN_TEST(test_is_on_curve_generator);
    RUN_TEST(test_is_on_curve_invalid_point);
    RUN_TEST(test_is_on_curve_null_point);
    RUN_TEST(test_sum_affine_same_points);
    RUN_TEST(test_sum_affine_with_null);
    RUN_TEST(test_sum_affine_inverse_points);
    RUN_TEST(test_mul_scalar_affine_identity);
    RUN_TEST(test_mul_scalar_affine_two);
    RUN_TEST(test_mul_scalar_affine_order);
    RUN_TEST(test_mul_scalar_affine_order_plus_one);
    
    return UNITY_END();
}
