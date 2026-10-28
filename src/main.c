#define WIDTH 800
#define HEIGHT 600
#define MAX_LIVES 5
#define MAX_SPEED 5.0
#define MAX_GHOST_TIME 30
#define PI 3.14159265358979323846

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
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
	Texture2D image;
	int lives;
	float imageScale;
	int ghostTime;
	int size;
} Player;

typedef struct PipeTexture
{
	Texture2D image;
	int width;
	int height;
	float scale;
} PipeTexture;

typedef struct PipePair
{
	Rectangle container;
	bool impacted;
	int topHeight;
	int bottHeight;
	PipeTexture *texture;
} PipePair;

typedef struct BgTextureData
{
	Texture2D *texture;
	float scale;
	float width;
} BgTextureData;

typedef struct BgRect
{
	Vector2 position;
	BgTextureData *textureData;
} BgRect;

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

void update_player_physics(Player *player, float maxSpeed)
{
	player->velocity.x += player->acceleration.x;
	player->velocity.y += player->acceleration.y;

	player->position.x += player->velocity.x;
	player->position.y += player->velocity.y;

	player->acceleration.x = 0;
	player->acceleration.y = 0;

	if (maxSpeed != INFINITY)
		limit_player_velocity(player, maxSpeed);
}

void update_player(Player *player)
{
	update_player_physics(player, MAX_SPEED);

	if (player->ghostTime < MAX_GHOST_TIME)
		player->ghostTime += 1;
}

void draw_player(Player *player)
{
	int x = player->position.x - player->size / 2;
	int y = player->position.y - player->size / 2;
	Vector2 imagePosition = {.x = x, .y = y};
	unsigned char alpha = 255;

	if (player->ghostTime < MAX_GHOST_TIME)
		alpha = fabs(sin(player->ghostTime / 3.0)) * 255;

	Color color = {.r = 255, .g = 255, .b = 255, .a = alpha};

	DrawTextureEx(player->image, imagePosition, 0, player->imageScale, color);
}

bool is_button_clicked(int x, int y, int width, int height)
{
	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		return false;

	Vector2 mouse = GetMousePosition();

	bool isWithinX = x <= mouse.x && mouse.x <= x + width;
	bool isWithinY = y <= mouse.y && mouse.y <= y + height;

	if (isWithinX && isWithinY)
		return true;

	return false;
}

void draw_paused_screen(enum State *state)
{
	int containerWidth = GetScreenWidth() * .5;
	int containerHeight = GetScreenHeight() * .7;
	int containerX = (GetScreenWidth() - containerWidth) / 2;
	int containerY = (GetScreenHeight() - containerHeight) / 2;

	DrawRectangle(containerX, containerY, containerWidth, containerHeight, LIGHTGRAY);

	char *h1 = "PAUSED";
	int h1FontSize = 40;
	int h1Width = MeasureText(h1, h1FontSize);
	int h1X = containerX + (containerWidth - h1Width) / 2;
	int h1Y = containerY + containerHeight / 4;

	DrawText(h1, h1X, h1Y, h1FontSize, BLACK);

	char *buttText = "RESUME";
	int buttFontSize = 20;
	int buttTextSize = MeasureText(buttText, buttFontSize);
	int buttPadding = 10;
	int buttWidth = buttTextSize + 2 * buttPadding;
	int buttHeight = buttFontSize + 2 * buttPadding;
	int buttX = containerX + (containerWidth - buttWidth) / 2;
	int buttY = containerY + containerHeight / 2;

	DrawRectangle(buttX, buttY, buttWidth, buttHeight, GRAY);

	int buttTextX = buttX + buttPadding;
	int buttTextY = buttY + buttPadding;

	DrawText(buttText, buttTextX, buttTextY, buttFontSize, BLACK);

	if (is_button_clicked(buttX, buttY, buttWidth, buttHeight) || IsKeyPressed(KEY_P))
		*state = PLAYING;
}

