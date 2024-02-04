#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "algorithms/xoshiro128plus.h"
#include "vector.h"
#include "primitives.h"


s16 min_s16(s16 a, s16 b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}


f32 max_f32(f32 a, f32 b) {
    if (a < b) {
        return b;
    } else {
        return a;
    }
}


f32 mod_f64(f64 a, f64 b) {
#if defined(__x86_64__)
    /* The maximum value of a double-precision floating-point number is ~1.8*10^308. 
     * It might at first seem obvious to convert the float to an integer, but to fit
     * the maximum value, you would need an int_128_t per float, plus one for the 
     * result. Not only is 128-bit integer computation unpreferable for current x64 
     * CPUs, but could also push an unnecessary amount of data out of the CPU cache. */
    f64 fi = a / b;
    __asm__("roundsd $1, %0, %0" : "+x"(fi) :: "rdx");
    return a - (fi * b);

#elif defined(__SIZEOF_INT128__)
    __uint128_t fi = (__uint128_t) a / (__uint128_t) b;
    return a - ((f64) fi * b);

#else
#error "mod_f64 is limited to x86_64"

#endif
}


f32 sqrt_f32(f32 x) {
#if defined(__x86_64__)
    f32 result;
    __asm__("sqrtss %1, %0" : "=x" (result) : "x" (x));
    return result;

#else

#error "sqrt_f32 is limited to x86_64"

#endif
}


f32 pyth_f32(f32 x, f32 y) {
    return sqrt_f32(x*x + y*y);
}


typedef struct {
    V2 position;
    V2 position_last;
    V2 acceleration;
    f32 radius;
} Ball;


void ball_update_position(Ball* ball, f32 dt) {
    V2 velocity = sub_v2(ball->position, ball->position_last);
    ball->position_last = ball->position;
    ball->position = add_v2(
        add_v2(ball->position, velocity), 
        mul_v2_f32(ball->acceleration, dt*dt)
    );
    ball->acceleration = (V2) { 0.0f, 0.0f };
}


void ball_update_acceleration(Ball* ball, V2 acceleration) {
    ball->acceleration = add_v2(ball->acceleration, acceleration);
}


void ball_solve_constraint(Ball* ball, V2 position, f32 radius) {
    V2 delta_position = sub_v2(ball->position, position);
    f32 distance = pyth_f32(delta_position.x, delta_position.y);

    if (distance > radius - ball->radius) {
        V2 normal = div_v2_f32(delta_position, distance);
        ball->position = add_v2(
            position, 
            mul_v2_f32(normal, radius - ball->radius)
        );
    }
}


void ball_solve_collisions(Ball* ball_a, uaddr ball_index, Ball* balls, u32 ball_count, f32 dt) {
    for (uaddr b = ball_index + 1; b != ball_index; b = (b + 1) % ball_count) {
        Ball* ball_b = balls + b;
        V2 collision_axis = sub_v2(ball_a->position, ball_b->position);
        f32 distance = pyth_f32(collision_axis.x, collision_axis.y);
        f32 minimum_intersection = ball_a->radius + ball_b->radius;
        if (distance < minimum_intersection) {
            V2 normal = div_v2_f32(collision_axis, distance);
            f32 delta = minimum_intersection - distance;
            ball_a->position = add_v2(
                ball_a->position,
                mul_v2_f32(normal, 0.5f * delta)
            );
            ball_b->position = sub_v2(
                ball_b->position,
                mul_v2_f32(normal, 0.5f * delta)
            );
        }
    }
}


const V2 GRAVITY = { 0.0f, 40.0f };


void init(Ball* balls, u32 ball_count, f32 dt) {
    Ball* ball = balls;
    for (u16 i = 0; i < ball_count; i += 1) {
        ball->position = (V2) {
            random_f32_range(-128.0f, 128.0f),
            random_f32_range(-128.0f, 128.0f),
        };
        ball->position_last = ball->position;
        ball->acceleration = (V2) { 0.0f, 0.0f };
        ball->radius = random_f32_range(1.0f, 8.0f);
        ball += 1;
    }
}


void tick(Ball* balls, u32 ball_count, f32 dt) {
    u16 sub_steps = 4;
    f32 sub_dt = dt / sub_steps;
    for (u16 step = 0; step < sub_steps; step += 1) {
        Ball* ball = balls;
        for (u32 i = 0; i < ball_count; i += 1) {
            ball_update_acceleration(ball, GRAVITY);
            ball_solve_constraint(ball, (V2) { 0.0f, 0.0f }, 128.0f);
            ball_solve_collisions(ball, i, balls, ball_count, sub_dt);
            ball_update_position(ball, sub_dt);
            ball += 1;
        }
    }
}


void render(Ball* balls, u32 ball_count, f32 dt) {
    Ball* ball;

    DrawCircleSector(
        (Vector2) { 0, 0 },
        128,
        0,
        360,
        180,
        BLUE
    );

    ball = balls;
    for (u16 i = 0; i < ball_count; i += 1) {
        DrawCircle(ball->position.x, ball->position.y, ball->radius, BLACK);
        ball += 1;
    }
}


typedef struct {
    int w;
    int h;
} WindowState;


void PositionComponents(WindowState* window, Camera2D* camera) {
    *window = (WindowState) { GetRenderWidth(), GetRenderHeight() };
    camera->offset = (Vector2) { window->w / 2, window->h / 2 };
    camera->zoom = 0.8f / (256.0f / min_s16(window->w, window->h));
}


int main(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    uint64_t time_nanos = time.tv_nsec;
    random_u32_seed((uint32_t[4]) {
        time_nanos >> 00,
        time_nanos >> 16,
        time_nanos >> 24,
        time_nanos >> 32,
    });
    random_u32_jump();

    WindowState window = (WindowState) { 850, 450 };
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(window.w, window.h, "raylib");
    SetWindowState(
        FLAG_VSYNC_HINT |
        FLAG_WINDOW_RESIZABLE |
        FLAG_WINDOW_TOPMOST
    );

    Camera2D camera = (Camera2D) {
        { 0, 0 },
        { 0, 0 },
        0,
        1.0f,
    };
    PositionComponents(&window, &camera);

    SetTargetFPS(60);

    f32 dt = 0;
    Ball balls[400];

    init(balls, countof(balls), dt);

    while (!WindowShouldClose())
    {
        if (IsWindowResized()) {
            PositionComponents(&window, &camera);
        }
        tick(balls, countof(balls), dt);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        render(balls, countof(balls), dt);
        EndMode2D();

        DrawText("perfection", 20, 20, 20, BLACK);

        EndDrawing();

        dt = GetFrameTime();
    }

    CloseWindow();

    return 0;
}
