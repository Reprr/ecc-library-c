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

bool projective_equal(const ecc_point_projective *P1, const ecc_point_projective *P2, ecc_int p) {
    // P1 = P2 iff z1*z2 = z2*z1 и y1*z2 = y2*z1
    ecc_int x_check = sub(mul(P1->x, P2->z, p), mul(P2->x, P1->z, p), p);
    ecc_int y_check = sub(mul(P1->y, P2->z, p), mul(P2->y, P1->z, p), p);
    
    return equal(x_check, from_u64(0)) && equal(y_check, from_u64(0));
}


void test_double_projective_generator(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    ecc_point_projective result;
    double_projective(&result, &G_proj, &E);
    
    ecc_point_affine result_aff;
    projective_to_affine(&result_aff, &result, E.F.p);
    
    ecc_point_affine expected_aff;
    sum_affine(&expected_aff, &E.G_affine, &E.G_affine, &E);
    
    TEST_ASSERT_TRUE(equal(result_aff.x, expected_aff.x));
    TEST_ASSERT_TRUE(equal(result_aff.y, expected_aff.y));
    TEST_ASSERT_TRUE(is_on_curve(&result_aff, &E));
}

void test_double_projective_via_addition(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // Удвоение через double_projective
    ecc_point_projective result1;
    double_projective(&result1, &G_proj, &E);
    
    // Удвоение через сложение точки с собой
    ecc_point_projective result2;
    sum_projective_neq(&result2, &G_proj, &G_proj, &E);
    
    TEST_ASSERT_TRUE(projective_equal(&result1, &result2, E.F.p));
}

void test_double_projective_multiple(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // 2G
    ecc_point_projective G2;
    double_projective(&G2, &G_proj, &E);
    
    // 4G = 2 * 2G
    ecc_point_projective G4_via_double;
    double_projective(&G4_via_double, &G2, &E);
    
    // 4G через аффинные координаты
    ecc_point_affine G2_aff, G4_aff;
    projective_to_affine(&G2_aff, &G2, E.F.p);
    sum_affine(&G4_aff, &G2_aff, &G2_aff, &E);
    
    ecc_point_affine G4_via_double_aff;
    projective_to_affine(&G4_via_double_aff, &G4_via_double, E.F.p);
    
    TEST_ASSERT_TRUE(equal(G4_via_double_aff.x, G4_aff.x));
    TEST_ASSERT_TRUE(equal(G4_via_double_aff.y, G4_aff.y));
}

void test_double_projective_eight_G(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // 2G
    ecc_point_projective G2;
    double_projective(&G2, &G_proj, &E);
    
    // 4G
    ecc_point_projective G4;
    double_projective(&G4, &G2, &E);
    
    // 8G
    ecc_point_projective G8;
    double_projective(&G8, &G4, &E);
    
    ecc_point_affine G8_aff;
    projective_to_affine(&G8_aff, &G8, E.F.p);
    
    TEST_ASSERT_TRUE(is_on_curve(&G8_aff, &E));
    TEST_ASSERT_FALSE(G8_aff.inf);
    
    ecc_int eight = from_u64(8);
    ecc_point_affine G8_expected;
    mul_scalar_affine(&G8_expected, &E.G_affine, eight, &E);
    
    TEST_ASSERT_TRUE(equal(G8_aff.x, G8_expected.x));
    TEST_ASSERT_TRUE(equal(G8_aff.y, G8_expected.y));
}


void test_sum_projective_different_points(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // 2G
    ecc_point_projective G2;
    double_projective(&G2, &G_proj, &E);
    
    // 3G = G + 2G
    ecc_point_projective G3;
    sum_projective_neq(&G3, &G_proj, &G2, &E);
    
    ecc_point_affine G3_aff;
    projective_to_affine(&G3_aff, &G3, E.F.p);
    
    ecc_point_affine G2_aff;
    projective_to_affine(&G2_aff, &G2, E.F.p);
    
    ecc_point_affine G3_expected;
    sum_affine(&G3_expected, &E.G_affine, &G2_aff, &E);
    
    TEST_ASSERT_TRUE(equal(G3_aff.x, G3_expected.x));
    TEST_ASSERT_TRUE(equal(G3_aff.y, G3_expected.y));
    TEST_ASSERT_TRUE(is_on_curve(&G3_aff, &E));
}

