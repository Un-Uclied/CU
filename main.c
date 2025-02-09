#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "Externals/dialogue.c"

typedef enum {MAIN_SCREEN = 0, MAIN_GAME, SETTINGS } GameScreen;

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
    
    Font dialogueSpeakerFont;
    Font dialogueDialogueFont;
    DialogueGenerateFontAtlas(&dialogueSpeakerFont, &dialogueDialogueFont, dialogue, currentDialogueIndex);
    // Dialogue End

    Texture playerNormalTexture = LoadTexture("Assets/Images/Player/Normal.png");

    while (!WindowShouldClose()){
        switch (currentGameScene)
        {
            case MAIN_SCREEN:
                BeginDrawing();
                    DrawText("CU SIMULATOR", 450, 200, 80, WHITE);
                    DrawFPS(1400, 10);

                EndDrawing();
                break;
            case MAIN_GAME:
                // Next Dialogue Key
                if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_SPACE) ){
                    currentDialogueIndex = fmin(dialogueLen - 1, currentDialogueIndex + 1);
                    DialogueGenerateFontAtlas(&dialogueSpeakerFont, &dialogueDialogueFont, dialogue, currentDialogueIndex);
                }

                BeginDrawing();

                    BeginMode2D(camera);

                    EndMode2D();
                    // Dialogue UI
                    DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);
                    DrawTextEx(dialogueSpeakerFont, dialogue[currentDialogueIndex][0], (Vector2){30, GetScreenHeight() - 180}, 30, 1, WHITE);
                    DrawTextEx(dialogueDialogueFont, dialogue[currentDialogueIndex][1], (Vector2){50, GetScreenHeight() - 140}, 30, 1, WHITE);
                    // Dialogue UI End

                    // Player Stats UI
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, GetScreenHeight()}, BLACK); // 
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, 120}, WHITE); // Time, Item UI
                    DrawTexture(playerNormalTexture, GetScreenWidth()-400, 0, WHITE);
                    // Player Stats End

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