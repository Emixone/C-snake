#include <random.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MIN_LEVEL_WIDTH  3
#define MIN_LEVEL_HEIGHT 3
#define MAX_LEVEL_WIDTH  20
#define MAX_LEVEL_HEIGHT 20
#define MAX_SPEED        9
#define MAX_PIECES_COUNT (MAX_LEVEL_WIDTH * MAX_LEVEL_HEIGHT)
#define RATIO            (16 / 9)
#define SCREEN_HEIGHT    720
#define SCREEN_WIDTH     (SCREEN_HEIGHT * RATIO)
#define FONT_SIZE        20.f
#define SPEED_CAP        0.047f
#define PIECE_WIDTH      ((SCREEN_HEIGHT * 0.8) / MAX_LEVEL_WIDTH)
#define PADDING          10
#define COLOR_BG         (Color){244, 236, 225, 255}
#define COLOR_SNAKE      (Color){188, 44, 26, 255}
#define COLOR_FRUIT      (Color){208, 179, 17, 255}
#define COLOR_BORDER     BLACK
#define COLOR_TEXT       BLACK

struct SnakePiece {
        int x;
        int y;
};

enum Direction {
        DIR_RIGHT,
        DIR_LEFT,
        DIR_DOWN,
        DIR_UP,
};

enum Screen {
        S_GAME,
        S_MENU,
        S_SCORE,
};

struct SnakePiece pieces[MAX_PIECES_COUNT];
int piecesCount;
enum Direction lastDirection;
enum Direction direction;
float timeFromLastMove;
int fruitPositionX;
int fruitPositionY;
int levelWidth;
int levelHeight;
int speed;
int score;
float scoreBonus;

bool
scoreScreen(void)
{
        int mx = SCREEN_WIDTH / 2;
        int y  = (SCREEN_HEIGHT / 2) - (int)((FONT_SIZE + PADDING) + (FONT_SIZE));

        {
                const char *s = "You scored:";
                int w         = MeasureText(s, FONT_SIZE);
                int x         = mx - (w / 2);
                DrawText(s, x, y, FONT_SIZE, COLOR_TEXT);
                y += FONT_SIZE + PADDING;
        }

        {
                char scoreString[17];
                snprintf(scoreString, sizeof(scoreString), "%d", score);

                int w = MeasureText(scoreString, FONT_SIZE * 2);
                int x = mx - (w / 2);
                DrawText(scoreString, x, y, FONT_SIZE * 2, COLOR_TEXT);
        }

        return IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE);
}

void
updateFruitPosition(void)
{
        bool coliding = true;
        while (coliding) {
                fruitPositionX = randomi32(0, levelWidth);
                fruitPositionY = randomi32(0, levelHeight);
                coliding       = false;

                for (int i = 1; i < piecesCount; i++) {
                        if (pieces[i].x == fruitPositionX &&
                            pieces[i].y == fruitPositionY) {
                                coliding = true;
                        }
                }
        }
}

void
drawMenuItem(char *text, int mx, int y, int *val, int min, int max)
{
        bool buttonPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        int w = MeasureText(text, FONT_SIZE);
        int x = mx - (w / 2);

        DrawText(text, x, y, 20.f, COLOR_TEXT);
        Vector2 t1a = {x - PADDING, y};
        Vector2 t1b = {x - FONT_SIZE - PADDING, y + (FONT_SIZE / 2)};
        Vector2 t1c = {x - PADDING, y + FONT_SIZE};
        Vector2 t2a = {x + w + FONT_SIZE + PADDING, y + (FONT_SIZE / 2)};
        Vector2 t2b = {x + w + PADDING, y};
        Vector2 t2c = {x + w + PADDING, y + FONT_SIZE};
        DrawTriangle(t1a, t1b, t1c, COLOR_TEXT);
        DrawTriangle(t2a, t2b, t2c, COLOR_TEXT);

        if (buttonPressed &&
            CheckCollisionPointTriangle(GetMousePosition(), t1a, t1b, t1c)) {
                if (*val > min) { *val -= 1; }
        }

        if (buttonPressed &&
            CheckCollisionPointTriangle(GetMousePosition(), t2a, t2b, t2c)) {
                if (*val < max) { *val += 1; }
        }
}

bool
menu(void)
{
        int mx = SCREEN_WIDTH / 2;

        int elements = 4;
        int y = (SCREEN_HEIGHT / 2) - ((((int)FONT_SIZE + PADDING) * elements + 20) / 2);

        char widthString[10]  = {0};
        char heightString[11] = {0};
        char speedString[10]  = {0};

        sprintf(widthString, "Width: %d", levelWidth);
        sprintf(heightString, "Height: %d", levelHeight);
        sprintf(speedString, "Speed: %d", speed);

        drawMenuItem(widthString, mx, y, &levelWidth, MIN_LEVEL_WIDTH, MAX_LEVEL_WIDTH);
        y += (FONT_SIZE + PADDING);
        drawMenuItem(heightString, mx, y, &levelHeight, MIN_LEVEL_HEIGHT,
                     MAX_LEVEL_WIDTH);
        y += (FONT_SIZE + PADDING);
        drawMenuItem(speedString, mx, y, &speed, 0, MAX_SPEED);
        y += (FONT_SIZE + PADDING);

        const char *playString = "Play";
        y += PADDING;
        int w   = MeasureText(playString, FONT_SIZE);
        int x   = mx - (w / 2);
        float h = FONT_SIZE + PADDING;
        DrawText(playString, x, y, h, COLOR_TEXT);
        if ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
             CheckCollisionPointRec(GetMousePosition(), (Rectangle){x, y, w, h})) ||
            IsKeyPressed(KEY_SPACE)) {
                return true;
        }

        return false;
}