void draw_game_over_screen(enum State *state, int score)
{
	int containerWidth = GetScreenWidth() * .5;
	int containerHeight = GetScreenHeight() * .7;
	int containerX = (GetScreenWidth() - containerWidth) / 2;
	int containerY = (GetScreenHeight() - containerHeight) / 2;

	DrawRectangle(containerX, containerY, containerWidth, containerHeight, LIGHTGRAY);

	char *h1Text = "GAME OVER";
	int h1FontSize = 40;
	int h1Width = MeasureText(h1Text, h1FontSize);
	int h1X = containerX + (containerWidth - h1Width) / 2;
	int h1Y = containerY + containerHeight / 4;

	DrawText(h1Text, h1X, h1Y, h1FontSize, BLACK);

	char scoreMessage[32];

	snprintf(scoreMessage, sizeof(scoreMessage), "SCORE: %d", score);

	int scoreFontSize = 30;
	int scoreWidth = MeasureText(scoreMessage, scoreFontSize);
	int scoreBoxPadding = 10;
	int scoreBoxWidth = scoreWidth + 2 * scoreBoxPadding;
	int scoreBoxHeight = scoreFontSize + 2 * scoreBoxPadding;
	int scoreBoxX = containerX + (containerWidth - scoreBoxWidth) / 2;
	int scoreBoxY = containerY + containerHeight / 2.25 - scoreBoxHeight / 2;

	int scoreX = scoreBoxX + scoreBoxPadding;
	int scoreY = scoreBoxY + scoreBoxPadding;

	DrawRectangle(scoreBoxX, scoreBoxY, scoreBoxWidth, scoreBoxHeight, YELLOW);
	DrawText(scoreMessage, scoreX, scoreY, scoreFontSize, BLACK);

	int buttsBoxWidth = .8 * containerWidth;
	int buttsBoxHeight = 75;
	int buttsBoxX = containerX + (containerWidth - buttsBoxWidth) / 2;
	int buttsBoxY = containerY + 2 * containerHeight / 3;
	int buttsBoxPadd = 10;

	DrawRectangle(buttsBoxX, buttsBoxY, buttsBoxWidth, buttsBoxHeight, LIGHTGRAY);

	int buttWidth = buttsBoxWidth * .4;
	int buttHeight = buttsBoxHeight - 2 * buttsBoxPadd;
	int leftButtX = buttsBoxX + buttsBoxPadd;
	int leftButtY = buttsBoxY + buttsBoxPadd;
	int rightButtX = buttsBoxX + buttsBoxWidth - buttsBoxPadd - buttWidth;
	int rightButtY = buttsBoxY + buttsBoxPadd;

	DrawRectangle(leftButtX, leftButtY, buttWidth, buttHeight, GRAY);
	DrawRectangle(rightButtX, rightButtY, buttWidth, buttHeight, GRAY);

	int buttPadd = 5;
	int buttFontSize = 20;

	char *leftButtText = "HOME";
	int leftTextWidth = MeasureText(leftButtText, buttFontSize);
	int leftTextX = leftButtX + (buttWidth - leftTextWidth) / 2;
	int leftTextY = leftButtY + (buttHeight - buttFontSize) / 2;

	DrawText(leftButtText, leftTextX, leftTextY, buttFontSize, BLACK);

	char *rightButtText = "QUIT";
	int rightTextWidth = MeasureText(rightButtText, buttFontSize);
	int rightTextX = rightButtX + (buttWidth - rightTextWidth) / 2;
	int rightTextY = rightButtY + (buttHeight - buttFontSize) / 2;

	DrawText(rightButtText, rightTextX, rightTextY, buttFontSize, BLACK);

	if (is_button_clicked(leftButtX, leftButtY, buttWidth, buttHeight) || IsKeyReleased(KEY_ENTER))
		*state = START;
	else if (is_button_clicked(rightButtX, rightButtY, buttWidth, buttHeight) || IsKeyReleased(KEY_Q))
		CloseWindow();
}

