#include <stdlib.h>
#include <stdio.h>
#include "include/ecc/field.h"
#include "include/ecc/types.h"

int main(void) {
    ecc_int a = 1, b = 0, p = 5;
    ecc_field F = { .p = p };
    ecc_curve E = { .F = &F, .a = a, .b = b };

    ecc_point_affine test = { .x = 0, .y = 0, .inf = false };
    if (is_on_curve(test, E))
        printf("Точка (0,0) лежит на кривой\n");

    ecc_point_affine double_point = sum_affine(test, test, E);
    printf("2*(0,0) = (%d,%d), inf=%d\n", double_point.x, double_point.y, double_point.inf);

    ecc_point_affine mult = mul_scalar_affine(test, 3, E);
    printf("3*(0,0) = (%d,%d), inf=%d\n", mult.x, mult.y, mult.inf);

    return 0;
}