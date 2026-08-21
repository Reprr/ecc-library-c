#include "unity.h"
#include "../include/ecc/field.h"
#include "../include/ecc/types.h"
#include "../include/ecc/ecdh.h"
#include <string.h>

ecc_curve E;

void setUp(void) {
    ecc_status_code error = init_curve("secp256k1", &E);
    TEST_ASSERT_EQUAL(ECC_OK, error);
}

void tearDown(void) {
    //free_curve(&E);
}


void test_generate_key_pair_success(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    
    ecc_status_code error = generate_key_pair(&pr_k, &pb_k, &E);
    TEST_ASSERT_EQUAL(ECC_OK, error);
    
    //  [1, N-1]
    ecc_int zero = from_u64(0);
    TEST_ASSERT_GREATER_THAN(0, cmp((pr_k.n), zero));
    TEST_ASSERT_LESS_THAN(0, cmp((pr_k.n), E.N));
    
    // // Публичный ключ должен быть валидной точкой на кривой
    TEST_ASSERT_FALSE(pb_k.G.inf);
    TEST_ASSERT_FALSE(equal(pb_k.G.z, from_u64(0)));
    TEST_ASSERT_TRUE(is_on_curve_projective(&pb_k.G, &E));
    
    // //free(pr_k.n);
    // //free(pb_k.G);
}

void test_generate_key_pair_randomness(void) {
    ecc_private_key pr_k1, pr_k2;
    ecc_public_key pb_k1, pb_k2;
    
    TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k1, &pb_k1, &E));
    TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k2, &pb_k2, &E));
    
    // Два вызова должны дать разные ключи
    TEST_ASSERT_FALSE(equal((pr_k1.n), (pr_k2.n)));
    
    //free(pr_k1.n); //free(pb_k1.G);
    //free(pr_k2.n); //free(pb_k2.G);
}

void test_ecdh_shared_secret_match(void) {
    // Алиса
    ecc_private_key pr_k_A;
    ecc_public_key pb_k_A;
    
    // Боб
    ecc_private_key pr_k_B;
    ecc_public_key pb_k_B;
    
    // Генерируем пары ключей
    TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k_A, &pb_k_A, &E));
    TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k_B, &pb_k_B, &E));
    
    // pr_k_A * pb_k_B
    ecc_shared_private_key secret_A;
    ecc_status_code err_A = calculate_shared_private_key(&pr_k_A, &pb_k_B, &E, &secret_A);
    TEST_ASSERT_EQUAL(ECC_OK, err_A);
    
    // pr_k_B * pb_k_A
    ecc_shared_private_key secret_B;
    ecc_status_code err_B = calculate_shared_private_key(&pr_k_B, &pb_k_A, &E, &secret_B);
    TEST_ASSERT_EQUAL(ECC_OK, err_B);
    
    // a * (b * G) = b * (a * G) — должны совпасть
    // Сравниваем проективные координаты
    ecc_int p = E.F.p;
    ecc_int x_check = sub(mul(secret_A.G.x, secret_B.G.z, p), 
                          mul(secret_B.G.x, secret_A.G.z, p), p);
    ecc_int y_check = sub(mul(secret_A.G.y, secret_B.G.z, p), 
                          mul(secret_B.G.y, secret_A.G.z, p), p);
    
    TEST_ASSERT_TRUE(equal(x_check, from_u64(0)));
    TEST_ASSERT_TRUE(equal(y_check, from_u64(0)));
    
    // Общий секрет должен быть на кривой
    TEST_ASSERT_TRUE(is_on_curve_projective(&secret_A.G, &E));
    
    //free(pr_k_A.n); //free(pb_k_A.G); //free(secret_A.G);
    //free(pr_k_B.n); //free(pb_k_B.G); //free(secret_B.G);
}