void draw_start_screen(enum State *state, Texture2D *logo, int logoSize, float logoScale)
{
	int containerWidth = GetScreenWidth() * .5;
	int containerHeight = GetScreenHeight() * .7;
	int containerX = (GetScreenWidth() - containerWidth) / 2;
	int containerY = (GetScreenHeight() - containerHeight) / 2;

	DrawRectangle(containerX, containerY, containerWidth, containerHeight, LIGHTGRAY);

	int logoX = containerX + (containerWidth - logoSize) / 2;
	int logoY = containerY + containerHeight / 4 - logoSize / 2;
	Vector2 logoPosition = {.x = logoX, .y = logoY};

	DrawTextureEx(*logo, logoPosition, 0, logoScale, WHITE);

	char *text = "Flappy Bird";
	int fontSize = 30;
	int textWidth = MeasureText(text, fontSize);
	int textX = containerX + (containerWidth - textWidth) / 2;
	int textY = containerY + containerHeight / 2.5;

	DrawText(text, textX, textY, fontSize, BLACK);

	int buttonWidth = 100;
	int buttonHeight = 50;
	int buttonX = containerX + (containerWidth - buttonWidth) / 2;
	int buttonY = containerY + 1.8 * (containerHeight - buttonHeight) / 3;

	DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, BEIGE);

	char *buttonText = "START";
	int buttonTextFontSize = 20;
	int buttonTextWidth = MeasureText(buttonText, buttonTextFontSize);
	int buttonTextX = buttonX + (buttonWidth - buttonTextWidth) / 2;
	int buttonTextY = buttonY + (buttonHeight - buttonTextFontSize) / 2;

	DrawText(buttonText, buttonTextX, buttonTextY, buttonTextFontSize, BLACK);

	if (IsKeyReleased(KEY_ENTER))
		*state = PLAYING;
	else if (is_button_clicked(buttonX, buttonY, buttonWidth, buttonHeight))
		*state = PLAYING;
}

