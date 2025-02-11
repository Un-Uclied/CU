#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "Externals/dialogue.c"
#include "Externals/button.c"

#define CORNER_COUNTER 0
#define CORNER_SNACKS 1
#define CORNER_DRINKS 2
#define CORNER_RAMENS 3
#define CORNER_TOOLS 4

#define SCENE_MAIN_TITLE 0
#define SCENE_MAIN_GAME 1
#define SCENE_SETTINGS 2

#define DIFFICULTY_EASY "Easy"
#define DIFFICULTY_NORMAL "Normal"
#define DIFFICULTY_HARD "Hard"

#define INVENTORY_MAX_LEN 5

int itemUiPos[10][2] = {
    {200, 200},
    {350, 200},
    {500, 200},
    {640, 200},
    {800, 200},

    {200, 400},
    {350, 400},
    {500, 400},
    {640, 400},
    {800, 400},
};

// 싱글톤 구조체
typedef struct {
    char* currentDifficulty;
    int currentSceneIndex;
    int currentCornerIndex;

    ItemStorage itemsButtonUIs[10];

    // 플레이어 현재 스탯
    int playerHealth;
    int playerRating;

    bool isInventoryOpened;
    bool isPosMachineOpened;
    bool isCardMachineOpened;

    char** currentInventory; // 연결 리스트 구현하기 귀찮;
    Texture* currentInventoryItemTextures;
    int currentInventoryLen;
} Globals;

// 정적 변수로 싱글톤 객체 유지 이게 맞냐??
Globals* GetGlobalVariables() {
    static Globals instance = {0};  // 프로그램 실행 중 단 하나만 존재
    return &instance;
}

cJSON* GetJsonData(char* fileName);

char* GetCurrentCornerName();

char* GetItemCategoryFolderPath();

void OnPrevSceneButtonClicked(ButtonUI* btn);

void OnNextSceneButtonClicked(ButtonUI* btn);

void OnInventoryButtonClicked(ButtonUI* btn);

void OnItemStorageItemLeftClicked(ItemStorage* itemStorage);

void OnItemStorageItemRightClicked(ItemStorage* itemStorage);

void OnItemDeleteButtonClicked(ButtonUI* btn);

void LoadFontAll(Font* font);

void StartDialogue(cJSON* jsonData, char*** dialogue, int* dialogueLen, int* currentDialogueIndex, char* dialogueFilePath){
    jsonData = GetJsonData(dialogueFilePath);
    dialogue = GetDialogueData(jsonData);
    *dialogueLen = GetDialogueLen(jsonData);
    *currentDialogueIndex = 0;
}

void LoadItemGraphics(ItemStorage* itemBtns){
    cJSON* itemsData = GetJsonData("Assets/Data/Items.json");
    char* itemBar = GetItemCategoryFolderPath();
    cJSON* currentBar = cJSON_GetObjectItem(itemsData, itemBar);
    for (int i = 0; i < 10; i++){
        char temp[50];
        sprintf(temp, "Assets/Images/Items/%s/%d.png", itemBar, i);
        char *itemName = cJSON_GetArrayItem(currentBar, i)->valuestring;

        itemBtns[i] = (ItemStorage){
            (Rectangle){itemUiPos[i][0], itemUiPos[i][1], 70, 70},
            LoadTexture(temp),
            itemName,
            OnItemStorageItemLeftClicked,
            OnItemStorageItemRightClicked
        };
    }
}

bool IsPopUpOpened();