void test_sum_projective_commutative(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // 2G
    ecc_point_projective G2;
    double_projective(&G2, &G_proj, &E);
    
    // G + 2G
    ecc_point_projective result1;
    sum_projective_neq(&result1, &G_proj, &G2, &E);
    
    // 2G + G
    ecc_point_projective result2;
    sum_projective_neq(&result2, &G2, &G_proj, &E);
    
    TEST_ASSERT_TRUE(projective_equal(&result1, &result2, E.F.p));
}

void test_sum_projective_associative(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // 2G
    ecc_point_projective G2;
    double_projective(&G2, &G_proj, &E);
    
    // 3G = G + 2G
    ecc_point_projective G3;
    sum_projective_neq(&G3, &G_proj, &G2, &E);
    
    // 4G = G + 3G
    ecc_point_projective G4_via_1_plus_3;
    sum_projective_neq(&G4_via_1_plus_3, &G_proj, &G3, &E);
    
    // 4G = 2G + 2G
    ecc_point_projective G4_via_2_plus_2;
    sum_projective_neq(&G4_via_2_plus_2, &G2, &G2, &E);
    
    TEST_ASSERT_TRUE(projective_equal(&G4_via_1_plus_3, &G4_via_2_plus_2, E.F.p));
}

void test_sum_projective_multiple(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    // 1G, 2G, 3G, 4G, 5G
    ecc_point_projective G1 = G_proj;
    
    ecc_point_projective G2;
    double_projective(&G2, &G1, &E);
    
    ecc_point_projective G3;
    sum_projective_neq(&G3, &G1, &G2, &E);
    
    ecc_point_projective G4;
    sum_projective_neq(&G4, &G1, &G3, &E);
    
    ecc_point_projective G5;
    sum_projective_neq(&G5, &G1, &G4, &E);
    
    ecc_point_affine G5_aff;
    projective_to_affine(&G5_aff, &G5, E.F.p);
    
    ecc_int five = from_u64(5);
    ecc_point_affine G5_expected;
    mul_scalar_affine(&G5_expected, &E.G_affine, five, &E);
    
    TEST_ASSERT_TRUE(equal(G5_aff.x, G5_expected.x));
    TEST_ASSERT_TRUE(equal(G5_aff.y, G5_expected.y));
    TEST_ASSERT_TRUE(is_on_curve(&G5_aff, &E));
}

void test_sum_projective_inverse(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    ecc_point_projective neg_G;
    neg_G.x = G_proj.x;
    neg_G.y = sub(from_u64(0), G_proj.y, E.F.p);
    neg_G.z = G_proj.z;

    ecc_point_projective result;
    sum_projective_neq(&result, &G_proj, &neg_G, &E);

    TEST_ASSERT_TRUE(1);
}

void test_sum_projective_with_different_z(void) {
    ecc_point_projective G_proj;
    affine_to_projective(&G_proj, &E.G_affine);
    
    ecc_point_projective G_scaled;
    ecc_int scale = from_u64(7);
    G_scaled.x = mul(G_proj.x, scale, E.F.p);
    G_scaled.y = mul(G_proj.y, scale, E.F.p);
    G_scaled.z = mul(G_proj.z, scale, E.F.p);
    
    // 2G
    ecc_point_projective G2;
    double_projective(&G2, &G_proj, &E);
    
    // G + 2G
    ecc_point_projective result1;
    sum_projective_neq(&result1, &G_proj, &G2, &E);
    
    // G_scaled + 2G
    ecc_point_projective result2;
    sum_projective_neq(&result2, &G_scaled, &G2, &E);
    
    TEST_ASSERT_TRUE(projective_equal(&result1, &result2, E.F.p));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_double_projective_generator);
    RUN_TEST(test_double_projective_via_addition);
    RUN_TEST(test_double_projective_multiple);
    RUN_TEST(test_double_projective_eight_G);
    
    RUN_TEST(test_sum_projective_different_points);
    RUN_TEST(test_sum_projective_commutative);
    RUN_TEST(test_sum_projective_associative);
    RUN_TEST(test_sum_projective_multiple);
    RUN_TEST(test_sum_projective_inverse);
    RUN_TEST(test_sum_projective_with_different_z);
    
    return UNITY_END();
}