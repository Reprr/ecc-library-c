#include <stdlib.h>
#include <stdio.h>
#include "include/ecc/field.h"
#include "include/ecc/types.h"
#include "include/ecc/ecdh.h"

int main(void) {
    ecc_curve E;
    ecc_private_key pr_k;
    ecc_public_key pb_k;
    ecc_status_code error;

    pr_k.n = (ecc_int *) malloc (sizeof (ecc_int));
    *pr_k.n = from_u64(0);
    pb_k.G = (ecc_point_affine *) malloc (sizeof (ecc_point_affine));

    fprintf (stdout, "NUM_LIMBS: %d\n", NUM_LIMBS);

    error = init_curve("secp256k1", &E);
    if (error != ECC_OK) {
        print_ecc_status_code (error);
        exit(EXIT_FAILURE);
    }
    error = generate_private_key(&pr_k);
    if (error != ECC_OK) {
        print_ecc_status_code (error);
        exit(EXIT_FAILURE);
    }
    fprintf (stdout, "private key: ");
    for (int i = 0; i < NUM_LIMBS; ++i)
         fprintf (stdout, "%u ", pr_k.n->d[i]);
    fprintf(stdout, "\n");


    error = calculate_public_key (&pr_k, &E, &pb_k);
    if (error != ECC_OK) {
        print_ecc_status_code (error);
        exit(EXIT_FAILURE);
    }
    fprintf (stdout, "public key:\n");
    fprintf (stdout, "G_x: ");
    for (int i = 0; i < NUM_LIMBS; ++i) {
        fprintf (stdout, "%u ", pb_k.G->x.d[i]);
    } 
    fprintf(stdout, "\n");
    fprintf (stdout, "G_y: ");
    for (int i = 0; i < NUM_LIMBS; ++i) {
        fprintf (stdout, "%u ", pb_k.G->y.d[i]);
    } 
    fprintf(stdout, "\n");
    return 0;
}