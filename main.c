#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "Externals/dialogue.c"
#include "Externals/button.c"

typedef enum {MAIN_SCREEN = 0, MAIN_GAME, SETTINGS } GameMenu;
typedef enum {SCENE_COUNTER = 0, SCENE_SNACK_BAR, SCENE_BEVERAGE_BAR, SCENE_RAMEN_BAR, SCENE_STUFF_BAR} Scene;
typedef enum {DIFFICULTY_EASY = 0, DIFFICULTY_NORMAL, DIFFICULTY_HARD} Difficulty;
typedef struct {int health; int rating} Player;

// 싱글톤 구조체
typedef struct {
    GameMenu currentGameMenu;
    Difficulty currentDifficulty;
    int currentSceneIndex;

} GameManager;

// 정적 변수로 싱글톤 객체 유지
GameManager* GetGameManager() {
    static GameManager instance = {0};  // 프로그램 실행 중 단 하나만 존재
    return &instance;
}

cJSON* GetJsonData(char* fileName);

void OnPrevSceneButtonClicked();

void OnNextSceneButtonClicked();

void LoadFontAll(Font* font);

char* GetSceneName();

char* GetDifficultyName();

void StartDialogue(cJSON* jsonData, char*** dialogue, int* dialogueLen, int* currentDialogueIndex, char* dialogueFilePath){
    jsonData = GetJsonData(dialogueFilePath);
    dialogue = GetDialogueData(jsonData);
    *dialogueLen = GetDialogueLen(jsonData);
    *currentDialogueIndex = 0;
}

int main(void){
    InitWindow(1500, 900, "CU");
    
    Font font;
    LoadFontAll(&font);
    
    //SetTargetFPS(500);

    GameManager * gm = GetGameManager();
    gm->currentSceneIndex = 0;
    gm->currentDifficulty = DIFFICULTY_NORMAL;

    // Dialogue
    char customerDialoguePath[256];
    char* customerName = "Normal Customer";
    sprintf(customerDialoguePath, "Assets/Dialogue/Customers/%s/%s.json", customerName, GetDifficultyName());

    cJSON* jsonData = GetJsonData(customerDialoguePath);
    char*** dialogue = GetDialogueData(jsonData);
    int dialogueLen = 0;
    int currentDialogueIndex = 0;

    StartDialogue(jsonData, dialogue, &dialogueLen, &currentDialogueIndex, customerDialoguePath);
    // Dialogue End

    // Player Status UI
    Player playerStatus = (Player){100, 100};
    char healthText[50] = "Health : 100";
    char ratingText[50] = "Store Rating : 100";
    // Player Status UI End

    Texture playerNormalTexture = LoadTexture("Assets/Images/Player/Idle.png");
    Texture blank = LoadTexture("Assets/Images/Backgrounds/Blank.png");
    Texture backgrounds[5][2] = {
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), LoadTexture("Assets/Images/Backgrounds/CounterFore.png")},
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), blank},
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), blank},
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), blank},
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), blank},
    };

    Texture vignetteEffect = LoadTexture("Assets/Images/UI Effects/Vignette.png");

    Texture currentCustomerTexture = LoadTexture("Assets/Images/Customers/Normal Customer/Idle.png");
    float customerPosX = -currentCustomerTexture.width;

    // Scene Move Start
    bool canMoveScene = false;
    ButtonUI nextSceneButton = (ButtonUI){(Rectangle){1000, GetScreenHeight() - 120, 100, 100},
        LoadTexture("Assets/Images/UI/Go Right.png"),
        LoadTexture("Assets/Images/UI/Go Right Pressed.png"),
        LoadTexture("Assets/Images/UI/Go Right Hovered.png"),
        NULL,
        OnNextSceneButtonClicked
    };
    nextSceneButton.currentTexture = &nextSceneButton.normalTexture;

    ButtonUI prevSceneButton = (ButtonUI){(Rectangle){850, GetScreenHeight() - 120, 100, 100},
        LoadTexture("Assets/Images/UI/Go Left.png"),
        LoadTexture("Assets/Images/UI/Go Left Pressed.png"),
        LoadTexture("Assets/Images/UI/Go Left Hovered.png"),
        NULL,
        OnPrevSceneButtonClicked
    };
    prevSceneButton.currentTexture = &prevSceneButton.normalTexture;

    ButtonUI sceneMoveBtns[2] = {prevSceneButton, nextSceneButton};
    // Scene Move End

    // Hit Boxes Start
    Rectangle posMachine = (Rectangle) {820, 280, 280, 420};
    Rectangle cardMachine = (Rectangle) {550, 600, 200, 100};
    // Hit Boxes End

    while (!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        switch (gm->currentGameMenu)
        {
            case MAIN_SCREEN:
                if (IsKeyPressed(KEY_SPACE)){
                    gm->currentGameMenu = MAIN_GAME;
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
                    if (prevIndex == currentDialogueIndex){
                        canMoveScene = true;
                        printf("마지막 대화임!!!\n");
                    }
                }
                
                customerPosX = fmin(270, customerPosX + 1500 * deltaTime);
                
                if (canMoveScene){
                    for (int i = 0; i < 2; i++){
                        UpdateButtonUI(&sceneMoveBtns[i]);
                    }
                }
                
                BeginDrawing();
                    DrawTexture(backgrounds[gm->currentSceneIndex][0], 0, 0, WHITE);
                    if (gm->currentSceneIndex == SCENE_COUNTER){
                        DrawTextureV(currentCustomerTexture, (Vector2){customerPosX, 100}, WHITE);
                    
                        DrawRectangleRec(posMachine, (Color){255, 0, 0, 100});
                        DrawRectangleRec(cardMachine, (Color){0, 255, 0, 100});
                    }
                    DrawTexture(backgrounds[gm->currentSceneIndex][1], 0, 0, WHITE);

                    // Dialogue UI
                    DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);
                    DrawTextEx(font, dialogue[currentDialogueIndex][0], (Vector2){30, GetScreenHeight() - 180}, 50, 1, WHITE);
                    DrawTextEx(font, dialogue[currentDialogueIndex][1], (Vector2){80, GetScreenHeight() - 120}, 40, 1, WHITE);
                    // Dialogue UI End

                    // Scene Move UI Start
                    DrawRectangleRec((Rectangle){850, GetScreenHeight() - 200, 250, 200}, BROWN);
                    if (canMoveScene){
                        for (int i = 0; i < 2; i++){
                            RenderButtonUI(&sceneMoveBtns[i]);
                        }
                    }
                    
                    char str[20];
                    sprintf(str, "> %s", GetSceneName());
                    DrawTextEx(font, str, (Vector2){870, 725}, 35, 2, WHITE);
                    // Scene Move UI End

                    // Player Stats UI
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, GetScreenHeight()}, BLACK); // 
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, 120}, WHITE); // Time, Item UI
                    DrawTexture(playerNormalTexture, GetScreenWidth()-400, 0, WHITE);
                    
                    sprintf(healthText, "Health : %d", playerStatus.health);
                    sprintf(ratingText, "Store Rating : %d", playerStatus.rating);
                    
                    DrawTextEx(font, healthText, (Vector2){GetScreenWidth()-390, 10}, 25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 35, 300, 5}, RED);
                    
                    DrawTextEx(font, ratingText, (Vector2){GetScreenWidth()-390, 50},25, 2, BLACK);
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

