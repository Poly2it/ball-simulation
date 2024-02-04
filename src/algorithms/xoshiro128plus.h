#ifndef XOSHIRO128PLUS_H
#define XOSHIRO128PLUS_H


#include <stdint.h>


#include "../primitives.h"


void random_u32_seed(uint32_t seed[4]);
uint32_t random_u32(void);
void random_u32_jump(void);
void random_u32_long_jump(void);
f32 random_f32_range(f32 min, f32 max);
s32 random_s32_range(s32 min, s32 max);


#endif