int main(void){
    InitWindow(1500, 900, "CU");
    //SetTargetFPS(500);

    // 폰트 (한글도 되긴되게 했는데 FPS 2000대로 뚝떨어짐;)
    Font font;
    LoadFontAll(&font);
    
    // 글로벌 변수 초기화!!
    Globals * globals = GetGlobalVariables();
    globals->currentSceneIndex = SCENE_MAIN_TITLE;
    globals->currentDifficulty = DIFFICULTY_NORMAL;
    globals->currentCornerIndex = CORNER_COUNTER;

    globals->isInventoryOpened = false;
    globals->isCardMachineOpened = false;
    globals->isPosMachineOpened = false;

    globals->currentInventory = (char**)malloc(sizeof(char*) * INVENTORY_MAX_LEN);
    globals->currentInventoryItemTextures = (Texture*)malloc(sizeof(Texture) * INVENTORY_MAX_LEN);

    // Dialogue
    char customerDialoguePath[256];
    char* customerName = "Normal Customer";
    sprintf(customerDialoguePath, "Assets/Data/Customers/%s/%s.json", customerName, globals->currentDifficulty);

    cJSON* jsonData = GetJsonData(customerDialoguePath);
    char*** dialogue = GetDialogueData(jsonData);
    int dialogueLen = 0;
    int currentDialogueIndex = 0;

    StartDialogue(jsonData, dialogue, &dialogueLen, &currentDialogueIndex, customerDialoguePath);
    // Dialogue End

    Texture playerNormalTexture = LoadTexture("Assets/Images/Player/Idle.png");
    Texture blank = LoadTexture("Assets/Images/Backgrounds/Blank.png");
    Texture backgrounds[5][2] = {
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), LoadTexture("Assets/Images/Backgrounds/CounterFore.png")},
        {LoadTexture("Assets/Images/Backgrounds/Snack Bar.jpg"), blank},
        {LoadTexture("Assets/Images/Backgrounds/Drink Bar.jpg"), blank},
        {LoadTexture("Assets/Images/Backgrounds/Ramen Bar.jpg"), blank},
        {LoadTexture("Assets/Images/Backgrounds/Tools Bar.jpg"), blank},
    };

    Texture vignetteEffect = LoadTexture("Assets/Images/UI Effects/Vignette.png");

    Texture currentCustomerTexture = LoadTexture("Assets/Images/Customers/Normal Customer/Idle.png");
    float customerPosX = -currentCustomerTexture.width;

    // Scene Move Start
    bool canMoveScene = false;
    ButtonUI nextSceneButton = (ButtonUI){
        "", // 이름 필요 없음;
        (Rectangle){1000, GetScreenHeight() - 120, 100, 100},
        LoadTexture("Assets/Images/UI/Go Right.png"),
        LoadTexture("Assets/Images/UI/Go Right Pressed.png"),
        LoadTexture("Assets/Images/UI/Go Right Hovered.png"),
        NULL,
        OnNextSceneButtonClicked
    };
    nextSceneButton.currentTexture = &nextSceneButton.normalTexture;

    ButtonUI prevSceneButton = (ButtonUI){
        "", // 이름 필요 없음;
        (Rectangle){850, GetScreenHeight() - 120, 100, 100},
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

    // Inventory Start
    ButtonUI inventoryButtonUI = (ButtonUI){
        "", // 이름 필요 없음;
        (Rectangle) {GetScreenWidth()-380, GetScreenHeight() -120, 100, 100},
        LoadTexture("Assets/Images/UI/Inventory.png"),
        LoadTexture("Assets/Images/UI/Inventory Pressed.png"),
        LoadTexture("Assets/Images/UI/Inventory Hovered.png"),
        NULL, 
        OnInventoryButtonClicked
    };

    Rectangle inventoryBG = (Rectangle){20, 20, 1060, GetScreenHeight() - 240};

    Texture inventoryTexture[5];

    ButtonUI inventoryDeleteButtons[INVENTORY_MAX_LEN];
    for (int i = 0; i < INVENTORY_MAX_LEN; i++){
        char* name = (char*)MemAlloc(sizeof(char) * 10);
        sprintf(name, "%d", i);
        inventoryDeleteButtons[i] = (ButtonUI){
            name,
            (Rectangle) {950, i * 120 + 90, 100, 100},
            LoadTexture("Assets/Images/UI/Close Inventory.png"),
            LoadTexture("Assets/Images/UI/Close Inventory Pressed.png"),
            LoadTexture("Assets/Images/UI/Close Inventory Hovered.png"),
            NULL, 
            OnItemDeleteButtonClicked
        };
    }
    // Inventory End
    
    while (!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        switch (globals->currentSceneIndex)
        {
            case SCENE_MAIN_TITLE:
                if (IsKeyPressed(KEY_SPACE)){
                    globals->currentSceneIndex = SCENE_MAIN_GAME;
                }

                BeginDrawing();

                    DrawText("CU SIMULATOR", 450, 200, 80, WHITE);
                    DrawText("[SPACE] to start", 560, 560, 40, WHITE);
                    DrawFPS(1400, 10);

                EndDrawing();
                break;

            case SCENE_MAIN_GAME:
                // Next Dialogue Key
                if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_SPACE) && IsPopUpOpened() == false){
                    int prevIndex = currentDialogueIndex;
                    currentDialogueIndex = fmin(dialogueLen - 1, currentDialogueIndex + 1);
                    if (prevIndex == currentDialogueIndex){
                        canMoveScene = true;
                    }
                }
                
                // 고객은 항상 이동하려는중 새 고객올시 0으로 초기화 하셈 ㅇㅇ
                customerPosX = fmin(270, customerPosX + 1500 * deltaTime);
                
                // 씬을 움직일수 있을때 버튼 업데이트
                if (canMoveScene ){
                    if (IsPopUpOpened() == false){
                        for (int i = 0; i < 2; i++){
                            UpdateButtonUI(&sceneMoveBtns[i]);
                        }
                    }
                    if (globals->isCardMachineOpened == false && globals->isPosMachineOpened == false){
                        UpdateButtonUI(&inventoryButtonUI); // 토글 방식이라 인벤ㅌㅌ
                    }
                }

                // 카운터가 아님
                if (globals->currentCornerIndex != CORNER_COUNTER && IsPopUpOpened() == false){
                    for (int i = 0; i < 10; i++){
                        UpdateItemStorage(&globals->itemsButtonUIs[i]);
                    }
                }

                if (globals->isInventoryOpened){
                    for (int i = 0; i < globals->currentInventoryLen; i++){
                        UpdateButtonUI(&inventoryDeleteButtons[i]);
                    }
                }
                
                BeginDrawing();
                    DrawTexture(backgrounds[globals->currentCornerIndex][0], 0, 0, WHITE);
                    if (globals->currentCornerIndex == CORNER_COUNTER){
                        DrawTextureV(currentCustomerTexture, (Vector2){customerPosX, 100}, WHITE);
                    
                        // DrawRectangleRec(posMachine, (Color){255, 0, 0, 100});
                        // DrawRectangleRec(cardMachine, (Color){0, 255, 0, 100});
                    }
                    else{
                        for (int i = 0; i < 10; i++){
                            RenderItemStorage(&globals->itemsButtonUIs[i]);
                        }
                    }
                    // 카운터 책상 렌더에도 쓸수 있고 손 렌더할수도
                    DrawTexture(backgrounds[globals->currentCornerIndex][1], 0, 0, WHITE);
                    
                    // Dialogue UI
                    DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);
                    DrawTextEx(font, dialogue[currentDialogueIndex][0], (Vector2){30, GetScreenHeight() - 180}, 50, 1, WHITE);
                    DrawTextEx(font, dialogue[currentDialogueIndex][1], (Vector2){80, GetScreenHeight() - 120}, 40, 1, WHITE);
                    // Dialogue UI End

                    // Player Stats UI
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, GetScreenHeight()}, BLACK); // 
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, 120}, WHITE); // Time, Item UI
                    DrawTexture(playerNormalTexture, GetScreenWidth()-400, 0, WHITE);
                    
                    char healthText[50];
                    char ratingText[50];
                    sprintf(healthText, "Health : %d", globals->playerHealth); // sprintf에는 char*말고 char[]써야됨
                    sprintf(ratingText, "Store Rating : %d", globals->playerRating); // char*는 그냥 쌩 문자열 데이터라고 생각하면 될듯
                    
                    DrawTextEx(font, healthText, (Vector2){GetScreenWidth()-390, 10}, 25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 35, 300, 5}, RED);
                    
                    DrawTextEx(font, ratingText, (Vector2){GetScreenWidth()-390, 50},25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 80, 300, 5}, BLUE);
                    // Player Stats UI End

                    // Scene Move UI Start
                    DrawRectangleRec((Rectangle){850, GetScreenHeight() - 200, 250, 200}, BROWN);
                    if (canMoveScene){
                        for (int i = 0; i < 2; i++){
                            RenderButtonUI(&sceneMoveBtns[i]);
                        }
                        RenderButtonUI(&inventoryButtonUI);
                    }
                    
                    char str[20]; // 문자열 쓰기 해야되서 배열로;
                    sprintf(str, "> %s", GetCurrentCornerName());
                    DrawTextEx(font, str, (Vector2){870, 725}, 35, 2, WHITE);
                    // Scene Move UI End

                    if (globals->isInventoryOpened){
                        DrawRectangleRec(inventoryBG, BLUE);
                        DrawRectangleRec((Rectangle){20, 20, 1060, 60}, BLACK); // 나중에 UI 그래픽으로 바꿔야함;
                        char temp[50];
                        sprintf(temp, "인벤토리 %d / 5", globals->currentInventoryLen);
                        DrawTextEx(font, temp, (Vector2){35, 35}, 40, 2, WHITE);
                        
                        for (int i = 0; i < INVENTORY_MAX_LEN; i++){
                            DrawRectangleGradientV(20, i * 120 + 100, 1060, 100, BLUE, (Color){ 0, 101, 211, 255 } );
                        }

                        for (int i = 0; i < globals->currentInventoryLen; i++){
                            DrawTextEx(font, globals->currentInventory[i], (Vector2){45, i * 120 + 100}, 40, 2, WHITE);
                            DrawTextureV(globals->currentInventoryItemTextures[i], (Vector2){820, i * 120 + 90}, WHITE);
                            RenderButtonUI(&inventoryDeleteButtons[i]);
                        }
                    }

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

