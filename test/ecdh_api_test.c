#include "unity.h"
#include "../include/ecc/ecdh.h"
#include "../include/ecc/types.h"
#include "../include/ecc/field.h"
#include <string.h>

ecc_curve E;

void setUp(void) {
    ecc_status_code error = init_curve("secp256k1", &E);
    TEST_ASSERT_EQUAL(ECC_OK, error);
}

void tearDown(void) {
}

// Тесты инициализации кривой
void test_init_curve_secp256k1(void) {
    ecc_curve curve;
    ecc_status_code error = init_curve("secp256k1", &curve);
    TEST_ASSERT_EQUAL(ECC_OK, error);
    TEST_ASSERT_NOT_NULL(curve.G_affine);
    // TEST_ASSERT_NOT_NULL(curve.G_projective);
}

void test_init_curve_invalid(void) {
    ecc_curve curve;
    ecc_status_code error = init_curve("invalid_curve", &curve);
    TEST_ASSERT_NOT_EQUAL(ECC_OK, error);
}

// Тесты генерации приватного ключа
void test_generate_private_key_valid(void) {
    ecc_private_key pr_k;
    pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    *pr_k.n = from_u64(0);
    
    ecc_status_code error = generate_private_key(&pr_k);
    TEST_ASSERT_EQUAL(ECC_OK, error);
    
    // Ключ должен быть в диапазоне [1, N-1]
    ecc_int zero = from_u64(0);
    TEST_ASSERT_GREATER_THAN(0, cmp(*(pr_k.n), zero));
    TEST_ASSERT_LESS_THAN(0, cmp(*(pr_k.n), E.N));
    
    free(pr_k.n);
}

void test_generate_private_key_randomness(void) {
    ecc_private_key pr_k1, pr_k2;
    pr_k1.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k2.n = (ecc_int *)malloc(sizeof(ecc_int));
    *pr_k1.n = from_u64(0);
    *pr_k2.n = from_u64(0);
    
    generate_private_key(&pr_k1);
    generate_private_key(&pr_k2);
    
    // Два последовательных ключа должны быть разными
    TEST_ASSERT_FALSE(equal(*(pr_k1.n), *(pr_k2.n)));
    
    free(pr_k1.n);
    free(pr_k2.n);
}

// Тесты вычисления публичного ключа
void test_calculate_public_key_valid(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pb_k.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    *pr_k.n = from_u64(0);
    
    generate_private_key(&pr_k);
    
    ecc_status_code error = calculate_public_key(&pr_k, &E, &pb_k);
    TEST_ASSERT_EQUAL(ECC_OK, error);
    
    // Публичный ключ должен быть точкой на кривой
    TEST_ASSERT_TRUE(is_on_curve(*(pb_k.G), E));
    TEST_ASSERT_FALSE(pb_k.G->inf);
    
    free(pr_k.n);
    free(pb_k.G);
}

void test_calculate_public_key_deterministic(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k1, pb_k2;
    pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pb_k1.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    pb_k2.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    
    // Фиксированный приватный ключ
    *pr_k.n = from_u64(12345);
    
    calculate_public_key(&pr_k, &E, &pb_k1);
    calculate_public_key(&pr_k, &E, &pb_k2);
    
    // Один и тот же приватный ключ должен давать один и тот же публичный
    TEST_ASSERT_TRUE(equal(pb_k1.G->x, pb_k2.G->x));
    TEST_ASSERT_TRUE(equal(pb_k1.G->y, pb_k2.G->y));
    
    free(pr_k.n);
    free(pb_k1.G);
    free(pb_k2.G);
}

