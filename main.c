#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "Externals/dialogue.c"

typedef enum {MAIN_SCREEN = 0, MAIN_GAME, SETTINGS } GameScreen;
typedef struct {int health; int rating} Player;

int main(void){
    InitWindow(1500, 900, "CU");

    GameScreen currentGameScene = MAIN_SCREEN;

    // Camera
    Camera2D camera = (Camera2D){0};
    camera.zoom = 1;
    camera.target = (Vector2) {0, 0};
    // Camera End

    // Dialogue
    cJSON* jsonData = GetDialogueJson("Assets/Dialogue/test.json");
    if (jsonData == NULL){
        return -1;
    }
    char*** dialogue = GetDialogueData(jsonData);
    int dialogueLen = GetDialogueLen(jsonData);
    int currentDialogueIndex = 0;
    
    Font dialogueFont;
    DialogueGenerateFontAtlas(&dialogueFont, dialogue, dialogueLen);
    // Dialogue End

    Font englishFont = LoadFontEx("Assets/Fonts/SB Aggro M.ttf", 70, 0, 0);
    SetTextureFilter(englishFont.texture, TEXTURE_FILTER_BILINEAR);

    Player playerStatus = (Player){100, 100};
    char healthText[50] = "Health : 100";
    char ratingText[50] = "Store Rating : 100";

    Texture playerNormalTexture = LoadTexture("Assets/Images/Player/Normal.png");
    Texture counterPovBackgroundTexture = LoadTexture("Assets/Images/Backgrounds/CounterBack.png");
    Texture counterPovForegroundTexture = LoadTexture("Assets/Images/Backgrounds/CounterFore.png");

    Texture vignetteEffect = LoadTexture("Assets/Images/UI Effects/Vignette.png");

    Texture currentCustomerTexture = LoadTexture("Assets/Images/Customers/Normal Customer/Idle.png");
    float customerPosX = -currentCustomerTexture.width;

    while (!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        printf("DT : %2f\n", deltaTime);
        switch (currentGameScene)
        {
            case MAIN_SCREEN:
                if (IsKeyPressed(KEY_SPACE)){
                    currentGameScene = MAIN_GAME;
                }

                BeginDrawing();

                    DrawText("CU SIMULATOR", 450, 200, 80, WHITE);
                    DrawText("[SPACE] to start", 560, 560, 40, WHITE);
                    DrawFPS(1400, 10);

                EndDrawing();
                break;
            case MAIN_GAME:
                // Next Dialogue Key
                if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_SPACE) ){
                    int prevIndex = currentDialogueIndex;
                    currentDialogueIndex = fmin(dialogueLen - 1, currentDialogueIndex + 1);
                    if (prevIndex != currentDialogueIndex){
                        DialogueGenerateFontAtlas(&dialogueFont, dialogue, dialogueLen);
                    }
                    else{
                        printf("마지막 대화임!!!\n");
                    }
                }
                
                customerPosX = fmin(300, customerPosX + 1500 * deltaTime);

                BeginDrawing();

                    BeginMode2D(camera);

                    EndMode2D();

                    DrawTexture(counterPovBackgroundTexture, 0, 0, WHITE);
                    // Draw Customer
                    DrawTextureV(currentCustomerTexture, (Vector2){customerPosX, 100}, WHITE);
                    DrawTexture(counterPovForegroundTexture, 0, 0, WHITE);
                    //DrawTexture(vignetteEffect, 0, 0, BLACK);

                    // Dialogue UI
                    DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);
                    DrawTextEx(dialogueFont, dialogue[currentDialogueIndex][0], (Vector2){30, GetScreenHeight() - 180}, 30, 1, WHITE);
                    DrawTextEx(dialogueFont, dialogue[currentDialogueIndex][1], (Vector2){50, GetScreenHeight() - 140}, 30, 1, WHITE);
                    // Dialogue UI End

                    // Player Stats UI
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, GetScreenHeight()}, BLACK); // 
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, 120}, WHITE); // Time, Item UI
                    DrawTexture(playerNormalTexture, GetScreenWidth()-400, 0, WHITE);
                    
                    sprintf(healthText, "Health : %d", playerStatus.health);
                    sprintf(ratingText, "Store Rating : %d", playerStatus.rating);
                    
                    DrawTextEx(englishFont, healthText, (Vector2){GetScreenWidth()-390, 10}, 25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 35, 300, 5}, RED);
                    
                    DrawTextEx(englishFont, ratingText, (Vector2){GetScreenWidth()-390, 50},25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 80, 300, 5}, BLUE);
                    // Player Stats UI End

                    DrawFPS(1400, 10);

                EndDrawing();
                break;
            default:
                break;
        }
        ClearBackground(BLUE);
        
    }

    CloseWindow();

    return 0;
}