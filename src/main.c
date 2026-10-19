#define WIDTH 800
#define HEIGHT 600

#include <stdbool.h>
#include <stdlib.h> // x
#include <stdio.h>	// x
#include <math.h>
#include "raylib.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir

enum State
{
	START,
	PLAYING,
	PAUSED,
	GAME_OVER
};

typedef struct Player
{
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
	int size;
} Player;

typedef struct PipePair
{
	Rectangle container;
	int topHeight;
	int bottHeight;
} PipePair;

void update_player(Player *player)
{
	player->velocity.x += player->acceleration.x;
	player->velocity.y += player->acceleration.y;

	player->position.x += player->velocity.x;
	player->position.y += player->velocity.y;

	player->acceleration.x = 0;
	player->acceleration.y = 0;
}

void limit_player_velocity(Player *player, float maxMag)
{
	float e = 1e-3;
	float mag = sqrt(player->velocity.x * player->velocity.x + player->velocity.y * player->velocity.y);

	if (mag > e && mag > maxMag)
	{
		player->velocity.x = maxMag * player->velocity.x / mag;
		player->velocity.y = maxMag * player->velocity.y / mag;
	}
}

void draw_player(Player *player)
{
	int x = player->position.x - player->size / 2;
	int y = player->position.y - player->size / 2;

	DrawRectangle(x, y, player->size, player->size, RED);
}

void draw_start_screen(enum State *state)
{
	int containerWidth = GetScreenWidth() * .5;
	int containerHeight = GetScreenHeight() * .7;
	int containerX = (GetScreenWidth() - containerWidth) / 2;
	int containerY = (GetScreenHeight() - containerHeight) / 2;

	DrawRectangle(containerX, containerY, containerWidth, containerHeight, LIGHTGRAY);

	int logoWidth = 50;
	int logoHeight = 50;
	int logoX = containerX + (containerWidth - logoWidth) / 2;
	int logoY = containerY + containerHeight / 4 - logoHeight / 2;

	DrawRectangle(logoX, logoY, logoWidth, logoHeight, RED);

	char *text = "Flappy Bird";
	int fontSize = 30;
	int textWidth = MeasureText(text, fontSize);
	int textX = containerX + (containerWidth - textWidth) / 2;
	int textY = containerY + containerHeight / 3;

	DrawText(text, textX, textY, fontSize, BLACK);

	int buttonWidth = 100;
	int buttonHeight = 50;
	int buttonX = containerX + (containerWidth - buttonWidth) / 2;
	int buttonY = containerY + (containerHeight - buttonHeight) / 2;

	DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, BEIGE);

	char *buttonText = "START";
	int buttonTextFontSize = 20;
	int buttonTextWidth = MeasureText(buttonText, buttonTextFontSize);
	int buttonTextX = buttonX + (buttonWidth - buttonTextWidth) / 2;
	int buttonTextY = buttonY + (buttonHeight - buttonTextFontSize) / 2;

	DrawText(buttonText, buttonTextX, buttonTextY, buttonTextFontSize, BLACK);

	if (IsKeyReleased(KEY_ENTER))
		*state = PLAYING;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Vector2 mouse = GetMousePosition();

		bool isWithinX = buttonX <= mouse.x && mouse.x <= buttonX + buttonWidth;
		bool isWithinY = buttonY <= mouse.y && mouse.y <= buttonY + buttonHeight;

		if (isWithinX && isWithinY)
			*state = PLAYING;
	}
}

void randomize_pipe_size(PipePair *pair, int playerSize)
{
	int minHeight = pair->container.height * .25;
	int available = pair->container.height - 2 * (2 * playerSize + minHeight);
	int first = rand() % available;
	int second = available - first;

	pair->topHeight = first + minHeight;
	pair->bottHeight = second + minHeight;
}

void reset_pipe(PipePair *pair, int pipeCount, int pipeSpacing, int playerSize)
{
	int x = (pipeCount - 1) * (pair->container.width + pipeSpacing);

	pair->container.x = x + pipeSpacing;

	randomize_pipe_size(pair, playerSize);
}

void draw_pipes(PipePair *pipes, int pipeCount, int *firstIdx, int pipeSpacing)
{
	float speed = -5;

	int padding = 100;

	for (int i = 0; i < pipeCount; i++)
	{
		int idx = (*firstIdx + i) % pipeCount;

		PipePair *pair = &pipes[i];

		char str[32];

		snprintf(str, sizeof(str), "item %d", i);

		// DrawRectangle(pair->container.x, pair->container.y, pair->container.width, pair->container.height, ORANGE);
		DrawRectangle(pair->container.x, pair->container.y, pair->container.width, pair->topHeight, RED);
		DrawRectangle(pair->container.x, pair->container.y + pair->container.height - pair->bottHeight, pair->container.width, pair->bottHeight, RED);
		// DrawText(str, pair->container.x + 10, pair->container.y + 10, 10, YELLOW);

		pair->container.x += speed;
	}
}

void draw_score(int score)
{
	char str[32];

	snprintf(str, sizeof(str), "score: %d", score);

	int fontSize = 20;
	int textWidth = MeasureText(str, fontSize);
	int padding = 10;
	int boxWidth = textWidth + 2 * padding;
	int boxHeight = fontSize + 2 * padding;
	int margin = 20;
	int boxX = WIDTH - boxWidth - margin;
	int boxY = boxHeight + margin;

	DrawRectangle(boxX, boxY, boxWidth, boxHeight, YELLOW);

	int textX = boxX + padding;
	int textY = boxY + padding;

	DrawText(str, textX, textY, fontSize, RED);
}