bool
game(float dt)
{
        bool alive = true;
        timeFromLastMove += dt;

        scoreBonus -= dt * 60.f;

        if (timeFromLastMove > SPEED_CAP * (MAX_SPEED / (float)speed)) {
                timeFromLastMove -= SPEED_CAP * (MAX_SPEED / (float)speed);

                lastDirection = direction;
                for (int i = piecesCount - 1; i > 0; i--) {
                        pieces[i].x = pieces[i - 1].x;
                        pieces[i].y = pieces[i - 1].y;
                }

                if (direction == DIR_UP) { pieces[0].y--; }
                else if (direction == DIR_DOWN) {
                        pieces[0].y++;
                }
                else if (direction == DIR_LEFT) {
                        pieces[0].x--;
                }
                else if (direction == DIR_RIGHT) {
                        pieces[0].x++;
                }

                { /* check for colissions */
                        if (pieces[0].x == -1 || pieces[0].y == -1 ||
                            pieces[0].x == levelWidth || pieces[0].y == levelHeight) {
                                alive = false;
                        }
                        else {
                                for (int i = 1; i < piecesCount; i++) {
                                        if (pieces[0].x == pieces[i].x &&
                                            pieces[0].y == pieces[i].y) {
                                                alive = false;
                                        }
                                }
                        }
                }
        }

        if (IsKeyPressed(KEY_UP) && lastDirection != DIR_DOWN) { direction = DIR_UP; }
        else if (IsKeyPressed(KEY_LEFT) && lastDirection != DIR_RIGHT) {
                direction = DIR_LEFT;
        }
        else if (IsKeyPressed(KEY_DOWN) && lastDirection != DIR_UP) {
                direction = DIR_DOWN;
        }
        else if (IsKeyPressed(KEY_RIGHT) && lastDirection != DIR_LEFT) {
                direction = DIR_RIGHT;
        }

        if (pieces[0].x == fruitPositionX && pieces[0].y == fruitPositionY) {
                pieces[piecesCount] = (struct SnakePiece){
                    .x = pieces[piecesCount - 1].x,
                    .y = pieces[piecesCount - 1].y,
                };
                piecesCount++;
                score += 30;
                if (scoreBonus > 0) { score += (int)scoreBonus; }
                scoreBonus = 100;
                updateFruitPosition();
        }

        int levelBorderW = levelWidth * PIECE_WIDTH;
        int levelBorderH = levelHeight * PIECE_WIDTH;
        int levelBorderX = (SCREEN_WIDTH - levelBorderW) / 2;
        int levelBorderY = ((SCREEN_HEIGHT - levelBorderH) / 2) - (int)FONT_SIZE;

        DrawRectangleLinesEx(
            (Rectangle){levelBorderX, levelBorderY, levelBorderW, levelBorderH}, 3.0f,
            COLOR_BORDER);

        {
                char scoreString[32] = {0};
                sprintf(scoreString, "%d", score);
                int w = MeasureText(scoreString, FONT_SIZE);
                DrawText(scoreString, levelBorderX + levelBorderW - w,
                         levelBorderY + levelBorderH + (FONT_SIZE / 2), FONT_SIZE, COLOR_TEXT);
        }

        DrawRectangle(levelBorderX + fruitPositionX * PIECE_WIDTH + (PIECE_WIDTH / 4),
                      levelBorderY + fruitPositionY * PIECE_WIDTH + (PIECE_WIDTH / 4),
                      PIECE_WIDTH / 2, PIECE_WIDTH / 2, COLOR_FRUIT);

        for (int i = 0; i < piecesCount; i++) {
                DrawRectangle(levelBorderX + pieces[i].x * PIECE_WIDTH,
                              levelBorderY + pieces[i].y * PIECE_WIDTH,
                              PIECE_WIDTH - (PIECE_WIDTH / 4),
                              PIECE_WIDTH - (PIECE_WIDTH / 4), COLOR_SNAKE);
        }

        return ! alive;
}

void
initGame(void)
{
        score            = 0;
        scoreBonus       = 0;
        timeFromLastMove = 0.0;

        memset(pieces, 0, MAX_PIECES_COUNT * sizeof(struct SnakePiece));
        piecesCount = 2;
        pieces[0].x = randomi32(1, levelWidth - 2);
        pieces[0].y = randomi32(1, levelWidth - 2);

        if (pieces[0].x > MAX_LEVEL_WIDTH / 2) {
                direction   = DIR_RIGHT;
                pieces[1].x = pieces[0].x - 1;
                pieces[1].y = pieces[0].y;
        }
        else {
                direction   = DIR_LEFT;
                pieces[1].x = pieces[0].x + 1;
                pieces[1].y = pieces[0].y;
        }
        lastDirection = direction;

        updateFruitPosition();
}

int
main(void)
{
        initRandom();

        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");

        levelWidth    = 10;
        levelHeight   = 10;
        speed         = 3;
        enum Screen s = S_MENU;
        initGame();

        while (! WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(COLOR_BG);
                switch (s) {
                case S_GAME:
                        if (game(GetFrameTime())) { s = S_SCORE; }
                        break;
                case S_MENU:
                        if (menu()) {
                                s = S_GAME;
                                initGame();
                        }
                        break;
                case S_SCORE:
                        if (scoreScreen()) { s = S_MENU; }
                        break;
                }

                EndDrawing();
        }

        CloseWindow();

        return 0;
}