bool IsPopUpOpened(){
    Globals* globals = GetGlobalVariables();
    if (globals->isInventoryOpened){
        return true;
    }
    else if (globals->isCardMachineOpened){
        return true;
    }
    else if (globals->isPosMachineOpened){
        return true;
    }

    return false;
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
    
    // 메모리 프리하게 해줄게
    free(codepoints);
}

void OnPrevSceneButtonClicked(ButtonUI* btn){
    Globals* global = GetGlobalVariables();

    global->currentCornerIndex --;
    if (global->currentCornerIndex < 0){
        global->currentCornerIndex = 4;
    }

    // 카운터에는 물건들을 두지 않음.
    if (global->currentCornerIndex != CORNER_COUNTER){
        LoadItemGraphics(global->itemsButtonUIs);
    }
}

void OnNextSceneButtonClicked(ButtonUI* btn){
    Globals* global = GetGlobalVariables();

    global->currentCornerIndex ++;
    if (global->currentCornerIndex > 4){
        global->currentCornerIndex = 0;
    }

    if (global->currentCornerIndex != CORNER_COUNTER){
        LoadItemGraphics(global->itemsButtonUIs);
    }
}

void OnInventoryButtonClicked(ButtonUI* btn){
    Globals* globals = GetGlobalVariables();
    if (globals->isInventoryOpened){
        globals->isInventoryOpened = false;
    }
    else{
        globals->isInventoryOpened = true;
    }
}

