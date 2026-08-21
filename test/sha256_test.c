#include "unity.h"
#include "sha_256.h"
#include <stdlib.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void hash_full(const uint8_t *msg, size_t len, uint8_t out[32]) {
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, msg, len);
    sha256_finalize(&ctx, out);
}

static void assert_sha256(const uint8_t *msg, size_t len, const uint8_t expected[32]) {
    uint8_t digest[32];
    hash_full(msg, len, digest);
    TEST_ASSERT_EQUAL_MEMORY(expected, digest, 32);
}

void test_sha256_empty(void) {
    static const uint8_t exp[32] = {
        0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,
        0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24,
        0x27,0xae,0x41,0xe4,0x64,0x9b,0x93,0x4c,
        0xa4,0x95,0x99,0x1b,0x78,0x52,0xb8,0x55
    };
    assert_sha256((const uint8_t *)"", 0, exp);
}

void test_sha256_abc(void) {
    static const uint8_t exp[32] = {
        0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,
        0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
        0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,
        0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad
    };
    assert_sha256((const uint8_t *)"abc", 3, exp);
}

void test_sha256_448bit_two_blocks(void) {
    static const uint8_t exp[32] = {
        0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,
        0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
        0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,
        0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1
    };
    const char *m = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    assert_sha256((const uint8_t *)m, 56, exp);
}

void test_sha256_896bit(void) {
    static const uint8_t exp[32] = {
        0xcf,0x5b,0x16,0xa7,0x78,0xaf,0x83,0x80,
        0x03,0x6c,0xe5,0x9e,0x7b,0x04,0x92,0x37,
        0x0b,0x24,0x9b,0x11,0xe8,0xf0,0x7a,0x51,
        0xaf,0xac,0x45,0x03,0x7a,0xfe,0xe9,0xd1
    };
    const char *m = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                    "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    assert_sha256((const uint8_t *)m, 112, exp);
}

void test_sha256_million_a(void) {
    static const uint8_t exp[32] = {
        0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,
        0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
        0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,
        0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0
    };
    size_t len = 1000000;
    uint8_t *msg = malloc(len);
    TEST_ASSERT_NOT_NULL(msg);
    memset(msg, 'a', len);
    assert_sha256(msg, len, exp);
    free(msg);
}

void test_sha256_million_a_chunked(void) {
    static const uint8_t exp[32] = {
        0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,
        0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
        0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,
        0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0
    };
    size_t len = 1000000;
    uint8_t *msg = malloc(len);
    TEST_ASSERT_NOT_NULL(msg);
    memset(msg, 'a', len);

    sha256_ctx ctx;
    sha256_init(&ctx);
    for (size_t off = 0; off < len; off += 1000) {
        size_t chunk = (len - off < 1000) ? (len - off) : 1000;
        sha256_update(&ctx, msg + off, chunk);
    }
    uint8_t digest[32];
    sha256_finalize(&ctx, digest);
    TEST_ASSERT_EQUAL_MEMORY(exp, digest, 32);
    free(msg);
}

void test_sha256_padding_boundary_lengths(void) {
    uint8_t msg[128];
    for (size_t i = 0; i < sizeof(msg); ++i) msg[i] = (uint8_t)(i & 0xFF);

    const size_t lens[] = {1, 55, 56, 57, 63, 64, 65, 119, 120, 127, 128};
    for (size_t t = 0; t < sizeof(lens) / sizeof(lens[0]); ++t) {
        size_t len = lens[t];

        uint8_t d_full[32], d_byte[32], d_chunk[32];
        hash_full(msg, len, d_full);

        sha256_ctx ctx;
        sha256_init(&ctx);
        for (size_t i = 0; i < len; ++i)
            sha256_update(&ctx, &msg[i], 1);
        sha256_finalize(&ctx, d_byte);

        sha256_init(&ctx);
        for (size_t i = 0; i < len; i += 7) {
            size_t chunk = (len - i < 7) ? (len - i) : 7;
            sha256_update(&ctx, msg + i, chunk);
        }
        sha256_finalize(&ctx, d_chunk);

        TEST_ASSERT_EQUAL_MEMORY(d_full, d_byte, 32);
        TEST_ASSERT_EQUAL_MEMORY(d_full, d_chunk, 32);
    }
}

// Повторное использование контекста
void test_sha256_ctx_reuse(void) {
    uint8_t d1[32], d2[32];
    sha256_ctx ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)"abc", 3);
    sha256_finalize(&ctx, d1);

    sha256_init(&ctx);   // переинициализация после finalize
    sha256_update(&ctx, (const uint8_t *)"abc", 3);
    sha256_finalize(&ctx, d2);

    TEST_ASSERT_EQUAL_MEMORY(d1, d2, 32);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sha256_empty);
    RUN_TEST(test_sha256_abc);
    RUN_TEST(test_sha256_448bit_two_blocks);
    RUN_TEST(test_sha256_896bit);
    RUN_TEST(test_sha256_million_a);
    RUN_TEST(test_sha256_million_a_chunked);
    RUN_TEST(test_sha256_padding_boundary_lengths);
    RUN_TEST(test_sha256_ctx_reuse);
    return UNITY_END();
}