// Тесты ECDH - общий секретный ключ
void test_calculate_general_private_key(void) {
    // Создаем две пары ключей
    ecc_private_key pr_k_A, pr_k_B;
    ecc_public_key pb_k_A, pb_k_B;
    ecc_general_private_key gen_pr_k_A, gen_pr_k_B;
    
    pr_k_A.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k_B.n = (ecc_int *)malloc(sizeof(ecc_int));
    pb_k_A.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    pb_k_B.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    gen_pr_k_A.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    gen_pr_k_B.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    
    generate_private_key(&pr_k_A);
    generate_private_key(&pr_k_B);
    
    calculate_public_key(&pr_k_A, &E, &pb_k_A);
    calculate_public_key(&pr_k_B, &E, &pb_k_B);
    
    // Вычисляем общий секретный ключ
    ecc_status_code error_A = calculate_general_private_key(&pr_k_A, &pb_k_B, &E, &gen_pr_k_A);
    ecc_status_code error_B = calculate_general_private_key(&pr_k_B, &pb_k_A, &E, &gen_pr_k_B);
    
    TEST_ASSERT_EQUAL(ECC_OK, error_A);
    TEST_ASSERT_EQUAL(ECC_OK, error_B);
    
    // Оба должны получить одинаковый общий секрет
    TEST_ASSERT_TRUE(equal(gen_pr_k_A.G->x, gen_pr_k_B.G->x));
    TEST_ASSERT_TRUE(equal(gen_pr_k_A.G->y, gen_pr_k_B.G->y));
    
    // Общий секрет должен быть точкой на кривой
    TEST_ASSERT_TRUE(is_on_curve(*(gen_pr_k_A.G), E));
    
    free(pr_k_A.n);
    free(pr_k_B.n);
    free(pb_k_A.G);
    free(pb_k_B.G);
    free(gen_pr_k_A.G);
    free(gen_pr_k_B.G);
}

// Тест с известными значениями
void test_ecdh_known_values(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pb_k.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    
    // Устанавливаем приватный ключ = 1
    *pr_k.n = from_u64(1);
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    // Публичный ключ должен быть равен базовой точке G
    TEST_ASSERT_TRUE(equal(pb_k.G->x, E.G_affine->x));
    TEST_ASSERT_TRUE(equal(pb_k.G->y, E.G_affine->y));
    
    free(pr_k.n);
    free(pb_k.G);
}

// Тест на краевые случаи
void test_private_key_one(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pb_k.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    
    *pr_k.n = from_u64(1);
    
    ecc_status_code error = calculate_public_key(&pr_k, &E, &pb_k);
    TEST_ASSERT_EQUAL(ECC_OK, error);
    TEST_ASSERT_TRUE(is_on_curve(*(pb_k.G), E));
    
    free(pr_k.n);
    free(pb_k.G);
}

void test_private_key_two(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pb_k.G = (ecc_point_affine *)malloc(sizeof(ecc_point_affine));
    
    *pr_k.n = from_u64(2);
    
    ecc_status_code error = calculate_public_key(&pr_k, &E, &pb_k);
    TEST_ASSERT_EQUAL(ECC_OK, error);
    TEST_ASSERT_TRUE(is_on_curve(*(pb_k.G), E));
    
    // Публичный ключ должен быть равен 2*G
    ecc_point_affine expected = sum_affine(*(E.G_affine), *(E.G_affine), E);
    TEST_ASSERT_TRUE(equal(pb_k.G->x, expected.x));
    TEST_ASSERT_TRUE(equal(pb_k.G->y, expected.y));
    
    free(pr_k.n);
    free(pb_k.G);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_init_curve_secp256k1);
    RUN_TEST(test_init_curve_invalid);
    RUN_TEST(test_generate_private_key_valid);
    RUN_TEST(test_generate_private_key_randomness);
    RUN_TEST(test_calculate_public_key_valid);
    RUN_TEST(test_calculate_public_key_deterministic);
    RUN_TEST(test_calculate_general_private_key);
    RUN_TEST(test_ecdh_known_values);
    RUN_TEST(test_private_key_one);
    RUN_TEST(test_private_key_two);
    
    return UNITY_END();
}