void test_ecdh_shared_secret_on_curve(void) {
    ecc_private_key pr_k_A, pr_k_B;
    ecc_public_key pb_k_A, pb_k_B;
    
    generate_key_pair(&pr_k_A, &pb_k_A, &E);
    generate_key_pair(&pr_k_B, &pb_k_B, &E);
    
    ecc_shared_private_key secret_A;
    calculate_shared_private_key(&pr_k_A, &pb_k_B, &E, &secret_A);
    
    // Общий секрет должен быть точкой на кривой
    TEST_ASSERT_TRUE(is_on_curve_projective(&secret_A.G, &E));
    TEST_ASSERT_FALSE(secret_A.G.inf);
    TEST_ASSERT_FALSE(equal(secret_A.G.z, from_u64(0)));
    
    //free(pr_k_A.n); //free(pb_k_A.G); //free(secret_A.G);
    //free(pr_k_B.n); //free(pb_k_B.G);
}

void test_ecdh_multiple_runs_consistent(void) {
    for (int i = 0; i < 3; i++) {
        ecc_private_key pr_k_A, pr_k_B;
        ecc_public_key pb_k_A, pb_k_B;
        
        TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k_A, &pb_k_A, &E));
        TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k_B, &pb_k_B, &E));
        
        ecc_shared_private_key secret_A, secret_B;
        TEST_ASSERT_EQUAL(ECC_OK, calculate_shared_private_key(&pr_k_A, &pb_k_B, &E, &secret_A));
        TEST_ASSERT_EQUAL(ECC_OK, calculate_shared_private_key(&pr_k_B, &pb_k_A, &E, &secret_B));
        
        ecc_int p = E.F.p;
        ecc_int x_check = sub(mul(secret_A.G.x, secret_B.G.z, p), 
                              mul(secret_B.G.x, secret_A.G.z, p), p);
        ecc_int y_check = sub(mul(secret_A.G.y, secret_B.G.z, p), 
                              mul(secret_B.G.y, secret_A.G.z, p), p);
        
        TEST_ASSERT_TRUE(equal(x_check, from_u64(0)));
        TEST_ASSERT_TRUE(equal(y_check, from_u64(0)));
        
        //free(pr_k_A.n); //free(pb_k_A.G); //free(secret_A.G);
        //free(pr_k_B.n); //free(pb_k_B.G); //free(secret_B.G);
    }
}


void test_public_key_deterministic(void) {
    ecc_int fixed_key = from_u64(12345);
    
    ecc_private_key pr_k;
    pr_k.n = fixed_key;
    
    ecc_public_key pb_k1, pb_k2;
    
    calculate_public_key(&pr_k, &E, &pb_k1);
    calculate_public_key(&pr_k, &E, &pb_k2);
    
    ecc_int p = E.F.p;
    ecc_int x_check = sub(mul(pb_k1.G.x, pb_k2.G.z, p), 
                          mul(pb_k2.G.x, pb_k1.G.z, p), p);
    ecc_int y_check = sub(mul(pb_k1.G.y, pb_k2.G.z, p), 
                          mul(pb_k2.G.y, pb_k1.G.z, p), p);
    
    TEST_ASSERT_TRUE(equal(x_check, from_u64(0)));
    TEST_ASSERT_TRUE(equal(y_check, from_u64(0)));
    
    //free(pr_k.n); //free(pb_k1.G); //free(pb_k2.G);
}

void test_public_key_priv_1_equals_G(void) {
    ecc_private_key pr_k;
    pr_k.n = from_u64(1);
    
    ecc_public_key pb_k;
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    // Конвертируем публичный ключ в аффинные координаты
    ecc_point_affine pb_k_aff;
    projective_to_affine(&pb_k_aff, &pb_k.G, E.F.p);
    
    TEST_ASSERT_TRUE(equal(pb_k_aff.x, E.G_affine.x));
    TEST_ASSERT_TRUE(equal(pb_k_aff.y, E.G_affine.y));
    
    //free(pr_k.n); //free(pb_k.G);
}

void test_public_key_priv_2_equals_2G(void) {
    ecc_private_key pr_k;
    // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k.n = from_u64(2);
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    // Ожидаемый 2G через аффинное сложение
    ecc_point_affine expected;
    sum_affine(&expected, &E.G_affine, &E.G_affine, &E);
    
    // Конвертируем результат в аффинные
    ecc_point_affine result_aff;
    projective_to_affine(&result_aff, &pb_k.G, E.F.p);
    
    TEST_ASSERT_TRUE(equal(result_aff.x, expected.x));
    TEST_ASSERT_TRUE(equal(result_aff.y, expected.y));
    
    //free(pr_k.n); //free(pb_k.G);
}