bool rects_overlap(Vector2 l1, Vector2 r1, Vector2 l2, Vector2 r2)
{
	if (l1.x > r2.x || l2.x > r1.x)
		return false;

	if (l1.y < r2.y || l2.y < r1.y)
		return false;

	return true;
}

bool detect_collision(PipePair *pairs, int pipeCount, Player *player)
{
	for (int i = 0; i < pipeCount; i++)
	{
		PipePair pair = pairs[i];
		Vector2 topPipeTopLeft = {.x = pair.container.x, .y = pair.container.y};
		Vector2 topPipeBottRight = {.x = pair.container.x + pair.container.width, .y = pair.container.y + pair.topHeight};
		Vector2 bottPipeTopLeft = {.x = pair.container.x, .y = pair.container.y + pair.container.height - pair.bottHeight};
		Vector2 bottPipeBottRight = {.x = pair.container.x + pair.container.width, .y = pair.container.y + pair.container.height};

		DrawCircle(topPipeTopLeft.x, topPipeTopLeft.y, 10, LIGHTGRAY);
		DrawCircle(topPipeBottRight.x, topPipeBottRight.y, 10, DARKGRAY);

		DrawCircle(bottPipeTopLeft.x, bottPipeTopLeft.y, 10, GREEN);
		DrawCircle(bottPipeBottRight.x, bottPipeBottRight.y, 10, YELLOW);

		Vector2 playerTopLeft = {.x = player->position.x - player->size / 2, .y = player->position.y - player->size / 2};
		Vector2 playerBottRight = {.x = player->position.x + player->size / 2, .y = player->position.y + player->size / 2};

		DrawCircle(playerTopLeft.x, playerTopLeft.y, 10, ORANGE);
		DrawCircle(playerBottRight.x, playerBottRight.y, 10, PINK);

		bool intersect = rects_overlap(topPipeTopLeft, topPipeBottRight, playerTopLeft, playerBottRight) ||
						 rects_overlap(bottPipeTopLeft, bottPipeBottRight, playerTopLeft, playerBottRight);

		if (intersect)
			return true;
	}

	return false;
}

void play(PipePair *pairs, int pipeCount, int *firstIdx, int pipeSpacing, Player *player, int *score, bool *keyUpPressed)
{
	float maxMag = 5;
	float gravity = .25;
	float upwardsForce = 10;

	if (IsKeyPressed(KEY_UP))
		if (!(*keyUpPressed))
		{
			player->acceleration.y -= upwardsForce;
			*keyUpPressed = true;
		}

	if (IsKeyReleased(KEY_UP))
		*keyUpPressed = false;

	player->acceleration.y += gravity;

	update_player(&(*player));
	limit_player_velocity(&(*player), maxMag);
	draw_player(player);
	draw_pipes(pairs, pipeCount, firstIdx, pipeSpacing);
	draw_score(*score);

	bool collided = detect_collision(pairs, pipeCount, player);

	if (collided)
	{
		printf("fuck!\n");

		return;
	}

	PipePair *first = &pairs[*firstIdx];

	if (first->container.x + first->container.width < 0)
	{
		reset_pipe(first, pipeCount, pipeSpacing, player->size);
		*firstIdx = (*firstIdx + 1) % pipeCount;
		*score += 1;
	}
}

int compute_obstacle_count(int width, int spacing)
{
	int count = (int)ceil((float)(WIDTH + spacing) / (width + spacing));

	return count;
}

PipePair *make_pipes(int pipeCount, int pipeWidth, int pipeSpacing, int playerSize)
{
	PipePair *pipes = malloc(sizeof(*pipes) * pipeCount);

	for (int i = 0; i < pipeCount; i++)
	{
		PipePair pair = {
			.container = (Rectangle){
				.x = i * (pipeWidth + pipeSpacing) + WIDTH,
				.y = 0,
				.width = pipeWidth,
				.height = HEIGHT},
			.topHeight = 0,
			.bottHeight = 0,
		};

		randomize_pipe_size(&pair, playerSize);

		pipes[i] = pair;
	}

	return pipes;
}

int main()
{
	InitWindow(WIDTH, HEIGHT, "Flappy Bird");

	enum State state = START;
	Player player = {.position = {WIDTH / 10, 300}, .velocity = {0, 0}, .acceleration = {0, 0}, .size = 30};
	int score = 0;
	bool keyUpPressed = false;

	int firstIdx = 0;
	int pipeWidth = 50;
	int pipeSpacing = 200;
	int pipeCount = compute_obstacle_count(pipeWidth, pipeSpacing);
	PipePair *pipes = make_pipes(pipeCount, pipeWidth, pipeSpacing, player.size);

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	SetTargetFPS(60);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		BeginDrawing();

		ClearBackground(RAYWHITE);

		if (state == START)
			draw_start_screen(&state);
		else if (state == PLAYING)
			play(pipes, pipeCount, &firstIdx, pipeSpacing, &player, &score, &keyUpPressed);

		// draw_pipes(pipes, pipeCount, &firstIdx, pipeSpacing);

		EndDrawing();
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();

	free(pipes);

	return 0;
}
