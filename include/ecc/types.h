#ifndef ECC_TYPES_H
#define ECC_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// typedef uint64_t ecc_int;

#define NUM_LIMBS 8

typedef struct {
    uint32_t d[NUM_LIMBS];   
} ecc_int;

typedef struct {
    ecc_int low;
    ecc_int high;
} ecc_int_512;

typedef struct {
    ecc_int p;
} ecc_field;


typedef struct {
    ecc_int x;
    ecc_int y;
    bool inf;
} ecc_point_affine;

typedef struct { // Common projective
    ecc_int x;
    ecc_int y;
    ecc_int z;
    bool inf;
} ecc_point_projective;

typedef struct {
    ecc_field *F;

    ecc_int a;
    ecc_int b;

    ecc_point_affine *G_affine;
    ecc_point_projective *G_projective;

    ecc_int N;
    ecc_int h;
} ecc_curve;

typedef enum {
    ECC_OK,
    ECC_INVALID_PARAMS,
    ECC_FAIL,
    ECC_NOT_ON_CURVE
} ecc_status_code;

typedef struct {
    ecc_point_projective *G;
} ecc_public_key;

typedef struct {
    ecc_int *n;
} ecc_private_key;

typedef struct {
    ecc_point_projective *G;
} ecc_general_private_key;

#endif