/*
 * TODO
 * better collision detection when ball hits top/bottom of paddles.
 * add a start screen with countdown, and game over screen.
 */
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#define WINDOW_WIDTH 540
#define WINDOW_HEIGHT 480
#define PADDLE_WIDTH 12
#define PADDLE_HEIGHT 64
#define BALL_WIDTH 15
#define BALL_VELOCITY 5.f
#define PADDLE_VELOCITY 3.f

enum _movement {
	MOVE_NONE,
	MOVE_UP,
	MOVE_DOWN
};

typedef struct {
	unsigned char r, g, b, a;
} Color;

typedef struct {
	int x, y, w, h;
	int dx, dy;
} Object;

SDL_Window *window = NULL;
SDL_Renderer *rend = NULL;

Color background = {0, 0, 128, 255};
Color foreground = {255, 255, 255, 255};

Object left_paddle;
Object right_paddle;
Object ball;

int GAME_OVER = 0;

void die(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

void warn(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

int check_collision(Object *a, Object *b)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = a->x;
	rightA = a->x + a->w;
	topA= a->y;
	bottomA = a->y + a->h;

	leftB = b->x;
	rightB = b->x + b->w;
	topB= b->y;
	bottomB = b->y + b->h;

	if (bottomA < topB) {
		return 0;
	}
	if (rightA < leftB) {
		return 0;
	}
	if (topA > bottomB) {
		return 0;
	}
	if (leftA > rightB) {
		return 0;
	}
	return 1;
}

/* TODO: better collision detection when ball hits top/bottom of paddle. */
void update()
{
	left_paddle.y += left_paddle.dy;
	if (check_collision(&ball,&left_paddle)) {
		left_paddle.y -= left_paddle.dy;
		ball.dx = -ball.dx;
		ball.dy = -ball.dy;
		ball.x += ball.dx;
		ball.y += ball.dy;
	}
	if (left_paddle.y < 0) {
		left_paddle.y = 0;
	} else if (left_paddle.y + left_paddle.h > WINDOW_HEIGHT) {
		left_paddle.y = WINDOW_HEIGHT - left_paddle.h;
	}

	right_paddle.y += right_paddle.dy;
	if (check_collision(&ball,&left_paddle)) {
		left_paddle.y -= left_paddle.dy;
		ball.dx = -ball.dx;
		ball.dy = -ball.dy;
		ball.x += ball.dx;
		ball.y += ball.dy;
	}
	if (right_paddle.y < 0) {
		right_paddle.y = 0;
	} else if (right_paddle.y + right_paddle.h > WINDOW_HEIGHT) {
		right_paddle.y = WINDOW_HEIGHT - right_paddle.h;
	}


	ball.x += ball.dx;
	if (ball.x <= 0 || ball.x + ball.w >= WINDOW_WIDTH) {
		GAME_OVER = 1;
		return;
	}
	if (check_collision(&ball, &left_paddle)) {
		ball.x -= ball.dx;
		ball.dx = -ball.dx;
		float dy_scale = (((float)ball.y + (float)ball.h / 2.f) - (float)left_paddle.y) / (float)PADDLE_HEIGHT;
		dy_scale -= 0.5f; dy_scale *= 2.f;
		ball.dy = dy_scale * BALL_VELOCITY;
	} else if (check_collision(&ball, &right_paddle)) {
		ball.x -= ball.dx;
		ball.dx = -ball.dx;
		float dy_scale = (((float)ball.y + (float)ball.h / 2.f) - (float)right_paddle.y) / (float)PADDLE_HEIGHT;
		dy_scale -= 0.5f; dy_scale *= 2.f;
		ball.dy = dy_scale * BALL_VELOCITY;
	}

	ball.y += ball.dy;
	if (ball.y <= 0 || ball.y + ball.h >= WINDOW_HEIGHT) {
		ball.y -= ball.dy;
		ball.dy = -ball.dy;
	}
	if (check_collision(&ball,&left_paddle) || check_collision(&ball, &right_paddle)) {
		ball.y -= ball.dy;
		ball.dx = -ball.dx;
		ball.dy = -ball.dy;
		ball.x += ball.dx;
		ball.y += ball.dy;
	}
}


int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		die("Failed to open SDL window: %s\n", SDL_GetError());
	}
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		warn("Linear texture filtering is not enabled\n");
	}
	window = SDL_CreateWindow("PONG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		die("Failed to open SDL window: %s\n", SDL_GetError());
	}
	rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

	left_paddle.x = 10;
	left_paddle.y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.f;
	left_paddle.w = PADDLE_WIDTH;
	left_paddle.h = PADDLE_HEIGHT;

	right_paddle.x = WINDOW_WIDTH - 10 - PADDLE_WIDTH;
	right_paddle.y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.f;
	right_paddle.w = PADDLE_WIDTH;
	right_paddle.h = PADDLE_HEIGHT;

	ball.w = BALL_WIDTH;
	ball.h = BALL_WIDTH;
	ball.x = (WINDOW_WIDTH / 2.f) - (BALL_WIDTH / 2.f);
	ball.y = (WINDOW_HEIGHT / 2.f) - (BALL_WIDTH / 2.f);
	ball.dx = -BALL_VELOCITY;
	ball.dy = 0;

	int left_move, right_move;
	left_move = right_move = MOVE_NONE;

	SDL_Event e;
	for (;;) {
		SDL_PollEvent(&e);
		if (e.type == SDL_QUIT) {
			break;
		}

		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
			case SDLK_UP:
				right_move = MOVE_UP;
				break;
			case SDLK_DOWN:
				right_move = MOVE_DOWN;
				break;
			case SDLK_w:
				left_move = MOVE_UP;
				break;
			case SDLK_s:
				left_move = MOVE_DOWN;
				break;
			};
		}
		if (e.type == SDL_KEYUP) {
			switch (e.key.keysym.sym) {
			case SDLK_UP:
			case SDLK_DOWN:
				right_move = MOVE_NONE;
				break;
			case SDLK_w:
			case SDLK_s:
				left_move = MOVE_NONE;
				break;
			};
		}
		if (right_move == MOVE_NONE) {
			right_paddle.dy = 0;
		} else if (right_move == MOVE_UP) {
			right_paddle.dy = -PADDLE_VELOCITY;
		} else if (right_move == MOVE_DOWN) {
			right_paddle.dy = PADDLE_VELOCITY;
		}
		if (left_move == MOVE_NONE) {
			left_paddle.dy = 0;
		} else if (left_move == MOVE_UP) {
			left_paddle.dy = -PADDLE_VELOCITY;
		} else if (left_move == MOVE_DOWN) {
			left_paddle.dy = PADDLE_VELOCITY;
		}

		update();
		if (GAME_OVER) {
			printf("GAME OVER!\n");
			break;
		}

		SDL_SetRenderDrawColor(rend, background.r, background.g, background.b, background.a);
		SDL_RenderClear(rend);

		SDL_SetRenderDrawColor(rend, foreground.r, foreground.g, foreground.b, foreground.a);
		SDL_RenderFillRect(rend, &left_paddle);
		SDL_RenderFillRect(rend, &right_paddle);
		SDL_RenderFillRect(rend, &ball);

		SDL_RenderPresent(rend);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);

	return 0;
}