// UI 표시 내용
char* GetCurrentCornerName(){
    Globals* global = GetGlobalVariables();
    switch (global->currentCornerIndex)
    {
    case CORNER_COUNTER:
        return "카운터";
    case CORNER_SNACKS:
        return "스낵류";
    case CORNER_DRINKS:
        return "음료류";
    case CORNER_RAMENS:
        return "라면류";
    case CORNER_TOOLS:
        return "생활용품류";
    
    default:
        return "ERROR";
        break;
    }
}

// 파일 탐색용
char* GetItemCategoryFolderPath(){
    Globals* global = GetGlobalVariables();
    switch (global->currentCornerIndex)
    {
    case CORNER_SNACKS:
        return "Snacks";
    case CORNER_DRINKS:
        return "Drinks";
    case CORNER_RAMENS:
        return "Ramens";
    case CORNER_TOOLS:
        return "Tools";
    
    default:
        return "ERROR"; // 카운터에서 이거 부르면 에러남;
        break;
    }
}


void OnItemStorageItemLeftClicked(ItemStorage* itemStorage){
    Globals* globals = GetGlobalVariables();
    if (globals->currentInventoryLen == INVENTORY_MAX_LEN){
        return;
    }

    globals->currentInventory[globals->currentInventoryLen] = itemStorage->itemName;
    globals->currentInventoryItemTextures[globals->currentInventoryLen] = itemStorage->texture;
    globals->currentInventoryLen ++;
}

void OnItemStorageItemRightClicked(ItemStorage* itemStorage){

}

void OnItemDeleteButtonClicked(ButtonUI* btn){
    Globals * globals = GetGlobalVariables();
    globals->currentInventory[atoi(btn->objectName)] = NULL;
    globals->currentInventoryItemTextures[atoi(btn->objectName)].id = 0; // Texture **로 하기 귀찮아서 걍 id가 0인건 지워야할거로 생각
    int prevSize = globals->currentInventoryLen;
    globals->currentInventoryLen --;

    char** temp = (char**)malloc(sizeof(char*) * globals->currentInventoryLen);
    int j = 0;
    for (int i = 0; i < prevSize; i++){
        if (globals->currentInventory[i] != NULL){
            temp[j] = globals->currentInventory[i];
            j++;   
        }
    }

    Texture* textureTemp = (Texture*)malloc(sizeof(Texture) * globals->currentInventoryLen);
    int l = 0;
    for (int i = 0; i < prevSize; i++){
        if (globals->currentInventoryItemTextures[i].id != 0){
            textureTemp[l] = globals->currentInventoryItemTextures[i];
            l++;   
        }
    }

    free(globals->currentInventory);
    free(globals->currentInventoryItemTextures);
    globals->currentInventory = temp;
    globals->currentInventoryItemTextures = textureTemp;
}