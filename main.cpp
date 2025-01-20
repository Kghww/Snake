#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define WINDOW_X 100
#define WINDOW_Y 100

#define GRID_DIM 600    // Visual size of the board
#define GRID_COUNT 10   // Number of fields on the board

#define SNAKE_BASE_SPEED 0.15
#define SPEED_BOOST 10  // Buff in % of actual speed
#define SPEED_TIME_INTERVAL 5  // Time in seconds

#define BLUE_POINT_SCORING 10
#define RED_POINT_SCORING 50

#define NUMBER_OF_REMOVING_SEGMENTS 2

#define SLOW_BOOST 20 // Debuff in % of actual speed
#define RED_POINT_MAX_LIFETIME 5
#define CHANCE_OF_SPAWN 15000 // Bigger number -> lower chance 

enum {
    SNAKE_UP,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT,
};

typedef struct snake {
    int x;
    int y;
    int direction;
    struct snake* next;
} Snake;

typedef struct snake_data {
    Snake* head;
    Snake* tail;
    int prevTailX;
    int prevTailY;
} SnakeData;

typedef struct game_info {
    double worldTime;          
    double fpsTimer;           
    double fps;                
    int frames;                
    int gameOver;              
    double snakeMoveTimer;     
    double snakeMoveDelay;     
    char text[128];            
    char text_req[128];
    char text2[128];
    char text3[128];
    int info_box_x;            
    int info_box_y;            
    int info_box_w;            
    int info_box_h;            
    int grid_x;                
    int grid_y;                
    int points;
    int lastBoostTime;
} GameInfo;

typedef struct point {
    int x;
    int y;
} Point;





void DrawString(SDL_Renderer* renderer, SDL_Texture* charsetTexture, int x, int y, const char* text) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while (*text) {
        c = *text & 255;
        px = (c % 16) * 8;
        py = (c / 16) * 8;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_RenderCopy(renderer, charsetTexture, &s, &d);
        x += 8;
        text++;
    };
};

void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++) {
        int dx = (int)sqrt(radius * radius - y * y);
        for (int x = -dx; x <= dx; x++) {
            SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        }
    }
}

