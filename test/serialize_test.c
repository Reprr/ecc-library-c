#include "unity.h"
#include "../include/ecc/serialize.h"
#include "../include/ecc/ecdh.h"
#include "../include/ecc/field.h"
#include "../include/ecc/big_int.h"

ecc_curve E;
void setUp(void) {
    ecc_status_code error = init_curve("secp256k1", &E);
    TEST_ASSERT_EQUAL(ECC_OK, error);
}

void tearDown(void) {
}

void test_deserialize_uncompressed_known_vector_G(void) {
    const uint8_t g_bytes[65] = {
        0x04,
        0x79,0xBE,0x66,0x7E,0xF9,0xDC,0xBB,0xAC,0x55,0xA0,0x62,0x95,0xCE,0x87,0x0B,0x07,
        0x02,0x9B,0xFC,0xDB,0x2D,0xCE,0x28,0xD9,0x59,0xF2,0x81,0x5B,0x16,0xF8,0x17,0x98,
        0x48,0x3A,0xDA,0x77,0x26,0xA3,0xC4,0x65,0x5D,0xA4,0xFB,0xFC,0x0E,0x11,0x08,0xA8,
        0xFD,0x17,0xB4,0x48,0xA6,0x85,0x54,0x19,0x9C,0x47,0xD0,0x8F,0xFB,0x10,0xD4,0xB8
    };

    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_OK,
        deserialize_uncompressed_pub_key_sec1(&pb_k, g_bytes, sizeof(g_bytes), &E));

    TEST_ASSERT_TRUE(equal(pb_k.G.x, E.G_affine.x));
    TEST_ASSERT_TRUE(equal(pb_k.G.y, E.G_affine.y));
    TEST_ASSERT_TRUE(equal(pb_k.G.z, from_u64(1)));
    TEST_ASSERT_FALSE(pb_k.G.inf);

    //free(pb_k.G);
}

void test_deserialize_uncompressed_round_trip(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k, &pb_k, &E));

    serialized_pub_key_uncompressed buf;
    TEST_ASSERT_EQUAL(ECC_OK, serialize_uncompressed_pub_key_sec1(&pb_k, &buf, &E));

    ecc_public_key restored;
    TEST_ASSERT_EQUAL(ECC_OK,
        deserialize_uncompressed_pub_key_sec1(&restored, buf.data, sizeof(buf.data), &E));

    // TEST_ASSERT_TRUE(equal(restored.G.x, pb_k.G.x));
    // TEST_ASSERT_TRUE(equal(restored.G.y, pb_k.G.y));

    ecc_point_affine orig_aff;
    projective_to_affine(&orig_aff, &pb_k.G, E.F.p);

    TEST_ASSERT_TRUE(equal(restored.G.x, orig_aff.x));
    TEST_ASSERT_TRUE(equal(restored.G.y, orig_aff.y));
    TEST_ASSERT_TRUE(equal(restored.G.z, from_u64(1)));

    //free(pr_k.n); //free(pb_k.G); //free(restored.G);
}

void test_deserialize_uncompressed_rejects_bad_prefix(void) {
    uint8_t buf[65] = {0x05};
    buf[32] = 0x01; buf[64] = 0x01;

    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_INVALID_PARAMS,
        deserialize_uncompressed_pub_key_sec1(&pb_k, buf, sizeof(buf), &E));
}

void test_deserialize_uncompressed_rejects_bad_len(void) {
    uint8_t buf[65] = {0x04};

    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_INVALID_PARAMS,
        deserialize_uncompressed_pub_key_sec1(&pb_k, buf, 64, &E));
}

void test_deserialize_uncompressed_rejects_not_on_curve(void) {
    uint8_t buf[65] = {0x04};
    buf[32] = 0x01;   // x = 1;
    buf[64] = 0x01;   // y = 1; 

    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_INVALID_PARAMS,
        deserialize_uncompressed_pub_key_sec1(&pb_k, buf, sizeof(buf), &E));
}

void test_deserialize_uncompressed_rejects_x_ge_p(void) {
    uint8_t buf[65];
    memset(buf, 0xFF, sizeof(buf));   // x = y = 0xFF..FF > p
    buf[0] = 0x04;

    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_INVALID_PARAMS,
        deserialize_uncompressed_pub_key_sec1(&pb_k, buf, sizeof(buf), &E));
}

void test_deserialize_compressed_round_trip(void) {
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_OK, generate_key_pair(&pr_k, &pb_k, &E));

    serialized_pub_key_compressed buf;
    TEST_ASSERT_EQUAL(ECC_OK, serialize_compressed_pub_key_sec1(&pb_k, &buf, &E));

    ecc_public_key restored;
    TEST_ASSERT_EQUAL(ECC_OK,
        deserialize_compressed_pub_key_sec1(&restored, buf.data, sizeof(buf.data), &E));

    ecc_point_affine orig_aff;
    projective_to_affine(&orig_aff, &pb_k.G, E.F.p);

    TEST_ASSERT_TRUE(equal(restored.G.x, orig_aff.x));
    TEST_ASSERT_TRUE(equal(restored.G.y, orig_aff.y));

    //free(pr_k.n); //free(pb_k.G); //free(restored.G);
}

void test_deserialize_compressed_known_vector_G(void) {
    const uint8_t g_bytes[33] = {
        0x02,
        0x79,0xBE,0x66,0x7E,0xF9,0xDC,0xBB,0xAC,0x55,0xA0,0x62,0x95,0xCE,0x87,0x0B,0x07,
        0x02,0x9B,0xFC,0xDB,0x2D,0xCE,0x28,0xD9,0x59,0xF2,0x81,0x5B,0x16,0xF8,0x17,0x98
    };
    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_OK,
        deserialize_compressed_pub_key_sec1(&pb_k, g_bytes, sizeof(g_bytes), &E));
    TEST_ASSERT_TRUE(equal(pb_k.G.x, E.G_affine.x));
    TEST_ASSERT_TRUE(equal(pb_k.G.y, E.G_affine.y));
    //free(pb_k.G);
}

void test_deserialize_compressed_rejects_x_ge_p(void) {
    uint8_t buf[33] = {0x02};
    memset(buf + 1, 0xFF, 32);   // x = 0xFF..FF > p

    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_INVALID_PARAMS,
        deserialize_compressed_pub_key_sec1(&pb_k, buf, sizeof(buf), &E));
}

void test_deserialize_compressed_rejects_bad_prefix(void) {
    uint8_t buf[33] = {0x04};   // ! 02/03
    ecc_public_key pb_k;
    TEST_ASSERT_EQUAL(ECC_INVALID_PARAMS,
        deserialize_compressed_pub_key_sec1(&pb_k, buf, sizeof(buf), &E));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_deserialize_uncompressed_known_vector_G);
    RUN_TEST(test_deserialize_uncompressed_round_trip);
    RUN_TEST(test_deserialize_uncompressed_rejects_bad_prefix);
    RUN_TEST(test_deserialize_uncompressed_rejects_bad_len);
    RUN_TEST(test_deserialize_uncompressed_rejects_not_on_curve);
    RUN_TEST(test_deserialize_uncompressed_rejects_x_ge_p);

    RUN_TEST(test_deserialize_compressed_round_trip);
    RUN_TEST(test_deserialize_compressed_known_vector_G);
    RUN_TEST(test_deserialize_compressed_rejects_x_ge_p);
    RUN_TEST(test_deserialize_compressed_rejects_bad_prefix);

    return UNITY_END();
}