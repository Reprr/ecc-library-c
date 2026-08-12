#ifndef ECDH_H
#define ECDH_H

#include <stdlib.h>
#include <sys/random.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "big_int.h"
#include "field.h"

void print_ecc_status_code(ecc_status_code code);
ecc_status_code init_curve(char *curve_ident, ecc_curve *curve);
ecc_status_code generate_private_key(ecc_private_key *pr_k, const ecc_curve *curve);
ecc_status_code calculate_public_key (ecc_private_key *pr_k, ecc_curve *curve, ecc_public_key *pb_k);
ecc_status_code calculate_general_private_key (ecc_private_key *pr_k, ecc_public_key *pb_k, ecc_curve *curve, ecc_general_private_key *gen_pr_k);
void free_curve(ecc_curve *curve);
bool validate_public_key (ecc_public_key *pb_k, ecc_curve *curve);
ecc_status_code generate_key_pair (ecc_private_key *pr_k, ecc_public_key *pb_k, ecc_curve *E);

#endif