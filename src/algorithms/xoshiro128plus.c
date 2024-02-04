/* This C source file has had slight alteration applied for integration
 * with the ball-simulation project */

/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>
#include "../primitives.h"

/* This is xoshiro128+ 1.0, our best and fastest 32-bit generator for 32-bit
   floating-point numbers. We suggest to use its upper bits for
   floating-point generation, as it is slightly faster than xoshiro128**.
   It passes all tests we are aware of except for
   linearity tests, as the lowest four bits have low linear complexity, so
   if low linear complexity is not considered an issue (as it is usually
   the case) it can be used to generate 32-bit outputs, too.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. */


static inline u32 rotl(const u32 x, s32 k) {
    return (x << k) | (x >> (32 - k));
}


static u32 s[4];


void random_u32_seed(u32 seed[4]) {
    *s = *seed;
}


uint32_t random_u32(void) {
    const u32 result = s[0] + s[3];

    const u32 t = s[1] << 9;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 11);

    return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */

void random_u32_jump(void) {
    static const u32 JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

    u32 s0 = 0;
    u32 s1 = 0;
    u32 s2 = 0;
    u32 s3 = 0;
    for(u32 i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
        for(u32 b = 0; b < 32; b++) {
            if (JUMP[i] & UINT32_C(1) << b) {
                s0 ^= s[0];
                s1 ^= s[1];
                s2 ^= s[2];
                s3 ^= s[3];
            }
            random_u32();	
    }

    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
}


/* This is the long-jump function for the generator. It is equivalent to
   2^96 calls to next(); it can be used to generate 2^32 starting points,
   from each of which jump() will generate 2^32 non-overlapping
   subsequences for parallel distributed computations. */

void random_u32_long_jump(void) {
    static const u32 LONG_JUMP[] = { 0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

    u32 s0 = 0;
    u32 s1 = 0;
    u32 s2 = 0;
    u32 s3 = 0;
    for(u32 i = 0; i < sizeof(LONG_JUMP) / sizeof(*LONG_JUMP); i++)
        for(u32 b = 0; b < 32; b++) {
            if (LONG_JUMP[i] & UINT32_C(1) << b) {
                s0 ^= s[0];
                s1 ^= s[1];
                s2 ^= s[2];
                s3 ^= s[3];
            }
            random_u32();	
    }

    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
}


f32 random_f32_range(f32 min, f32 max) {
    return (f32) min + ((f32) random_u32() / (f32) UINT32_MAX * (max - min));
}


s32 random_s32_range(s32 min, s32 max) {
    return min + (random_u32() % (max - min));
}

