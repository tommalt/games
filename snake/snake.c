#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 820

#define TILE_SIZE 20
#define FRAME_LENGTH 82

#define BORDER_WIDTH TILE_SIZE

enum _movement {
	MOVE_NONE,
	MOVE_UP,
	MOVE_DOWN,
	MOVE_LEFT,
	MOVE_RIGHT
};

typedef struct {
	unsigned char r, g, b, a;
} Color;

typedef struct {
	int x, y, w, h;
	int dx, dy;
} Object;

typedef struct list {
	int x, y;
	struct list *prev;
	struct list *next;
} List;

typedef struct snake {
	List *head;
	List *tail;
} Snake;

SDL_Window *window = NULL;
SDL_Renderer *rend = NULL;

Color border_color = {255, 0, 0, 255};
Color background = {0, 0, 128, 255};
Color snake_color = {255, 255, 255, 255};
Color target_color = {255, 0, 0, 255};

Snake snake = {0};
SDL_Rect target = {0};

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

void init_snake()
{
	int xpos = 3;
	int ypos = 10;
	snake.head = snake.tail = malloc(sizeof *snake.tail);
	snake.tail->x = xpos * TILE_SIZE;
	snake.tail->y = ypos * TILE_SIZE;
	snake.tail->prev = snake.tail->next = NULL;

	for (int i = 0; i < 5; i++) {
		List *new_head;
		new_head = malloc(sizeof *new_head);
		new_head->x = (i + xpos + 1) * TILE_SIZE;
		new_head->y = ypos * TILE_SIZE;
		new_head->prev = NULL;
		new_head->next = snake.head;
		snake.head->prev = new_head;
		snake.head = new_head;
	}
}

void init_target()
{
	int tiles_w, tiles_h, total;
	int viewport_width, viewport_height;

	viewport_width = WINDOW_WIDTH - (BORDER_WIDTH * 2);
	viewport_height = WINDOW_HEIGHT - (BORDER_WIDTH * 2);

	tiles_w = viewport_width / TILE_SIZE;
	tiles_h = viewport_height / TILE_SIZE;
	total = tiles_w * tiles_h;
	int r = rand();
	int index = r % total;
	int row = index / tiles_w;
	int col = index % tiles_h;

	target.x = BORDER_WIDTH + (row * TILE_SIZE);
	target.y = BORDER_WIDTH + (col * TILE_SIZE);
	target.w = target.h = TILE_SIZE;
}

void print_snake()
{
	List *tail = snake.tail;

	while (tail) {
		printf("(%d,%d) ", tail->x, tail->y);
		tail = tail->prev;
	}
	putchar('\n');
}

int snake_size()
{
	int count = 0;

	List *head = snake.head;
	while (head) {
		count++;
		head = head->next;
	}
	return count;
}

void snake_move(int direction, int time_elapsed, int remove_tail)
{
	int head_x, head_y;

	if (time_elapsed < FRAME_LENGTH) {
		return;
	}

	head_x = snake.head->x;
	head_y = snake.head->y;

	if (direction == MOVE_UP) {
		head_y -= TILE_SIZE;
	} else if (direction == MOVE_DOWN) {
		head_y += TILE_SIZE;
	} else if (direction == MOVE_LEFT) {
		head_x -= TILE_SIZE;
	} else if (direction == MOVE_RIGHT) {
		head_x += TILE_SIZE;
	}

	if (direction != MOVE_NONE) {
		List *new_head;

		new_head = malloc(sizeof *new_head);
		new_head->x = head_x;
		new_head->y = head_y;
		new_head->prev = NULL;
		new_head->next = snake.head;
		snake.head->prev = new_head;
		snake.head = new_head;

		if (remove_tail) {
			List *old_tail = snake.tail;
			snake.tail = snake.tail->prev;
			snake.tail->next = NULL;
			free(old_tail);
		}
	}
}

int out_of_bounds()
{
	int x, y;

	x = snake.head->x;
	y = snake.head->y;
	if (x < BORDER_WIDTH ||
	   (x + TILE_SIZE) > (WINDOW_WIDTH - BORDER_WIDTH) ||
	   (y < BORDER_WIDTH) ||
	   (y + TILE_SIZE) > (WINDOW_HEIGHT - BORDER_WIDTH)) {
		return 1;
	}
	return 0;
}

