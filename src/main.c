/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include <stdbool.h>
#include <stdio.h>
#include "raylib.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir

typedef struct
{
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
	int radius;
} Body;

void update_body(Body *body)
{
	body->velocity.x += body->acceleration.x;
	body->velocity.y += body->acceleration.y;

	body->position.x += body->velocity.x;
	body->position.y += body->velocity.y;

	body->acceleration.x = 0;
	body->acceleration.y = 0;
}

enum State
{
	START,
	PLAYING,
	PAUSED,
	GAME_OVER
};

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

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))

	{
		Vector2 mouse = GetMousePosition();

		bool isWithinX = buttonX <= mouse.x && mouse.x <= buttonX + buttonWidth;
		bool isWithinY = buttonY <= mouse.y && mouse.y <= buttonY + buttonHeight;

		if (isWithinX && isWithinY)
			*state = PLAYING;
		}
}

int main()
{
	enum State state = START;
	int score = 0;

	double gravity = .01;
	Body ball = {.position = {400, 300}, .velocity = {0, 0}, .acceleration = {0, 0}, .radius = 10};

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(800, 600, "Hello Raylib");

	SetTargetFPS(60);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	// Texture wabbit = LoadTexture("wabbit_alpha.png");

	// game loop

	while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or presses the Close button on the window
	{

		BeginDrawing();

		ClearBackground(RAYWHITE);

		if (state == START)
			draw_start_screen(&state);

		EndDrawing();

		// ball.acceleration.y = gravity;

		// if (IsKeyDown(KEY_UP))
		// {
		// 	ball.velocity.y -= .1;
		// }

		// update_body(&ball);

		// // drawing
		// BeginDrawing();

		// // Setup the back buffer for drawing (clear color and depth buffers)
		// ClearBackground(BLACK);

		// // draw our texture to the screen
		// // DrawTexture(wabbit, 400, 200, WHITE);

		// DrawCircleV(ball.position, ball.radius, RED);

		// // end the frame and get ready for the next one  (display frame, poll input, etc...)
		// EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	// UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
