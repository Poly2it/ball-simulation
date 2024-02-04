#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "algorithms/xoshiro128plus.h"
#include "vector.h"
#include "primitives.h"


#define DEBUG_LOG_INFO
/* #define DEBUG_LOG_LOOP */


#define DEBUG_PRINTF(...) printf("%s %i: ", __FILE__, __LINE__); printf(__VA_ARGS__);
#define DEBUG_ASSERT(error, ...) \
    if (!(__VA_ARGS__)) { \
        DEBUG_PRINTF("assertion failed: %s", error); \
        exit(1); \
    }


#ifdef DEBUG_LOG_INFO
#define DEBUG_PRINTF_INFO(...) DEBUG_PRINTF(__VA_ARGS__);
#else
#define DEBUG_PRINTF_INFO(...) (void) 0;
#endif


#ifdef DEBUG_LOG_LOOP
#define DEBUG_PRINTF_LOOP(...) DEBUG_PRINTF(__VA_ARGS__);
#else
#define DEBUG_PRINTF_LOOP(...) (void) 0;
#endif


s16 min_s16(s16 a, s16 b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}


f32 min_f32(f32 a, f32 b) {
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


saddr min_saddr(saddr a, saddr b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}


saddr max_saddr(saddr a, saddr b) {
    if (a < b) {
        return b;
    } else {
        return a;
    }
}


saddr clamp_saddr(saddr x, saddr min, saddr max) {
    return min_saddr(max_saddr(x, min), max);
}


uaddr min_uaddr(uaddr a, uaddr b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}


uaddr max_uaddr(uaddr a, uaddr b) {
    if (a < b) {
        return b;
    } else {
        return a;
    }
}


uaddr clamp_uaddr(uaddr x, uaddr min, uaddr max) {
    return min_uaddr(max_uaddr(x, min), max);
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


typedef struct __Link {
    Ball* ball;
    saddr length;
    int validator;
    struct __Link* next;
} Link;


Link* link_create(void) {
    Link* link = malloc(sizeof(Link));
    *link = (Link) {
        .length = 0,
        .validator = 1010
    };
    return link;
}


void link_destroy(Link* link) {
    DEBUG_PRINTF_LOOP("Destroying link\n");
    Link* link_i = link;
    Link* link_j;
    uaddr length;
    do {
        length = link_i->length;
        link_j = link_i->next;
        free(link_i);
        link_i = link_j;
    } while (length > 0);
}


void link_append(Link* link, Ball* ball) {
    /* DEBUG_PRINTF("Appending link\n"); */
    Link* link_i = link;
    while (link_i->length > 0) {
        link_i->length += 1;
        link_i = link_i->next;
    }
    link_i->length += 1;
    link_i->next = link_create();
    link_i->ball = ball;
}


void link_clear(Link* link) {
    if (link->length > 0) {
        link_destroy(link->next);
        link->length = 0;
    }
}


typedef struct {
    uaddr w;
    uaddr h;
    int validator;
    Link*** link;
} Grid;


const uaddr GRID_W = 64;
const uaddr GRID_H = 64;


const f32 SIMULATION_W = 2048.0f;
const f32 SIMULATION_H = 2048.0f;


Grid* grid_create(void) {
    DEBUG_PRINTF_INFO("Creating grid\n");

    Link** row;
    Link* column;

    Link*** link_head = malloc(sizeof(Link**) * (GRID_W + 2));

    Grid* grid = malloc(sizeof(Grid));
    *grid = (Grid) {
        .w = GRID_W,
        .h = GRID_H,
        .validator = 2020,
        .link = link_head
    };
    DEBUG_PRINTF_INFO("Grid allocated\n");

    for (uaddr x = 0; x < GRID_W + 2; x += 1) {
        *(link_head + x) = malloc(sizeof(Link*) * (GRID_H + 2));
        row = *(link_head + x);

        for (uaddr y = 0; y < GRID_H + 2; y += 1) {
            DEBUG_PRINTF_LOOP("Allocating %i, %i\n", (int) x, (int) y);
            *(row + y) = link_create();
        }
    }

    DEBUG_PRINTF_INFO("Grid complete\n");
    return grid;
}


void grid_destroy(Grid* grid) {
    DEBUG_PRINTF_LOOP("Destroying grid\n");

    Link** row;
    Link* column;
    for (uaddr x = 0; x < GRID_W + 2; x += 1) {
        row = *(grid->link + x);
        for (uaddr y = 0; y < GRID_H + 2; y += 1) {
            link_destroy(*(row + y));
        }
    }
}


Link* grid_get(Grid* grid, saddr x, saddr y) {
    Link* cell = *(*(grid->link + (x + 1)) + (y + 1));
    return cell;
}


void grid_clear(Grid* grid) {
    DEBUG_PRINTF_LOOP("Clearing grid\n");
    for (uaddr x = 0; x < grid->w + 2; x += 1) {
        for (uaddr y = 0; y < grid->h + 2; y += 1) {
            Link* cell = *(*(grid->link + x) + y);
            link_clear(cell);
        }
    }
}


void ball_update_position(Ball* ball, f32 dt) {
    DEBUG_PRINTF_LOOP("Updating ball position grid\n");
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
    DEBUG_PRINTF_LOOP("Solving ball constraints\n");
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


void ball_solve_collisions(Ball* ball_a, Link* tile_link[9], f32 dt) {
    DEBUG_PRINTF_LOOP("Solving ball collissions\n");
    Link* link;

    for (uaddr i = 0; i < 9; i += 1) {
        DEBUG_PRINTF_LOOP("Comparing with tile %i/9\n", (int) i);
        link = tile_link[i];
        while (link->length > 0) {
            DEBUG_PRINTF_LOOP("link length: %i\n", (int) link->length);
            if (link->ball != ball_a) {
                Ball* ball_b = link->ball;

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
            link = link->next;
        }
    }
}


const V2 GRAVITY = { 0.0f, 500.0f };


void ball_solve_scene_constraints(Ball* ball) {
    ball_solve_constraint(
        ball, 
        (V2) { SIMULATION_W / 2.0f, SIMULATION_H / 2.0f }, 
        min_f32(SIMULATION_W, SIMULATION_H) / 2.0f
    );
}


const f32 BALL_MINIMUM_RADIUS = 8.0f;
const f32 BALL_MAXIMUM_RADIUS = 16.0f;


void init(Grid* grid, Ball* balls, u32 ball_count, f32 dt) {
    Ball* ball = balls;
    for (u16 i = 0; i < ball_count; i += 1) {
        ball->position = (V2) {
            random_f32_range(0.0f, SIMULATION_W),
            random_f32_range(0.0f, SIMULATION_H),
        };
        ball->acceleration = (V2) { 0.0f, 0.0f };
        ball->radius = random_f32_range(BALL_MINIMUM_RADIUS, BALL_MAXIMUM_RADIUS);
        ball_solve_scene_constraints(ball);
        ball->position_last = ball->position;

        uaddr gx = ball->position.x / SIMULATION_W * GRID_W;
        uaddr gy = ball->position.y / SIMULATION_H * GRID_H;
        Link* link = grid_get(grid, gx, gy);
        link_append(link, ball);

        ball += 1;
    }
}


void tick(Grid* grid, Ball* balls, u32 ball_count, f32 dt) {
    u16 sub_steps = 4;
    f32 sub_dt = dt / sub_steps;
    for (u16 step = 0; step < sub_steps; step += 1) {
        for (uaddr x = 0; x < grid->w; x += 1) {
            Link* link_grid[9] = {
                grid_get(grid, x - 1, -1), grid_get(grid, x + 0, -1), grid_get(grid, x + 1, -1),
                grid_get(grid, x - 1, +0), grid_get(grid, x + 0, +0), grid_get(grid, x + 1, +0),
                grid_get(grid, x - 1, +1), grid_get(grid, x + 0, +1), grid_get(grid, x + 1, +1),
            };
            for (uaddr y = 0; y < grid->h; y += 1) {
                saddr ys = y % 3;
                for (saddr xs = 0; xs < 3; xs += 1) {
                    link_grid[ys * 3 + xs] = grid_get(grid, x + xs - 1, y + 1);
                }

                Link* link = grid_get(grid, x, y);

                while (link->length > 0) {
                    Ball* ball = link->ball;
                    ball_update_acceleration(ball, GRAVITY);
                    ball_solve_collisions(ball, link_grid, sub_dt);
                    ball_update_position(ball, sub_dt);

                    link = link->next;
                };
            }
        }

        grid_clear(grid);
        Ball* ball = balls;
        for (u16 i = 0; i < ball_count; i += 1) {
            ball_solve_scene_constraints(ball);
            
            saddr gx = clamp_saddr(ball->position.x / SIMULATION_W * GRID_W, 0, GRID_W);
            saddr gy = clamp_saddr(ball->position.y / SIMULATION_H * GRID_H, 0, GRID_H);
            Link* link = grid_get(grid, gx, gy);
            link_append(link, ball);

            ball += 1;
        }
    }
}


void render(Ball* balls, u32 ball_count, f32 dt) {
    Ball* ball;

    DrawCircleSector(
        (Vector2) { SIMULATION_W / 2.0f, SIMULATION_H / 2.0f },
        min_f32(SIMULATION_W, SIMULATION_H) / 2.0f,
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
    camera->zoom = 0.8f / (min_f32(SIMULATION_W, SIMULATION_H) / min_s16(window->w, window->h));
    camera->offset = (Vector2) {
        (window->w - SIMULATION_W * camera->zoom) / 2.0f,
        (window->h - SIMULATION_H * camera->zoom) / 2.0f
    };
}


int main(void){
    DEBUG_ASSERT("Grid cell fits maximum-size entity", ((SIMULATION_W / GRID_W) > BALL_MAXIMUM_RADIUS));
    struct timespec time;
    // clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    // uint64_t time_nanos = time.tv_nsec;
    u64 time_nanos = 128481284919;
    random_u32_seed((uint32_t[4]) {
        time_nanos >> 00,
        time_nanos >> 16,
        time_nanos >> 24,
        time_nanos >> 32,
    });
    random_u32_jump();

    WindowState window = (WindowState) { 850, 450 };
    // SetConfigFlags(FLAG_MSAA_4X_HINT);
    // SetTraceLogLevel(LOG_ERROR);
    InitWindow(window.w, window.h, "raylib");
    SetWindowState(
        FLAG_VSYNC_HINT |
	FLAG_WINDOW_TOPMOST |
        FLAG_WINDOW_RESIZABLE
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
    f32 fps;

    Grid* grid = grid_create();
    const uaddr BALL_COUNT = 3000;
    Ball balls[BALL_COUNT];

    init(grid, balls, BALL_COUNT, dt);

    while (!WindowShouldClose())
    {
        if (IsWindowResized()) {
            PositionComponents(&window, &camera);
        }

        tick(grid, balls, BALL_COUNT, dt);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        render(balls, BALL_COUNT, dt);
        EndMode2D();

        f32 fps = 1.0f / dt;
        c8 fps_str[64];
        sprintf(fps_str, "FPS: %0.2f", fps);
        DrawText(fps_str, 20, 20, 20, BLACK);

        EndDrawing();

        dt = GetFrameTime();
    }

    grid_destroy(grid);

    CloseWindow();

    return 0;
}

