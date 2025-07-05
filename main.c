#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "skyweave.h"

#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

const int screenWidth = 800;
const int screenHeight = 540;
Color goalColor = {20, 20, 20, 255};

Vector2 scaleObject = {1.0f, 1.5f};
Font robotoFont;
int p1Score = 0;
int p2Score = 0;
bool isGamePaused = false;

struct GameObject
{
    float speed;
    Vector2 position;
    Texture2D texture;
};

void DisplayText(Font font, int fontSize, Color color, Vector2 position)
{
    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "FPS:%d", GetFPS());
    DrawTextEx(font, fpsText, position, fontSize, 10, color);
}

void ResetBall(Vector2 *position, Vector2 *velocity, bool toLeft)
{
    *position = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    *velocity = (Vector2){(toLeft ? -600 : 600), 0};
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(screenWidth, screenHeight, "Poong");
    SetExitKey(KEY_NULL);

    robotoFont = LoadFont("resources/font/roboto.ttf");

    // -- Player Setup --
    struct GameObject player1 = {.speed = 400, .position = {100.0f, 100.0f}};
    struct GameObject player2 = {.speed = 400, .position = {screenWidth - 100.0f, 100.0f}};

    Image player1Image = LoadImage("resources/textures/player_concept.png");
    Image player2Image = LoadImage("resources/textures/player_concept.png");

    player1.texture = LoadTextureFromImage(player1Image);
    player2.texture = LoadTextureFromImage(player2Image);

    UnloadImage(player1Image);
    UnloadImage(player2Image);

    // -- Ball Setup --
    Vector2 ballPosition = {screenWidth / 2.0f, screenHeight / 2.0f};
    Vector2 ballVelocity = {600, 0};
    float ballRadius = 5;

    Rectangle barrier = {screenWidth / 2.0f - 5, screenHeight / 2.0f - 100, 10, 200};
    Rectangle p1Goal = {player1.position.x - 100, screenHeight / 2.0f - 300, 5, 1000};
    Rectangle p2Goal = {player2.position.x + 95, screenHeight / 2.0f - 300, 5, 1000};

    SetTargetFPS(120);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // -- Pause Toggle --
        if (IsKeyPressed(KEY_P))
        {
            isGamePaused = !isGamePaused;
        }

        if (!isGamePaused)
        {
            // -- Paddle Movement --
            if (IsKeyDown(KEY_W)) player1.position.y -= player1.speed * deltaTime;
            if (IsKeyDown(KEY_S)) player1.position.y += player1.speed * deltaTime;
            if (IsKeyDown(KEY_UP)) player2.position.y -= player2.speed * deltaTime;
            if (IsKeyDown(KEY_DOWN)) player2.position.y += player2.speed * deltaTime;

            float scaledHeight = player1.texture.height * scaleObject.y;
            player1.position.y = CLAMP(player1.position.y, 0, screenHeight - scaledHeight);
            player2.position.y = CLAMP(player2.position.y, 0, screenHeight - scaledHeight);

            // -- Ball Movement --
            ballPosition.x += ballVelocity.x * deltaTime;
            ballPosition.y += ballVelocity.y * deltaTime;

            // -- Wall Collision --
            if ((ballPosition.y - ballRadius <= 0 && ballVelocity.y < 0) ||
                (ballPosition.y + ballRadius >= screenHeight && ballVelocity.y > 0))
            {
                ballVelocity.y *= -1;
            }

            // -- Scoring --
            if (CheckCollisionCircleRec(ballPosition, ballRadius, p1Goal))
            {
                p2Score++;
                //ResetBall(&ballPosition, &ballVelocity, true); //Reset ball position after scored
            }

            if (CheckCollisionCircleRec(ballPosition, ballRadius, p2Goal))
            {
                p1Score++;
                //ResetBall(&ballPosition, &ballVelocity, false);
            }

            // -- Paddle Collision Boxes --
            Rectangle player1Rect = {
                player1.position.x,
                player1.position.y,
                player1.texture.width * scaleObject.x,
                player1.texture.height * scaleObject.y};

            Rectangle player2Rect = {
                player2.position.x,
                player2.position.y,
                player2.texture.width * scaleObject.x,
                player2.texture.height * scaleObject.y};

            // -- Ball vs Paddle --
            if (CheckCollisionCircleRec(ballPosition, ballRadius, player1Rect) && ballVelocity.x < 0)
            {
                ballVelocity.x *= -1;
                float dy = (ballPosition.y - (player1Rect.y + player1Rect.height / 2)) / (player1Rect.height / 2);
                ballVelocity.y = dy * 250;
                ballPosition.x = player1Rect.x + player1Rect.width + ballRadius;
            }

            if (CheckCollisionCircleRec(ballPosition, ballRadius, player2Rect) && ballVelocity.x > 0)
            {
                ballVelocity.x *= -1;
                float dy = (ballPosition.y - (player2Rect.y + player2Rect.height / 2)) / (player2Rect.height / 2);
                ballVelocity.y = dy * 250;
                ballPosition.x = player2Rect.x - ballRadius;
            }
        }

        // -- Rendering --
        BeginDrawing();
        ClearBackground(BLACK);

        DisplayText(robotoFont, 20, RAYWHITE, (Vector2){10, 10});

        DrawRectangleRec(barrier, DARKGRAY);
        DrawRectangleRec(p1Goal, goalColor);
        DrawRectangleRec(p2Goal, goalColor);

        DrawCircleV(ballPosition, ballRadius, WHITE);

        const char *p1ScoreText = TextFormat("%d", p1Score);
        const char *p2ScoreText = TextFormat("%d", p2Score);

        DrawText(p1ScoreText, 300, 20, 20, WHITE);
        DrawText(" - ", 365, 20, 50, WHITE);
        DrawText(p2ScoreText, 500, 20, 20, WHITE);

        DrawText("Player: 1", player1.position.x, player1.position.y - 30, 20, RED);
        DrawText("Player: 2", player2.position.x, player2.position.y - 30, 20, RED);

        DrawTextureEx(player1.texture, player1.position, 0.0f, scaleObject.y, WHITE);
        DrawTextureEx(player2.texture, player2.position, 0.0f, scaleObject.y, WHITE);

        if (isGamePaused)
        {
            DrawText("PAUSED", screenWidth / 2 - 100, screenHeight / 2 - 50, 50, GREEN);
        }

        EndDrawing();
    }

    // -- Cleanup --
    UnloadTexture(player1.texture);
    UnloadTexture(player2.texture);
    UnloadFont(robotoFont);
    CloseWindow();

    return 0;
}