cJSON* GetJsonData(char* fileName){
    cJSON* jsonData = GetDialogueJson(fileName);
    if (jsonData == NULL){
        exit(-1);
    }
    else{
        return jsonData;
    }
}

void LoadFontAll(Font* font){
    // 코드포인트 배열 생성 (영어, 숫자, 특수기호, 한글)
    int codepointCount = 0;
    int *codepoints = (int *)malloc(sizeof(int) * (11172 + 26 * 2 + 10 + 32)); // 한글 + 영어 + 숫자 + 특수기호
    int i = 0;

    // 영어 대문자 (A-Z)
    for (int ch = 'A'; ch <= 'Z'; ch++, i++) {
        codepoints[i] = ch;
    }

    // 영어 소문자 (a-z)
    for (int ch = 'a'; ch <= 'z'; ch++, i++) {
        codepoints[i] = ch;
    }

    // 숫자 (0-9)
    for (int ch = '0'; ch <= '9'; ch++, i++) {
        codepoints[i] = ch;
    }

    // 특수기호 (예: !"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~)
    char specialChars[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    for (int j = 0; specialChars[j] != '\0'; j++, i++) {
        codepoints[i] = specialChars[j];
    }

    // 한글 음절 (U+AC00 ~ U+D7A3)
    for (int ch = 0xAC00; ch <= 0xD7A3; ch++, i++) {
        codepoints[i] = ch;
    }

    codepointCount = i; // 전체 코드포인트 수

    // 폰트 로드 (특정 폰트 사용)
    *font = LoadFontEx("Assets/Fonts/SB Aggro M.ttf", 70, codepoints, codepointCount);
    SetTextureFilter(font->texture, TEXTURE_FILTER_BILINEAR);
}

void OnPrevSceneButtonClicked(){
    GameManager* gm = GetGameManager();

    gm->currentSceneIndex --;
    if (gm->currentSceneIndex < 0){
        gm->currentSceneIndex = 4;
    }
}

void OnNextSceneButtonClicked(){
    GameManager* gm = GetGameManager();

    gm->currentSceneIndex ++;
    if (gm->currentSceneIndex > 4){
        gm->currentSceneIndex = 0;
    }
}

char* GetSceneName(){
    GameManager* gm = GetGameManager();
    switch (gm->currentSceneIndex)
    {
    case 0:
        return "카운터";
    case 1:
        return "스낵류";
    case 2:
        return "음료류";
    case 3:
        return "라면류";
    case 4:
        return "생활용품류";
    
    default:
        return "ERROR";
        break;
    }
}

char* GetDifficultyName(){
    GameManager* gm = GetGameManager();
    switch (gm->currentDifficulty)
    {
    case DIFFICULTY_EASY:
        return "Easy";
    case DIFFICULTY_NORMAL:
        return "Normal";
    case DIFFICULTY_HARD:
        return "Hard";
    
    default:
        return "ERROR";
    }
}