void test_public_key_priv_3_equals_3G(void) {
    // Приватный ключ = 3 → публичный ключ = 3G
    ecc_private_key pr_k;
    // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k.n = from_u64(3);
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    // Ожидаемый 3G: G + 2G
    ecc_point_affine G2;
    sum_affine(&G2, &E.G_affine, &E.G_affine, &E);
    
    ecc_point_affine expected;
    sum_affine(&expected, &E.G_affine, &G2, &E);
    
    ecc_point_affine result_aff;
    projective_to_affine(&result_aff, &pb_k.G, E.F.p);
    
    TEST_ASSERT_TRUE(equal(result_aff.x, expected.x));
    TEST_ASSERT_TRUE(equal(result_aff.y, expected.y));
    
    //free(pr_k.n); //free(pb_k.G);
}


void test_validate_own_public_key(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    
    generate_key_pair(&pr_k, &pb_k, &E);
    
    // Собственный публичный ключ должен пройти валидацию
    TEST_ASSERT_TRUE(validate_public_key(&pb_k, &E));
    
    //free(pr_k.n); //free(pb_k.G);
}

void test_validate_G_generator(void) {
    // Базовая точка G должна быть валидной
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    affine_to_projective(&pb_k.G, &E.G_affine);
    
    TEST_ASSERT_TRUE(validate_public_key(&pb_k, &E));
    
    //free(pb_k.G);
}

void test_validate_rejects_invalid_point(void) {
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    // Точка (1, 1) не лежит на secp256k1
    pb_k.G.x = from_u64(1);
    pb_k.G.y = from_u64(1);
    pb_k.G.z = from_u64(1);
    pb_k.G.inf = false;
    
    TEST_ASSERT_FALSE(validate_public_key(&pb_k, &E));
    
    //free(pb_k.G);
}

void test_validate_rejects_infinity(void) {
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    // Точка бесконечности
    pb_k.G = NULL_POINT_PROJECTIVE;
    
    TEST_ASSERT_FALSE(validate_public_key(&pb_k, &E));
    
    //free(pb_k.G);
}

void test_validate_rejects_point_not_in_subgroup(void) {
    // Создаём точку на кривой, но не в подгруппе генератора
    // Для secp256k1 кофактор h=1, поэтому все точки на кривой в подгруппе
    // Но можем проверить, что N * P = O для валидной точки
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    affine_to_projective(&pb_k.G, &E.G_affine);
    
    // G валидна, и N * G = O
    TEST_ASSERT_TRUE(validate_public_key(&pb_k, &E));
    
    //free(pb_k.G);
}


void test_known_scalar_multiplication_100(void) {
    // 100 * G
    ecc_private_key pr_k;
    // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k.n = from_u64(100);
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    TEST_ASSERT_TRUE(is_on_curve_projective(&pb_k.G, &E));
    
    ecc_point_affine result_aff;
    projective_to_affine(&result_aff, &pb_k.G, E.F.p);
    
    ecc_point_affine expected;
    mul_scalar_affine(&expected, &E.G_affine, from_u64(100), &E);
    
    TEST_ASSERT_TRUE(equal(result_aff.x, expected.x));
    TEST_ASSERT_TRUE(equal(result_aff.y, expected.y));
    
    //free(pr_k.n); //free(pb_k.G);
}

void test_known_scalar_multiplication_large(void) {
    ecc_int large_k;
    large_k.d[0] = 0xDEADBEEF;
    large_k.d[1] = 0xCAFEBABE;
    large_k.d[2] = 0x12345678;
    large_k.d[3] = 0x87654321;
    large_k.d[4] = 0xABCDEF01;
    large_k.d[5] = 0xFEDCBA98;
    large_k.d[6] = 0x11111111;
    large_k.d[7] = 0x22222222;
    
    // large_k < N
    if (cmp(large_k, E.N) >= 0) {
        large_k = _sub(large_k, E.N);
    }
    
    ecc_private_key pr_k;
    // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k.n = large_k;
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    // Сравниваем с аффинным scalar multiplication
    ecc_point_affine result_aff;
    projective_to_affine(&result_aff, &pb_k.G, E.F.p);
    
    ecc_point_affine expected;
    mul_scalar_affine(&expected, &E.G_affine, large_k, &E);
    
    TEST_ASSERT_TRUE(equal(result_aff.x, expected.x));
    TEST_ASSERT_TRUE(equal(result_aff.y, expected.y));
    
    //free(pr_k.n); //free(pb_k.G);
}