int snake_collision()
{
	List *head = snake.head;
	List *tail = snake.head->next;
	while (tail) {
		if (head->x == tail->x && head->y == tail->y)
			return 1;
		tail = tail->next;
	}
	return 0;
}

int eaten()
{
	return (snake.head->x == target.x && snake.head->y == target.y);
}

void update(int move, int time_elapsed)
{
	static int delay_counter = 0;

	int head_x, head_y;

	if (delay_counter > 0) {
		--delay_counter;
	}

	head_x = snake.head->x;
	head_y = snake.head->y;

	if (move == MOVE_UP) {
		head_y -= TILE_SIZE;
	} else if (move == MOVE_DOWN) {
		head_y += TILE_SIZE;
	} else if (move == MOVE_LEFT) {
		head_x -= TILE_SIZE;
	} else if (move == MOVE_RIGHT) {
		head_x += TILE_SIZE;
	}

	if (move != MOVE_NONE) {
		List *new_head;

		new_head = malloc(sizeof *new_head);
		new_head->x = head_x;
		new_head->y = head_y;
		new_head->prev = NULL;
		new_head->next = snake.head;
		snake.head->prev = new_head;
		snake.head = new_head;

		if (delay_counter == 0) {
			List *old_tail = snake.tail;
			snake.tail = snake.tail->prev;
			snake.tail->next = NULL;
			free(old_tail);
		}
	}

	if (out_of_bounds() || snake_collision()) {
		GAME_OVER = 1;
	}
	if (eaten()) {
		delay_counter = 5;
		init_target();
	}
}

void render_snake()
{
	List *body;

	body = snake.tail;
	while (body) {
		SDL_Rect rect;
		rect.x = body->x;
		rect.y = body->y;
		rect.w = rect.h = TILE_SIZE;
		SDL_RenderFillRect(rend, &rect);
		body = body->prev;
	}
}

void render_border()
{
	SDL_Rect rect;

	// left
	rect.x = rect.y = 0;
	rect.w = BORDER_WIDTH;//TILE_SIZE;
	rect.h = WINDOW_HEIGHT;
	SDL_RenderFillRect(rend, &rect);

	// top
	rect.w = WINDOW_WIDTH;
	rect.h = BORDER_WIDTH;
	SDL_RenderFillRect(rend, &rect);

	// right
	rect.x = WINDOW_WIDTH - BORDER_WIDTH;
	rect.w = BORDER_WIDTH;
	rect.h = WINDOW_HEIGHT;
	SDL_RenderFillRect(rend, &rect);

	// bottom
	rect.x = 0;
	rect.y = WINDOW_HEIGHT - BORDER_WIDTH;
	rect.w = WINDOW_WIDTH;
	rect.h = BORDER_WIDTH;
	SDL_RenderFillRect(rend, &rect);
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

	srand(time(NULL));
	init_snake();
	init_target();

	Uint32 elapsed;
	Uint32 ticks;
	int move = MOVE_NONE;
	SDL_Event e;

	elapsed = 0;
	for (;;) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				die("quit\n");
			}

			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_UP:
					move = (move == MOVE_DOWN) ? MOVE_DOWN : MOVE_UP; // prevent going backwards
					break;
				case SDLK_DOWN:
					move = (move == MOVE_UP) ? MOVE_UP : MOVE_DOWN;
					break;
				case SDLK_LEFT:
					move = (move == MOVE_RIGHT) ? MOVE_RIGHT : MOVE_LEFT;
					break;
				case SDLK_RIGHT:
					move = (move == MOVE_LEFT) ? MOVE_LEFT : MOVE_RIGHT;
					break;
				};
			}
		}

		int time_elapsed = SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();
		elapsed += time_elapsed;
		if (elapsed > FRAME_LENGTH) {
			update(move, elapsed);
			elapsed = 0;
		}

		if (GAME_OVER) {
			die("GAME OVER\n");
		}

		SDL_SetRenderDrawColor(rend, background.r, background.g, background.b, background.a);
		SDL_RenderClear(rend);

		SDL_SetRenderDrawColor(rend, border_color.r, border_color.g, border_color.b, border_color.a);
		render_border();

		SDL_SetRenderDrawColor(rend, snake_color.r, snake_color.g, snake_color.b, snake_color.a);
		render_snake();

		SDL_SetRenderDrawColor(rend, target_color.r, target_color.g, target_color.b, target_color.a);
		SDL_RenderFillRect(rend, &target);

		SDL_RenderPresent(rend);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);
	return 0;
}

