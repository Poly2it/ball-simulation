#include "primitives.h"


#include "vector.h"


V2 add_v2(V2 a, V2 b) {
    return (V2) {
        a.x + b.x,
        a.y + b.y,
    };
}


V2 sub_v2(V2 a, V2 b) {
    return (V2) {
        a.x - b.x,
        a.y - b.y,
    };
}


V2 mul_v2(V2 a, V2 b) {
    return (V2) {
        a.x * b.x,
        a.y * b.y,
    };
}


V2 div_v2(V2 a, V2 b) {
    return (V2) {
        a.x / b.x,
        a.y / b.y,
    };
}


V2 add_v2_f32(V2 a, f32 b) {
    return (V2) {
        a.x + b,
        a.y + b,
    };
}


V2 sub_v2_f32(V2 a, f32 b) {
    return (V2) {
        a.x - b,
        a.y - b,
    };
}


V2 mul_v2_f32(V2 a, f32 b) {
    return (V2) {
        a.x * b,
        a.y * b,
    };
}


V2 div_v2_f32(V2 a, f32 b) {
    return (V2) {
        a.x / b,
        a.y / b,
    };
}