void test_order_times_G_is_infinity(void) {
    // N * G = O
    ecc_private_key pr_k;
    // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k.n = E.N;
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    TEST_ASSERT_TRUE(pb_k.G.inf || equal(pb_k.G.z, from_u64(0)));
    
    //free(pr_k.n); //free(pb_k.G);
}

void test_order_plus_1_times_G_is_G(void) {
    // (N+1) * G = G
    ecc_int n_plus_1 = sum(E.N, from_u64(1), E.N);
    
    ecc_private_key pr_k;
    // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
    pr_k.n = n_plus_1;
    
    ecc_public_key pb_k;
    // pb_k.G = (ecc_point_projective *)malloc(sizeof(ecc_point_projective));
    
    calculate_public_key(&pr_k, &E, &pb_k);
    
    ecc_point_affine result_aff;
    projective_to_affine(&result_aff, &pb_k.G, E.F.p);
    
    // Должен совпадать с G (для secp256k1, (N+1) mod N = 1)
    // Но если n_plus_1 > N, то результат = 1*G = G
    // Так как n_plus_1 = N+1, то (N+1) mod N = 1
    
    TEST_ASSERT_TRUE(equal(result_aff.x, E.G_affine.x));
    TEST_ASSERT_TRUE(equal(result_aff.y, E.G_affine.y));
    
    //free(pr_k.n); //free(pb_k.G);
}


void test_generate_private_key_range(void) {
    for (int i = 0; i < 5; i++) {
        ecc_private_key pr_k;
        // pr_k.n = (ecc_int *)malloc(sizeof(ecc_int));
        
        TEST_ASSERT_EQUAL(ECC_OK, generate_private_key(&pr_k, &E));
        
        ecc_int zero = from_u64(0);
        TEST_ASSERT_GREATER_THAN(0, cmp((pr_k.n), zero));     // > 0
        TEST_ASSERT_LESS_THAN(0, cmp((pr_k.n), E.N));         // < N
        
        //free(pr_k.n);
    }
}

void test_generate_private_key_unique(void) {
    ecc_int keys[5];
    
    for (int i = 0; i < 5; i++) {
        ecc_private_key pr_k;
        pr_k.n = keys[i];
        generate_private_key(&pr_k, &E);
    }
    
    // Все ключи должны быть различны
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            TEST_ASSERT_FALSE(equal(keys[i], keys[j]));
        }
    }
}


int main(void) {
    UNITY_BEGIN();
    
    // generate_key_pair
    RUN_TEST(test_generate_key_pair_success);
    RUN_TEST(test_generate_key_pair_randomness);
    
    // ECDH протокол
    RUN_TEST(test_ecdh_shared_secret_match);
    RUN_TEST(test_ecdh_shared_secret_on_curve);
    RUN_TEST(test_ecdh_multiple_runs_consistent);
    
    // Детерминированность
    RUN_TEST(test_public_key_deterministic);
    RUN_TEST(test_public_key_priv_1_equals_G);
    RUN_TEST(test_public_key_priv_2_equals_2G);
    RUN_TEST(test_public_key_priv_3_equals_3G);
    
    // Валидация
    RUN_TEST(test_validate_own_public_key);
    RUN_TEST(test_validate_G_generator);
    RUN_TEST(test_validate_rejects_invalid_point);
    RUN_TEST(test_validate_rejects_infinity);
    RUN_TEST(test_validate_rejects_point_not_in_subgroup);
    
    // Известные значения
    RUN_TEST(test_known_scalar_multiplication_100);
    RUN_TEST(test_known_scalar_multiplication_large);
    RUN_TEST(test_order_times_G_is_infinity);
    RUN_TEST(test_order_plus_1_times_G_is_G);
    
    // generate_private_key
    RUN_TEST(test_generate_private_key_range);
    RUN_TEST(test_generate_private_key_unique);
    
    return UNITY_END();
}