int InitializeGame(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** charsetTexture) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    *window = SDL_CreateWindow("Snake", WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* charsetSurface = SDL_LoadBMP("./cs8x8.bmp");
    if (charsetSurface == NULL) {
        printf("Couldn't load charset! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 1;
    }
    SDL_SetColorKey(charsetSurface, SDL_TRUE, SDL_MapRGB(charsetSurface->format, 0, 0, 0));
    *charsetTexture = SDL_CreateTextureFromSurface(*renderer, charsetSurface);
    SDL_FreeSurface(charsetSurface);

    if (*charsetTexture == NULL) {
        printf("Couldn't create charset texture! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(*renderer, 0x11, 0x11, 0x11, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);

    return 0;
}

void InitializeGameInfo(GameInfo* gameInfo) {
    gameInfo->worldTime = 0.0;
    gameInfo->fpsTimer = 0.0;
    gameInfo->fps = 0.0;
    gameInfo->frames = 0;
    gameInfo->gameOver = 0;
    gameInfo->snakeMoveTimer = 0.0;
    gameInfo->snakeMoveDelay = SNAKE_BASE_SPEED;
    gameInfo->info_box_x = 0;
    gameInfo->info_box_y = 0;
    gameInfo->info_box_w = WINDOW_WIDTH;
    gameInfo->info_box_h = WINDOW_HEIGHT * 0.07;
    gameInfo->grid_x = WINDOW_WIDTH / 2 - GRID_DIM / 2;
    gameInfo->grid_y = WINDOW_HEIGHT / 2 - GRID_DIM / 2;
    gameInfo->points = 0;
    gameInfo->lastBoostTime = -1;

}

void RenderGrid(SDL_Renderer* renderer, GameInfo* gameInfo) {

    SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, 255);

    SDL_Rect gridFrame;
    gridFrame.x = gameInfo->grid_x;
    gridFrame.y = gameInfo->grid_y;
    gridFrame.w = GRID_DIM;
    gridFrame.h = GRID_DIM;

    SDL_RenderDrawRect(renderer, &gridFrame);

    /*
    int cell_size = GRID_DIM / GRID_COUNT;

    SDL_Rect cell;
    cell.w = cell_size;
    cell.h = cell_size;

    for (int i = 0; i < GRID_COUNT; i++) {
        for (int j = 0; j < GRID_COUNT; j++) {
            cell.x = gameInfo->grid_x + (i * cell_size);
            cell.y = gameInfo->grid_y + (j * cell_size);
            SDL_RenderDrawRect(renderer, &cell);
        }
    } */

    return;
}

void DrawInfoBox(SDL_Renderer* renderer, GameInfo* gameInfo) {
    SDL_Rect info_box = {gameInfo->info_box_x, gameInfo->info_box_y, gameInfo->info_box_w, gameInfo->info_box_h};
    SDL_SetRenderDrawColor(renderer, 120, 15, 60, 255); 
    SDL_RenderFillRect(renderer, &info_box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_RenderDrawRect(renderer, &info_box);
}

void DrawEndgameInfoBox(SDL_Renderer* renderer, GameInfo *gameInfo) {
    SDL_Rect eg_info_box = { gameInfo->grid_x, gameInfo->grid_y + GRID_DIM / 2, GRID_DIM, 40 };
    SDL_SetRenderDrawColor(renderer, 120, 15, 60, 255); 
    SDL_RenderFillRect(renderer, &eg_info_box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_RenderDrawRect(renderer, &eg_info_box);
}

void InitializeSnake(SnakeData* snakeData) {
    Snake* head = (Snake*)malloc(sizeof(Snake));
    if (head == NULL) {
        printf("Error: Memory allocation failed for snake head.\n");
    }

    head->x = GRID_COUNT / 2;
    head->y = GRID_COUNT / 2;
    head->direction = SNAKE_RIGHT;
    head->next = NULL;

    snakeData->head = head;
    snakeData->tail = head;
}

void InitializeBluePoint(Point* bluePoint, SnakeData* snakeData) {
    int found = 1;
    while (found) {
        bluePoint->x = rand() % GRID_COUNT;
        bluePoint->y = rand() % GRID_COUNT;
        found = 0;  

        Snake* current = snakeData->head;
        while (current != NULL) {
            if (bluePoint->x == current->x && bluePoint->y == current->y) {
                found = 1;
                break;
            }
            current = current->next;
        }
    }
}

void InitializeRedPoint(Point* redPoint, Point* bluePoint, SnakeData* snakeData) {
    int found = 1;
    while (found) {
        redPoint->x = rand() % GRID_COUNT;
        redPoint->y = rand() % GRID_COUNT;
        found = 0;  

        Snake* current = snakeData->head;
        while (current != NULL) {
            if ((redPoint->x == current->x) && (redPoint->y == current->y) && (redPoint -> x == bluePoint -> x) && (redPoint -> y == bluePoint -> y)) {
                found = 1;
                break;
            }
            current = current->next;
        }
    }
}

void IncreaseSnake(SnakeData* snakeData) {
    Snake* newSegment = (Snake*)malloc(sizeof(Snake));
    if (newSegment == NULL) {
        printf("Error: Memory allocation failed for new snake segment.\n");
    }

    // Data of the new segment is set to previous snake's segment tail
    newSegment->x = snakeData->prevTailX;
    newSegment->y = snakeData->prevTailY;

    newSegment->direction = snakeData->tail->direction;
    newSegment->next = NULL;

    // Adding segment on the end of the snake
    snakeData->tail->next = newSegment;
    snakeData->tail = newSegment;
}

void MoveSnake(SnakeData* snakeData) {
    Snake* current = snakeData->head;

    snakeData->prevTailX = snakeData->tail->x;
    snakeData->prevTailY = snakeData->tail->y;

    int prevX = current->x;
    int prevY = current->y;
    int prevDirection = current->direction;

    if (current->direction == SNAKE_UP) {
        current->y--;
        if (current->y < 0) {
            current->y++;
            if (current->x < GRID_COUNT - 1) {
                current->x++;
                current->direction = SNAKE_RIGHT;
            }
            else {
                current->x--;
                current->direction = SNAKE_LEFT;
            }
        }
    }
    else if (current->direction == SNAKE_DOWN) {
        current->y++;
        if (current->y == GRID_COUNT) {
            current->y--;
            if (current->x == 0) {
                current->x++;
                current->direction = SNAKE_RIGHT;
            }
            else {
                current->x--;
                current->direction = SNAKE_LEFT;
            }
        }
    }
    else if (current->direction == SNAKE_LEFT) {
        current->x--;
        if (current->x < 0) {
            current->x++;
            if (current->y > 0) {
                current->y--;
                current->direction = SNAKE_UP;
            }
            else {
                current->y++;
                current->direction = SNAKE_DOWN;
            }
        }
    }
    else if (current->direction == SNAKE_RIGHT) {
        current->x++;
        if (current->x == GRID_COUNT) {
            current->x--;
            if (current->y < GRID_COUNT - 1) {
                current->y++;
                current->direction = SNAKE_DOWN;
            }
            else {
                current->y--;
                current->direction = SNAKE_UP;
            }
        }
    }

    // Move the rest of the snake
    current = current->next;
    while (current != NULL) {
        int tempX = current->x;
        int tempY = current->y;
        int tempDirection = current->direction;

        current->x = prevX;
        current->y = prevY;
        current->direction = prevDirection;

        prevX = tempX;
        prevY = tempY;
        prevDirection = tempDirection;

        current = current->next;
    }
}

void RenderSnake(SDL_Renderer* renderer, SnakeData* snakeData, GameInfo* gameInfo) {
    int cell_size = GRID_DIM / GRID_COUNT;

    Snake* current = snakeData->head;
    int isHead = 1;

    while (current != NULL) {
        SDL_Rect segmentRect = {
            gameInfo->grid_x + (current->x * cell_size),
            gameInfo->grid_y + (current->y * cell_size),
            cell_size,
            cell_size
        };

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &segmentRect);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderDrawRect(renderer, &segmentRect);

        if (isHead) {
            // Drawing eyes
            int eyeSize = cell_size / 5;
            SDL_Rect leftEye, rightEye;

            switch (current->direction) {
            case SNAKE_UP:
                leftEye.x = segmentRect.x + cell_size / 4;
                leftEye.y = segmentRect.y + cell_size / 8;
                leftEye.w = eyeSize;
                leftEye.h = eyeSize;

                rightEye.x = segmentRect.x + 3 * cell_size / 4 - eyeSize;
                rightEye.y = segmentRect.y + cell_size / 8;
                rightEye.w = eyeSize;
                rightEye.h = eyeSize;
                break;

            case SNAKE_DOWN:
                leftEye.x = segmentRect.x + cell_size / 4;
                leftEye.y = segmentRect.y + 7 * cell_size / 8 - eyeSize;
                leftEye.w = eyeSize;
                leftEye.h = eyeSize;

                rightEye.x = segmentRect.x + 3 * cell_size / 4 - eyeSize;
                rightEye.y = segmentRect.y + 7 * cell_size / 8 - eyeSize;
                rightEye.w = eyeSize;
                rightEye.h = eyeSize;
                break;

            case SNAKE_LEFT:
                leftEye.x = segmentRect.x + cell_size / 8;
                leftEye.y = segmentRect.y + cell_size / 4;
                leftEye.w = eyeSize;
                leftEye.h = eyeSize;

                rightEye.x = segmentRect.x + cell_size / 8;
                rightEye.y = segmentRect.y + 3 * cell_size / 4 - eyeSize;
                rightEye.w = eyeSize;
                rightEye.h = eyeSize;
                break;

            case SNAKE_RIGHT:
                leftEye.x = segmentRect.x + 7 * cell_size / 8 - eyeSize;
                leftEye.y = segmentRect.y + cell_size / 4;
                leftEye.w = eyeSize;
                leftEye.h = eyeSize;

                rightEye.x = segmentRect.x + 7 * cell_size / 8 - eyeSize;
                rightEye.y = segmentRect.y + 3 * cell_size / 4 - eyeSize;
                rightEye.w = eyeSize;
                rightEye.h = eyeSize;
                break;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &leftEye);
            SDL_RenderFillRect(renderer, &rightEye);
        }

        isHead = 0;
        current = current->next;
    }
}

void RenderBluePoint(SDL_Renderer* renderer, Point* bluePoint, GameInfo* gameInfo) {
    int cell_size = GRID_DIM / GRID_COUNT;
    int centerX = gameInfo->grid_x + bluePoint->x * cell_size + cell_size / 2;
    int centerY = gameInfo->grid_y + bluePoint->y * cell_size + cell_size / 2;
    int radius = cell_size / 3;

    SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
    DrawFilledCircle(renderer, centerX, centerY, radius);
}

void RenderRedPoint(SDL_Renderer* renderer, Point* redPoint, GameInfo* gameInfo) {
    int cell_size = GRID_DIM / GRID_COUNT;
    int centerX = gameInfo->grid_x + redPoint->x * cell_size + cell_size / 2;
    int centerY = gameInfo->grid_y + redPoint->y * cell_size + cell_size / 2;
    int radius = cell_size / 2;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    DrawFilledCircle(renderer, centerX, centerY, radius);
}

void FreeSnake(SnakeData* snakeData) {
    Snake* current = snakeData->head;
    while (current != NULL) {
        Snake* next = current->next;
        free(current);
        current = next;
    }
    snakeData->head = NULL;
    snakeData->tail = NULL;
}

void ResetGame(SnakeData* snakeData, GameInfo* gameInfo, Point* bluePoint, int* redPointActive, double* redPointTimer) {
    FreeSnake(snakeData);
    InitializeSnake(snakeData);
    gameInfo->snakeMoveTimer = 0.0;
    gameInfo->worldTime = 0.0;
    gameInfo->gameOver = 0;
    gameInfo->points = 0;
    gameInfo->snakeMoveDelay = 0.15;

    InitializeBluePoint(bluePoint, snakeData);

    *redPointActive = 0;
    *redPointTimer = 0.0;
}

void Input(SDL_Event* event, GameInfo* gameInfo, SnakeData* snakeData, int* directionChanged, int* quit, Point* bluePoint, int* redPointActive, double* redPointTimer) {
    switch (event->type) {
    case SDL_QUIT:
        *quit = 1;
        break;

    case SDL_KEYDOWN:
        if (gameInfo->gameOver) {
            switch (event->key.keysym.sym) {
            case SDLK_ESCAPE:
                *quit = 1;
                break;
            case SDLK_n:
                ResetGame(snakeData, gameInfo, bluePoint, redPointActive, redPointTimer);
                *directionChanged = 0;
                break;
            }
        }
        else {
            if (!*directionChanged) {
                switch (event->key.keysym.sym) {
                case SDLK_ESCAPE:
                    *quit = 1;
                    break;
                case SDLK_UP:
                    if (snakeData->head->direction != SNAKE_DOWN && snakeData->head->y > 0) {
                        snakeData->head->direction = SNAKE_UP;
                        *directionChanged = 1;
                    }
                    break;
                case SDLK_DOWN:
                    if (snakeData->head->direction != SNAKE_UP && snakeData->head->y < GRID_COUNT - 1) {
                        snakeData->head->direction = SNAKE_DOWN;
                        *directionChanged = 1;
                    }
                    break;
                case SDLK_LEFT:
                    if (snakeData->head->direction != SNAKE_RIGHT && snakeData->head->x > 0) {
                        snakeData->head->direction = SNAKE_LEFT;
                        *directionChanged = 1;
                    }
                    break;
                case SDLK_RIGHT:
                    if (snakeData->head->direction != SNAKE_LEFT && snakeData->head->x < GRID_COUNT - 1) {
                        snakeData->head->direction = SNAKE_RIGHT;
                        *directionChanged = 1;
                    }
                    break;
                case SDLK_n:
                    ResetGame(snakeData, gameInfo, bluePoint, redPointActive, redPointTimer);
                    *directionChanged = 0;
                    break;
                }
            }
        }
        break;
    }
}

int Crash(SnakeData* snakeData) {
    Snake* current = snakeData->head->next;
    while (current != NULL) {
        if (snakeData->head->x == current->x && snakeData->head->y == current->y) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

int BluePointDetect(Point* bluePoint, SnakeData* snakeData) {
    Snake* current = snakeData->head;
    while (current != NULL) {
        if (bluePoint->x == current->x && bluePoint->y == current->y) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

int RedPointDetect(Point* redPoint, SnakeData* snakeData){
    Snake* current = snakeData->head;
    while (current != NULL) {
        if (redPoint->x == current->x && redPoint->y == current->y) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void Close(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* charsetTexture) {
    SDL_DestroyTexture(charsetTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void RemoveLastNSegments(SnakeData* snakeData, int n) {
    while (n > 0 && snakeData->tail != snakeData->head) {
        Snake* prev = snakeData->head;
        while (prev->next != snakeData->tail) {
            prev = prev->next;
        }

        free(snakeData->tail);
        prev->next = NULL;
        snakeData->tail = prev;

        n--;
    }
}

int snakeLength(SnakeData* snakeData) {
    int length = 0;
    Snake* current = snakeData->head;
    while (current != NULL) {
        length++;
        current = current->next;
    }
    return length;
}

void DrawProgressBar(SDL_Renderer* renderer, GameInfo* gameInfo, int w, int h, double progress) {
    SDL_Rect borderRect = { gameInfo->grid_x + GRID_DIM + 20, gameInfo->grid_y, w, h };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &borderRect);

    int fillHeight = (int)(h * progress);
    SDL_Rect fillRect = { gameInfo->grid_x + GRID_DIM + 20, gameInfo->grid_y + (h - fillHeight), w, fillHeight };

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &fillRect);
}

void UpdateGameLogic(GameInfo* gameInfo, SnakeData* snakeData, Point* bluePoint, Point* redPoint, int* redPointActive, double* redPointTimer, double delta, int* directionChanged) {
    // Snake speed boost
    int roundedTime = (int)gameInfo->worldTime;
    if (roundedTime % SPEED_TIME_INTERVAL == 0 && roundedTime != gameInfo->lastBoostTime) {
        double speed_effect = (SPEED_BOOST / 100.0) * gameInfo->snakeMoveDelay;
        gameInfo->snakeMoveDelay -= speed_effect;
        if (gameInfo->snakeMoveDelay < 0.05) {
            gameInfo->snakeMoveDelay = 0.05;
        }
        gameInfo->lastBoostTime = roundedTime;
    }

    // Red point init
    if (!gameInfo->gameOver && !(*redPointActive) && (rand() % CHANCE_OF_SPAWN < 2)) {
        InitializeRedPoint(redPoint, bluePoint, snakeData);
        *redPointActive = 1;
        *redPointTimer = 0;
    }

    // Lifespan of red point
    if (*redPointActive) {
        *redPointTimer += delta;
        if (*redPointTimer >= RED_POINT_MAX_LIFETIME) {
            *redPointActive = 0;
        }
    }

    // Snake moving condition
    if ((gameInfo->snakeMoveTimer >= gameInfo->snakeMoveDelay) && !gameInfo->gameOver) {
        if (Crash(snakeData)) {
            gameInfo->gameOver = 1;
        }
        else {
            MoveSnake(snakeData);
            gameInfo->snakeMoveTimer -= gameInfo->snakeMoveDelay;

            *directionChanged = 0;

            if (BluePointDetect(bluePoint, snakeData)) {
                gameInfo->points += BLUE_POINT_SCORING;
                IncreaseSnake(snakeData);
                InitializeBluePoint(bluePoint, snakeData);
            }

            if (*redPointActive && RedPointDetect(redPoint, snakeData)) {
                *redPointActive = 0;
                gameInfo->points += RED_POINT_SCORING;

                if ((rand() % 2 == 0) && (snakeLength(snakeData) > NUMBER_OF_REMOVING_SEGMENTS)) {
                    RemoveLastNSegments(snakeData, NUMBER_OF_REMOVING_SEGMENTS);
                }
                else {
                    double slow_effect = (SLOW_BOOST / 100.0) * gameInfo->snakeMoveDelay;
                    gameInfo->snakeMoveDelay += slow_effect;
                    if (gameInfo->snakeMoveDelay > 0.5) {
                        gameInfo->snakeMoveDelay = 0.5;
                    }
                }
            }
        }
    }
}

void RenderGame(SDL_Renderer* renderer, SDL_Texture* charsetTexture, GameInfo* gameInfo, SnakeData* snakeData, Point* bluePoint, Point* redPoint, int redPointActive, double redPointTimer) {
    sprintf(gameInfo->text, "Elapsed time: %.1lf s | FPS: %.0lf | Points: %d | Press 'n' for new game | Press 'Esc' to exit ",
        gameInfo->worldTime, gameInfo->fps, gameInfo->points);
    sprintf(gameInfo->text3, "\030 - UP      \031 - DOWN     \032 - LEFT     \033 - RIGHT       1,2,3,4 | A,B,C,D");
    sprintf(gameInfo->text2, "Game Over! Total points collected: %d | Total playtime: %.1lf s",
        gameInfo->points, gameInfo->worldTime);

    SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
    SDL_RenderClear(renderer);

    DrawInfoBox(renderer, gameInfo);
    DrawString(renderer, charsetTexture, gameInfo->info_box_x + 20, gameInfo->info_box_y + 20, gameInfo->text);
    DrawString(renderer, charsetTexture, gameInfo->info_box_x + 40, gameInfo->info_box_y + 40, gameInfo->text3);

    RenderGrid(renderer, gameInfo);
    RenderBluePoint(renderer, bluePoint, gameInfo);

    if (redPointActive) {
        RenderRedPoint(renderer, redPoint, gameInfo);

        if (!gameInfo->gameOver) {
            double progress = 1.0 - (redPointTimer / RED_POINT_MAX_LIFETIME);
            if (progress < 0) progress = 0;
            DrawProgressBar(renderer, gameInfo, 20, GRID_DIM, progress);
        }
    }

    RenderSnake(renderer, snakeData, gameInfo);

    if (gameInfo->gameOver) {
        DrawEndgameInfoBox(renderer, gameInfo);
        DrawString(renderer, charsetTexture, gameInfo->grid_x + 30,
            gameInfo->grid_y + GRID_DIM / 2 + 10, gameInfo->text2);
    }

    SDL_RenderPresent(renderer);
}



#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    srand(time(NULL));

    SDL_Event event;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* charsetTexture = NULL;

    if (InitializeGame(&window, &renderer, &charsetTexture) != 0) {
        return 1;
    }

    GameInfo gameInfo;
    InitializeGameInfo(&gameInfo);

    SnakeData snakeData;
    InitializeSnake(&snakeData);

    Point bluePoint;
    Point redPoint;

    int t1 = SDL_GetTicks();
    int quit = 0;
    int directionChanged = 0;
    int redPointActive = 0;
    double redPointTimer = 0;

    InitializeBluePoint(&bluePoint, &snakeData);

    while (!quit) {
        int t2 = SDL_GetTicks();
        double delta = (t2 - t1) * 0.001;
        t1 = t2;

        if (!gameInfo.gameOver) {
            gameInfo.worldTime += delta;
        }
        gameInfo.snakeMoveTimer += delta;

        UpdateGameLogic(&gameInfo, &snakeData, &bluePoint, &redPoint, &redPointActive, &redPointTimer, delta, &directionChanged);
        RenderGame(renderer, charsetTexture, &gameInfo, &snakeData, &bluePoint, &redPoint, redPointActive, redPointTimer);

        while (SDL_PollEvent(&event)) {
            Input(&event, &gameInfo, &snakeData, &directionChanged, &quit, &bluePoint, &redPointActive, &redPointTimer);
        }

        gameInfo.frames++;
    }

    FreeSnake(&snakeData);
    Close(window, renderer, charsetTexture);

    return 0;
}