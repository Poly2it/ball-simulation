#include "primitives.h"


typedef struct {
    f32 x;
    f32 y;
} V2;


V2 add_v2(V2 a, V2 b);
V2 sub_v2(V2 a, V2 b);
V2 mul_v2(V2 a, V2 b);
V2 div_v2(V2 a, V2 b);


V2 add_v2_f32(V2 a, f32 b);
V2 sub_v2_f32(V2 a, f32 b);
V2 mul_v2_f32(V2 a, f32 b);
V2 div_v2_f32(V2 a, f32 b);

