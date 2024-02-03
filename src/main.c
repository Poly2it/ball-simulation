#include "raylib.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"


#define countof(a)    (size_t) (sizeof(a) / sizeof(*(a)))
#define lengthof(s)   (countof(s) - 1)


int min_int16(int16_t a, int16_t b) {
	if (a < b) {
		return a;
	} else {
		return b;
	}
}


double mod_double(double a, double b) {
#if defined(__x86_64__)
	/* The maximum value of a double-precision floating-point number is ~1.8*10^308. 
	 * It might at first seem obvious to convert the float to an integer, but to fit
	 * the maximum value, you would need an int_128_t per float, plus one for the 
	 * result. Not only is 128-bit integer computation unpreferable for current x64 
	 * CPUs, but could also push an unnecessary amount of data out of the CPU cache. */
	double fi = a / b;
	__asm__("roundsd $1, %0, %0" : "+x"(fi) :: "rdx");
	return a - (fi * b);
#else
	__uint128_t fi = (__uint128_t) a / (__uint128_t) b;
	return a - ((double) fi * b);
#endif
}


#if defined(__x86_64__)
#define mod_double_inline(a, b) ({                            \
	double __fi = a / b;                                  \
	__asm__("roundsd $1, %0, %0" : "+x"(__fi) :: "rdx");  \
	a - (__fi * b);                                       \
})
#else
#error "Rounding is only implemented on x86_64"
#endif

typedef struct {
	int16_t x;
	int16_t y;
	int16_t dx;
	int16_t dy;
	float radius;
} Ball;


typedef struct {
	int w;
	int h;
} WindowState;


void PositionComponents(WindowState* window, Camera2D* camera) {
	*window = (WindowState) { GetRenderWidth(), GetRenderHeight() };
	camera->offset = (Vector2) { window->w / 2, window->h / 2 };
}


int main(void)
{
	WindowState window = (WindowState) { 850, 450 };
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

	double seed = 805396235182.6108908079348923;
	Ball balls[128];
	Ball* ball = balls;
	for (uint16_t i = 0; i < lengthof(balls); i += 1) {
		seed = mod_double(seed * 77149.9249 - 2, 12279054862.0);
		ball->x = (int16_t) (mod_double(seed, 12.0) - 6.0);
		ball->y = 0;
		ball->dx = 0;
		ball->dy = 0;
		ball->radius = (float) (mod_double(seed, 12.0) + 4.0);
		printf("%f\n", ball->radius);
		ball += 1;
	}

	SetTargetFPS(60);
	
	PositionComponents(&window, &camera);

	while (!WindowShouldClose())
	{
		if (IsWindowResized()) {
			PositionComponents(&window, &camera);
		}
		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode2D(camera);
		DrawCircleSector(
			(Vector2) { 0, 0 },
			(int16_t) (((float) min_int16(window.w, window.h) / 2) / 5 * 4),
			0,
			360,
			180,
			BLUE
		);

		Ball* ball = balls;
		for (uint16_t i = 0; i < lengthof(balls); i += 1) {
			DrawCircle(ball->x, ball->y, ball->radius, BLACK);
			ball += 1;
		}

		EndMode2D();

		DrawText("perfection", 20, 20, 20, BLACK);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}