void draw_lives(int lives)
{
	int liveSize = 10;
	int gap = 10;
	int padding = 10;
	int containerWidth = 2 * MAX_LIVES * liveSize + (MAX_LIVES - 1) * gap + 2 * padding;
	int containerHeight = liveSize + 2 * padding;
	int marginTop = 25;
	int marginRight = 20;
	int containerX = WIDTH - marginRight - containerWidth;
	int containerY = marginTop;

	DrawRectangle(containerX, containerY, containerWidth, containerHeight, LIGHTGRAY);

	for (int i = 0; i < lives; i++)
	{
		int xi = containerX + padding + gap * i + 2 * liveSize * i + liveSize;
		int yi = containerY + containerHeight / 2;

		DrawCircle(xi, yi, liveSize, RED);
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

void draw_pipes(PipePair *pipes, int pipeCount, int *firstIdx, int pipeSpacing)
{
	int bottRotation = 180;

	for (int i = 0; i < pipeCount; i++)
	{
		PipePair *pair = &pipes[i];
		Vector2 topTexturePosition = {.x = pair->container.x, .y = pair->container.y - pair->texture->height + pair->topHeight};
		Vector2 bottTexturePosition = {.x = pair->container.x + pair->container.width,
									   .y = pair->container.y + pair->container.height - pair->bottHeight + pair->texture->height};

		DrawTextureEx(pair->texture->image, topTexturePosition, 0, pair->texture->scale, WHITE);
		DrawTextureEx(pair->texture->image, bottTexturePosition, bottRotation, pair->texture->scale, WHITE);
	}
}

void update_pipes(PipePair *pairs, int pipeCount)
{
	float speed = -5;

	for (int i = 0; i < pipeCount; i++)
		pairs[i].container.x += speed;
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
	pair->impacted = false;

	randomize_pipe_size(pair, playerSize);
}

bool rects_overlap(Vector2 l1, Vector2 r1, Vector2 l2, Vector2 r2)
{
	if (l1.x > r2.x || l2.x > r1.x)
		return false;

	if (r1.y < l2.y || r2.y < l1.y)
		return false;

	return true;
}

bool detect_collision(PipePair *pairs, int pipeCount, Player *player)
{
	if (player->ghostTime < MAX_GHOST_TIME)
		return false;

	for (int i = 0; i < pipeCount; i++)
	{
		PipePair *pair = &pairs[i];

		Vector2 topPipeTopLeft = {.x = pair->container.x, .y = pair->container.y};
		Vector2 topPipeBottRight = {.x = pair->container.x + pair->container.width, .y = pair->container.y + pair->topHeight};
		Vector2 bottPipeTopLeft = {.x = pair->container.x, .y = pair->container.y + pair->container.height - pair->bottHeight};
		Vector2 bottPipeBottRight = {.x = pair->container.x + pair->container.width, .y = pair->container.y + pair->container.height};

		Vector2 playerTopLeft = {.x = player->position.x - player->size / 2, .y = player->position.y - player->size / 2};
		Vector2 playerBottRight = {.x = player->position.x + player->size / 2, .y = player->position.y + player->size / 2};

		bool intersect = rects_overlap(topPipeTopLeft, topPipeBottRight, playerTopLeft, playerBottRight) ||
						 rects_overlap(bottPipeTopLeft, bottPipeBottRight, playerTopLeft, playerBottRight);

		if (intersect)
			return true;
	}

	return false;
}

void respawn(Player *player)
{
	player->lives -= 1;
	player->position.y = GetScreenHeight() / 2;
	player->ghostTime = 0;
}

bool fell_off(Player *player)
{
	bool isWayUp = player->position.y + player->size / 2 < 0;
	bool isWayDown = player->position.y - player->size / 2 > GetScreenHeight();

	return isWayUp || isWayDown;
}

void update_bg_rects(BgRect *rects, int rectCount)
{
	float speed = -2.5;
	float rightShift = rects[0].textureData->width * (rectCount - 1);

	for (int i = 0; i < rectCount; i++)
	{
		rects[i].position.x += speed;
	}

	for (int i = 0; i < rectCount; i++)
	{
		if (rects[i].position.x + rects[i].textureData->width < 0)
		{
			float furthest = 0;

			for (int j = 0; j < rectCount; j++)
			{
				float extreme = rects[j].position.x + rects[j].textureData->width;

				if (extreme > furthest)
					furthest = extreme;
			}

			rects[i].position.x = furthest;
		}
	}
}

void draw_bg_rects(BgRect *bgRects, int rectCount)
{
	for (int i = 0; i < rectCount; i++)
	{
		DrawTextureEx(
			*(bgRects[i].textureData->texture),
			bgRects[i].position,
			0,
			bgRects[i].textureData->scale,
			WHITE);
	}
}

void play(enum State *state,
		  BgRect bgRects[],
		  int bgRectCount,
		  PipePair *pairs,
		  int pipeCount,
		  int *firstIdx,
		  int pipeSpacing,
		  Player *player,
		  int *score,
		  bool *keyUpPressed)
{
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

	if (IsKeyPressed(KEY_P))
	{
		*state = PAUSED;

		return;
	}

	player->acceleration.y += gravity;

	update_bg_rects(bgRects, bgRectCount);
	draw_bg_rects(bgRects, bgRectCount);

	update_pipes(pairs, pipeCount);
	draw_pipes(pairs,
			   pipeCount,
			   firstIdx,
			   pipeSpacing);

	update_player(player);
	draw_player(player);

	draw_score(*score);
	draw_lives(player->lives);

	bool fellOff = fell_off(player);

	if (fellOff)
		respawn(player);

	bool collided = detect_collision(pairs, pipeCount, player);

	if (collided)
	{
		player->lives -= 1;
		player->ghostTime = 0;
	}

	if (player->lives == 0)
	{
		*state = GAME_OVER;
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

PipePair *make_pipes(int pipeCount, int pipeWidth, int pipeSpacing, int playerSize, PipeTexture *texture)
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
			.texture = texture};

		randomize_pipe_size(&pair, playerSize);

		pipes[i] = pair;
	}

	return pipes;
}

int count_bg_rects(float bgImageWidth)
{
	return ceil(GetScreenWidth() / bgImageWidth) + 1;
}

BgRect *make_bg_rects(int rectCount, BgTextureData *bgTextData)
{
	BgRect *rects = malloc(rectCount * sizeof(*rects));

	for (int i = 0; i < rectCount; i++)
	{
		Vector2 position = {.x = i * bgTextData->width, .y = 0};
		BgRect rect = {.position = position, .textureData = bgTextData};

		rects[i] = rect;
	}

	return rects;
}

void reset_actors(int *score, Player *player, PipePair *pairs, int pairCount, int *firstIdx, int pipeWidth, int pipeSpacing)
{
	*score = 0;
	player->position.y = 300;
	player->ghostTime = MAX_GHOST_TIME;
	player->lives = MAX_LIVES;

	for (int i = 0; i < pairCount; i++)
		pairs[i].container.x = i * (pipeWidth + pipeSpacing) + WIDTH;

	*firstIdx = 0;
}

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	InitWindow(WIDTH, HEIGHT, "Flappy Bird");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	SetTargetFPS(60);
	enum State state = START;
	Texture2D playerImage = LoadTexture("bird.png");
	float imageSize = 256;
	float playerSize = 30;
	float imageScale = playerSize / imageSize;
	Player player = {
		.position = {WIDTH / 10, 300},
		.velocity = {0, 0},
		.acceleration = {0, 0},
		.image = playerImage,
		.lives = MAX_LIVES,
		.imageScale = imageScale,
		.ghostTime = MAX_GHOST_TIME,
		.size = (int)playerSize};
	int score = 0;
	bool keyUpPressed = false;

	int lives = MAX_LIVES;
	int firstIdx = 0;
	int pipeWidth = 50;
	int pipeSpacing = 200;
	int pipeCount = compute_obstacle_count(pipeWidth, pipeSpacing);
	Texture2D pipeImage = LoadTexture("pipe.png");
	int pipeImageNaturalWidth = 128;
	int pipeImageNaturalHeight = 1024;
	float pipeImageScale = (float)pipeWidth / pipeImageNaturalWidth;
	int pipeImageWidth = pipeWidth;
	int pipeImageHeight = pipeImageNaturalHeight * pipeImageScale;
	PipeTexture pipeTexture = {.image = pipeImage,
							   .width = pipeImageWidth,
							   .height = pipeImageHeight,
							   .scale = pipeImageScale};
	PipePair *pipes = make_pipes(pipeCount, pipeWidth, pipeSpacing, player.size, &pipeTexture);
	Texture2D bgImage = LoadTexture("background.png");
	int bgImageNaturalWidth = 5120;
	int bgImageNaturalHeight = 1024;
	float bgImageScale = (float)GetScreenHeight() / bgImageNaturalHeight;
	float bgRectWidth = bgImageNaturalWidth * bgImageScale;
	BgTextureData bgTextureData = {.texture = &bgImage, .scale = bgImageScale, .width = bgRectWidth};
	int bgRectCount = count_bg_rects(bgRectWidth);
	BgRect *bgRects = make_bg_rects(bgRectCount, &bgTextureData);

	int logoSize = 100;
	float logoScale = (float)logoSize / imageSize;

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(RAYWHITE);

		if (state == START)
		{
			reset_actors(&score, &player, pipes, pipeCount, &firstIdx, pipeWidth, pipeSpacing);
			draw_start_screen(&state, &playerImage, logoSize, logoScale);
		}
		else if (state == PLAYING)
			play(&state,
				 bgRects,
				 bgRectCount,
				 pipes,
				 pipeCount,
				 &firstIdx,
				 pipeSpacing,
				 &player,
				 &score,
				 &keyUpPressed);
		else if (state == PAUSED)
			draw_paused_screen(&state);
		else if (state == GAME_OVER)
			draw_game_over_screen(&state, score);

		EndDrawing();
	}

	UnloadTexture(playerImage);
	UnloadTexture(pipeImage);
	UnloadTexture(bgImage);

	free(pipes);
	free(bgRects);

	CloseWindow();

	return 0;
}
