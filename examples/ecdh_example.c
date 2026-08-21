// Пример: полный ECDH-обмен между Алисой и Бобом + вывод сессионного ключа.
// Сборка: make example && ./build/bin/ecdh_example
#include <stdio.h>
#include <stdlib.h>
#include "ecc/ecdh.h"
#include "ecc/serialize.h"
#include "ecc/field.h"
#include "ecc/sha_256.h"
#include "ecc/KDF.h"

static void print_hex(const char *label, const uint8_t *buf, size_t len) {
    printf("%-12s: ", label);
    for (size_t i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}

static void ecc_int_to_bytes(const ecc_int *x, uint8_t out[32]) {
    for (int i = NUM_LIMBS - 1, k = 0; i >= 0; --i)
        for (int j = 3; j >= 0; --j, ++k)
            out[k] = (uint8_t)(x->d[i] >> (8 * j));
}

int main(void) {
    ecc_curve curve;
    if (init_curve("secp256k1", &curve) != ECC_OK) {
        fprintf(stderr, "init_curve failed\n");
        return 1;
    }

    // 1. Алиса и Боб генерируют ключевые пары
    ecc_private_key alice_priv, bob_priv;
    ecc_public_key  alice_pub,  bob_pub;
    if (generate_key_pair(&alice_priv, &alice_pub, &curve) != ECC_OK) return 1;
    if (generate_key_pair(&bob_priv,  &bob_pub,  &curve) != ECC_OK) return 1;

    // 2. Сериализуем публичные ключи
    serialized_pub_key_compressed alice_wire, bob_wire;
    serialize_compressed_pub_key_sec1(&alice_pub, &alice_wire, &curve);
    serialize_compressed_pub_key_sec1(&bob_pub,   &bob_wire,   &curve);
    print_hex("Alice pub", alice_wire.data, sizeof(alice_wire.data));
    print_hex("Bob pub",   bob_wire.data,   sizeof(bob_wire.data));

    // 3. Десериализуем на другой стороне
    ecc_public_key alice_at_bob, bob_at_alice;
    if (deserialize_compressed_pub_key_sec1(&alice_at_bob, alice_wire.data,
            sizeof(alice_wire.data), &curve) != ECC_OK) return 1;
    if (deserialize_compressed_pub_key_sec1(&bob_at_alice, bob_wire.data,
            sizeof(bob_wire.data), &curve) != ECC_OK) return 1;

    // 4. ECDH: каждая сторона считает общий секрет
    ecc_shared_private_key shared_a, shared_b;
    calculate_shared_private_key(&alice_priv, &bob_at_alice, &curve, &shared_a);
    calculate_shared_private_key(&bob_priv,  &alice_at_bob, &curve, &shared_b);

    // 5. Проверяем, что секреты совпали
    ecc_point_affine za, zb;
    projective_to_affine(&za, &shared_a.G, curve.F.p);
    projective_to_affine(&zb, &shared_b.G, curve.F.p);
    if (!equal(za.x, zb.x)) {
        fprintf(stderr, "shared secret mismatch!\n");
        return 1;
    }
    uint8_t z_bytes[32];
    ecc_int_to_bytes(&za.x, z_bytes);
    print_hex("Shared Z", z_bytes, sizeof(z_bytes));

    // 6. Сессионный ключ
    const uint8_t si[] = "ecdh-example"; // SharedInfo
    uint8_t K[32];
    ansi_x963_kdf_sha256(K, sizeof(K), z_bytes, sizeof(z_bytes), si, sizeof(si) - 1);
    print_hex("Session key", K, sizeof(K));


    printf("ECDH OK\n");
    return 0;
}
