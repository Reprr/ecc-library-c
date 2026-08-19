#include "../include/ecc/sha_256.h"

void sha256_init(sha256_ctx *ctx) {
    ctx->data_len = ctx->bit_len = 0;
    static const uint32_t H0[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    memcpy(ctx->state, H0, sizeof(H0));
}

void sha256_update(const uint8_t *msg, size_t msg_len, sha256_ctx *ctx) {
    for (size_t i = 0; i < msg_len; ++i) {
        ctx->data[ctx->data_len++] = msg[i];
        ctx->bit_len += 8;
        if (ctx->data_len == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->data_len = 0;
        }
    }
    // sha256_finalize(ctx, ctx->data);
}

void sha256_transform(sha256_ctx *ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, f, g, h, W[64], T_1, T_2;
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    // for (size_t i = )

    for (size_t t = 0; t < 64; ++t) {
        if (t <= 15)
            W[t] =  ((uint32_t)data[t*4]<<24)   |
                    ((uint32_t)data[t*4+1]<<16) |
                    ((uint32_t)data[t*4+2]<<8)  |
                    ((uint32_t)data[t*4+3])     ;
        else
            W[t] = SIG_SMALL_1(W[t-2]) + W[t-7] + SIG_SMALL_0(W[t-15]) + W[t-16];
        T_1 = h + SIG_BIG_1(e) + Ch(e, f, g) + K[t] + W[t];
        T_2 = SIG_BIG_0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + T_1;
        d = c; c = b; b = a; a = T_1 + T_2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_finalize(sha256_ctx *ctx, uint8_t hash[32]) {
    size_t i = ctx->data_len;
    if (ctx->data_len < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->data[56] = ctx->bit_len >> 56;
    ctx->data[57] = ctx->bit_len >> 48;
    ctx->data[58] = ctx->bit_len >> 40;
    ctx->data[59] = ctx->bit_len >> 32;
    ctx->data[60] = ctx->bit_len >> 24;
    ctx->data[61] = ctx->bit_len >> 16;
    ctx->data[62] = ctx->bit_len >> 8;
    ctx->data[63] = ctx->bit_len;
    sha256_transform(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            hash[i + j * 4] = (ctx->state[j] >> (24 - i * 8)) & 0xff;
        }
    }
}
