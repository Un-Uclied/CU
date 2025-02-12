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

typedef enum{
    TIME_DAY, TIME_NIGHT
} DayTime;

typedef struct Globals {
    char* currentDifficulty;
    int currentSceneIndex;
    int currentCornerIndex;

    char* currentSpeaker;
    char* currentText;

    ItemStorage itemsButtonUIs[10];

    // 플레이어 현재 스탯
    int playerHealth;
    int playerRating;

    bool isInventoryOpened;
    bool isPosMachineOpened;
    bool isCardMachineOpened;

    char** currentInventory; // 연결 리스트 구현하기 귀찮;
    Texture* currentInventoryItemTextures;
    char** currentInventoryItemPrice;
    int currentInventoryLen;

    Rectangle toolTipRect;
    char* toolTipName;
    char* toolTipText;
    bool isToolTipShow;

    DayTime currentTime;
} Globals;

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


// 정적 변수로 싱글톤 객체 유지 이게 맞냐??
Globals* GetGlobalVariables() {
    static Globals instance = {0};  // 프로그램 실행 중 단 하나만 존재
    return &instance;
}

char* GetCurrentCornerName();

char* GetItemCategoryFolderPath();

void LoadFontAll(Font* font);

void StartDialogue(cJSON* jsonData, char*** dialogue, int* dialogueLen, int* currentDialogueIndex, char* dialogueFilePath){
    jsonData = GetJsonData(dialogueFilePath);
    dialogue = GetDialogueData(jsonData);
    *dialogueLen = GetDialogueLen(jsonData);
    *currentDialogueIndex = 0;
}

bool IsPopUpOpened();

// 버튼은 main함수에서 만들기에 선언을 미리 하고 구현은 아래에서 함
ButtonUI NewButton(char* objectName, Rectangle hitBox, Texture normalTexture, Texture pressedTexture, Texture hoveredTexture, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText);
TransparentButton NewTransparentButton(char* objectName, Rectangle hitBox, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText);

void OnPrevSceneButtonClicked(ButtonUI* btn);
void OnNextSceneButtonClicked(ButtonUI* btn);

void OnInventoryButtonClicked(ButtonUI* btn);

void OnItemDeleteButtonClicked(ButtonUI* btn);

void OnCustomerButtonClicked(TransparentButton* btn);

void OnCardReaderButtonClicked(TransparentButton* btn);

void OnPosMachineButtonClicked(TransparentButton* btn);

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
    globals->currentInventoryItemPrice = (char**)malloc(sizeof(char*) * INVENTORY_MAX_LEN);

    globals->toolTipRect = (Rectangle){0, 0, 200, 70};

    globals->currentTime = TIME_DAY;
 
    // Dialogue
    char customerDialoguePath[256];
    char* customerName = "Normal Customer";
    sprintf(customerDialoguePath, "Assets/Data/Customer Dialogues/%s/%s.json", customerName, globals->currentDifficulty);

    cJSON* jsonData = GetJsonData(customerDialoguePath);
    char*** dialogue = GetDialogueData(jsonData);
    int dialogueLen = 0;
    int currentDialogueIndex = 0;

    StartDialogue(jsonData, dialogue, &dialogueLen, &currentDialogueIndex, customerDialoguePath);
    // Dialogue End

    Texture playerNormalTexture = LoadTexture("Assets/Images/Player/Idle.png");
    Texture blank = LoadTexture("Assets/Images/Backgrounds/Blank.png");
    Texture normalCorner = LoadTexture("Assets/Images/Backgrounds/Corner.jpg");
    Texture drinksCorner = LoadTexture("Assets/Images/Backgrounds/Drinks Corner.jpg");
    Texture backgrounds[5][2] = {
        {LoadTexture("Assets/Images/Backgrounds/CounterBack.png"), LoadTexture("Assets/Images/Backgrounds/CounterFore.png")},
        {normalCorner, blank},
        {drinksCorner, blank},
        {normalCorner, blank},
        {normalCorner, blank},
    };

    Texture vignetteEffect = LoadTexture("Assets/Images/UI Effects/Vignette.png");

    Texture currentCustomerTexture = LoadTexture("Assets/Images/Customers/Normal Customer/Idle.png");
    float customerPosX = -currentCustomerTexture.width;

    // Scene Move Start
    bool canMoveCorner = false;
    ButtonUI sceneMoveBtns[2] = {
        NewButton("", (Rectangle){850, GetScreenHeight() - 120, 100, 100},
            LoadTexture("Assets/Images/UI/Go Left.png"),
            LoadTexture("Assets/Images/UI/Go Left Pressed.png"),
            LoadTexture("Assets/Images/UI/Go Left Hovered.png"),
            OnPrevSceneButtonClicked,
            false, "", ""
        ),

        NewButton("", (Rectangle){1000, GetScreenHeight() - 120, 100, 100},
            LoadTexture("Assets/Images/UI/Go Right.png"),
            LoadTexture("Assets/Images/UI/Go Right Pressed.png"),
            LoadTexture("Assets/Images/UI/Go Right Hovered.png"),
            OnNextSceneButtonClicked,
            false, "", ""
        )
    };
    // Scene Move End

    // 계산 Start
    TransparentButton customerButton = NewTransparentButton("customer", (Rectangle) {380, 180, 420, 420}, OnCustomerButtonClicked, true, "손님", "물건 결제 시작하기");
    TransparentButton posMachineButton = NewTransparentButton("posMachine", (Rectangle) {820, 280, 280, 420}, OnPosMachineButtonClicked, true, "포스기", "아직 결제중이 아니다");
    TransparentButton cardMachineButton = NewTransparentButton("cardMachine", (Rectangle) {550, 600, 200, 100}, OnCardReaderButtonClicked, true, "카드 리더기", "아직 결제중이 아니다");
    // 계산 End

    // Inventory Start
    ButtonUI inventoryButtonUI  = NewButton("", // 이름 필요 없음;
        (Rectangle) {GetScreenWidth()-380, GetScreenHeight() -120, 100, 100},
        LoadTexture("Assets/Images/UI/Inventory.png"),
        LoadTexture("Assets/Images/UI/Inventory Pressed.png"),
        LoadTexture("Assets/Images/UI/Inventory Hovered.png"),
        OnInventoryButtonClicked,
        false, "", ""
    );
    
    Rectangle inventoryBG = (Rectangle){20, 20, 1060, GetScreenHeight() - 240};

    Texture inventoryTexture[5];

    ButtonUI inventoryDeleteButtons[INVENTORY_MAX_LEN];
    for (int i = 0; i < INVENTORY_MAX_LEN; i++){
        char* name = (char*)MemAlloc(sizeof(char) * 10);
        sprintf(name, "%d", i);
        inventoryDeleteButtons[i] = NewButton(
            name,
            (Rectangle) {950, i * 120 + 90, 100, 100},
            LoadTexture("Assets/Images/UI/Close Inventory.png"),
            LoadTexture("Assets/Images/UI/Close Inventory Pressed.png"),
            LoadTexture("Assets/Images/UI/Close Inventory Hovered.png"), 
            OnItemDeleteButtonClicked,
            false, "", ""
        );
    }
    // Inventory End

    // Time Start
    Texture clock = LoadTexture("Assets/Images/UI/Clock.png");
    
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
                globals->isToolTipShow = false;
                // Next Dialogue Key
                if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_SPACE) && IsPopUpOpened() == false){
                    int prevIndex = currentDialogueIndex;
                    currentDialogueIndex = fmin(dialogueLen - 1, currentDialogueIndex + 1);
                    if (prevIndex == currentDialogueIndex){
                        canMoveCorner = true;
                    }
                    else{
                        globals->currentSpeaker = dialogue[currentDialogueIndex][0];
                        globals->currentText = dialogue[currentDialogueIndex][1];
                    }
                }
                
                // 고객은 항상 이동하려는중 새 고객올시 0으로 초기화 하셈 ㅇㅇ
                customerPosX = fmin(270, customerPosX + 1500 * deltaTime);
                
                // 씬을 움직일수 있을때 버튼 업데이트
                if (canMoveCorner){
                    if (globals->currentCornerIndex == CORNER_COUNTER){
                        UpdateTransparentButton(&customerButton);
                        UpdateTransparentButton(&posMachineButton);
                        UpdateTransparentButton(&cardMachineButton);
                    }
                    

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

                globals->toolTipRect.x = GetMousePosition().x;
                globals->toolTipRect.y = GetMousePosition().y + 25;
                
                BeginDrawing();
                    DrawTexture(backgrounds[globals->currentCornerIndex][0], 0, 0, WHITE);
                    if (globals->currentCornerIndex == CORNER_COUNTER){
                        DrawTextureV(currentCustomerTexture, (Vector2){customerPosX, 100}, WHITE);
                    }
                    else{
                        for (int i = 0; i < 10; i++){
                            RenderItemStorage(&globals->itemsButtonUIs[i]);
                        }
                    }
                    // 카운터 책상 렌더에도 쓸수 있고 손 렌더할수도
                    DrawTexture(backgrounds[globals->currentCornerIndex][1], 0, 0, WHITE);
                    
                    // Vignette 이펙트
                    DrawTexture(vignetteEffect, 0, 0, (Color){0, 0, 0, 200});
                    
                    // Dialogue UI
                    DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);
                    DrawTextEx(font, globals->currentSpeaker, (Vector2){30, GetScreenHeight() - 180}, 50, 1, WHITE);
                    DrawTextEx(font, globals->currentText, (Vector2){80, GetScreenHeight() - 120}, 40, 1, WHITE);
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

                    // 현재 시간 보여줌
                    if (globals->currentCornerIndex == CORNER_COUNTER){
                        DrawTexture(clock, 40, 600, WHITE);
                    }

                    // Scene Move UI Start
                    DrawRectangleRec((Rectangle){850, GetScreenHeight() - 200, 250, 200}, BROWN);
                    if (canMoveCorner){
                        for (int i = 0; i < 2; i++){
                            RenderButtonUI(&sceneMoveBtns[i]);
                        }
                        RenderButtonUI(&inventoryButtonUI);
                    }
                    
                    char str[20]; // 문자열 쓰기 해야되서 배열로;
                    sprintf(str, "> %s", GetCurrentCornerName());
                    DrawTextEx(font, str, (Vector2){870, 725}, 35, 2, WHITE);
                    // Scene Move UI End

                    // Inventory UI Start
                    if (globals->isInventoryOpened){
                        DrawRectangleRec(inventoryBG, BLUE);
                        DrawRectangleRec((Rectangle){20, 20, 1060, 60}, BLACK); // 나중에 UI 그래픽으로 바꿔야함;
                        char temp[50];
                        sprintf(temp, "인벤토리 %d / 5", globals->currentInventoryLen);
                        DrawTextEx(font, temp, (Vector2){35, 35}, 40, 2, WHITE);
                        
                        for (int i = 0; i < INVENTORY_MAX_LEN; i++){
                            DrawRectangleGradientV(20, i * 120 + 100, 1060, 100, BLUE, (Color){ 0, 101, 211, 255 } );
                        }
                        
                        // 인벤토리에 있는 아이템 정보 렌더
                        for (int i = 0; i < globals->currentInventoryLen; i++){
                            DrawTextEx(font, globals->currentInventory[i], (Vector2){45, i * 120 + 100}, 40, 2, WHITE);
                            DrawTextEx(font, globals->currentInventoryItemPrice[i], (Vector2){505, i * 120 + 100}, 40, 2, WHITE);
                            DrawTextureV(globals->currentInventoryItemTextures[i], (Vector2){820, i * 120 + 90}, WHITE);
                            RenderButtonUI(&inventoryDeleteButtons[i]); // 아이템 지우기 버튼
                        }
                    }
                    // Inventory UI End
                    
                    // ToolTip Draw Start
                    if (globals->isToolTipShow){
                        DrawRectangleRec(globals->toolTipRect, RAYWHITE);
                        DrawTextEx(font, globals->toolTipName, (Vector2){globals->toolTipRect.x + 5, globals->toolTipRect.y + 5}, 20, 2, BLACK);
                        DrawTextEx(font, globals->toolTipText, (Vector2){globals->toolTipRect.x + 5, globals->toolTipRect.y + 30}, 15, 2, BLACK);
                    }
                    // ToolTip Draw End

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

// UI EVENTS START =========================================================================================

// 먼저 선언
void MakeItemStorageButton(ItemStorage* itemBtns);

// 전의 코너로 돌아가기 버튼 구현
void OnPrevSceneButtonClicked(ButtonUI* btn){
    Globals* global = GetGlobalVariables();

    global->currentCornerIndex --;
    if (global->currentCornerIndex < 0){
        global->currentCornerIndex = 4;
    }

    // 카운터에는 물건들을 두지 않음.
    if (global->currentCornerIndex != CORNER_COUNTER){
        MakeItemStorageButton(global->itemsButtonUIs);
    }
}

// 다음 코너로 가기 버튼 구현
void OnNextSceneButtonClicked(ButtonUI* btn){
    Globals* global = GetGlobalVariables();

    global->currentCornerIndex ++;
    if (global->currentCornerIndex > 4){
        global->currentCornerIndex = 0;
    }

    if (global->currentCornerIndex != CORNER_COUNTER){
        MakeItemStorageButton(global->itemsButtonUIs);
    }
}

// 인벤토리 버튼 구현
void OnInventoryButtonClicked(ButtonUI* btn){
    Globals* globals = GetGlobalVariables();
    if (globals->isInventoryOpened){
        globals->isInventoryOpened = false;
    }
    else{
        globals->isInventoryOpened = true;
    }
}

// 아이템 얻는 버튼 왼쪽 클릭 구현
void OnItemStorageItemLeftClicked(ItemStorage* itemStorage){
    Globals* globals = GetGlobalVariables();
    if (globals->currentInventoryLen == INVENTORY_MAX_LEN){
        return;
    }

    // 인벤토리에 아이템 저장
    globals->currentInventory[globals->currentInventoryLen] = itemStorage->itemName;
    globals->currentInventoryItemTextures[globals->currentInventoryLen] = itemStorage->texture;
    
    char price[10]; // 숫자를 문자열로 바꿔서 저장 (매 프레임마다 계속 정수를 문자열로 바꾸는것보단 성능 좋겠제)
    sprintf(price, "%d\\", itemStorage->price);
    globals->currentInventoryItemPrice[globals->currentInventoryLen] = strdup(price);
    globals->currentInventoryLen ++;

    char text[50];
    sprintf(text, "(%s을(를) 바구니에 담았다.)", itemStorage->itemName);
    globals->currentSpeaker = "당신";
    globals->currentText = strdup(text);
}

// 아이템 얻는 버튼 오른쪽 클릭 구현
void OnItemStorageItemRightClicked(ItemStorage* itemStorage){

}

// 아이템 지우는 버튼 구현
void OnItemDeleteButtonClicked(ButtonUI* btn){
    Globals * globals = GetGlobalVariables();
    globals->currentInventory[atoi(btn->objectName)] = NULL;
    int prevSize = globals->currentInventoryLen;
    globals->currentInventoryLen --;

    char** temp = (char**)malloc(sizeof(char*) * globals->currentInventoryLen);
    Texture* textureTemp = (Texture*)malloc(sizeof(Texture) * globals->currentInventoryLen);
    char** priceTemp = (char**)malloc(sizeof(char*) * globals->currentInventoryLen);

    int j = 0;
    for (int i = 0; i < prevSize; i++){
        if (globals->currentInventory[i] != NULL){
            temp[j] = globals->currentInventory[i];
            textureTemp[j] = globals->currentInventoryItemTextures[i];
            priceTemp[j] = globals->currentInventoryItemPrice[i];
            j++;   
        }
    }

    free(globals->currentInventory);
    free(globals->currentInventoryItemTextures);
    free(globals->currentInventoryItemPrice);

    globals->currentInventory = temp;
    globals->currentInventoryItemTextures = textureTemp;
    globals->currentInventoryItemPrice = priceTemp;
}

// 손님 클릭
void OnCustomerButtonClicked(TransparentButton* btn){
    Globals* globals = GetGlobalVariables();
    if (globals->currentInventoryLen == 0){
        globals->currentSpeaker = "당신";
        globals->currentText = "(아직 손님이 요청한\n물건들을 갖고 오지 않은것같다.)";
    }
}

// 카드 리더기 버튼 구현
void OnCardReaderButtonClicked(TransparentButton* btn){
    printf("Card Reader\n");
}

// 포스기 버튼 구현
void OnPosMachineButtonClicked(TransparentButton* btn){
    printf("Pos Machine\n");
}

void OnButtonUIHovered(ButtonUI* buttonUI){
    Globals* globals = GetGlobalVariables();
}

void OnTransparentButtonUIHovered(TransparentButton* buttonUI){
    Globals* globals = GetGlobalVariables();
    globals->isToolTipShow = true;
    globals->toolTipName = buttonUI->toolTip.toolTipName;
    globals->toolTipText = buttonUI->toolTip.toolTipText;
}

void OnItemStorageUIHovered(ItemStorage* itemStorageUI){
    Globals* globals = GetGlobalVariables();
    globals->isToolTipShow = true;
    globals->toolTipName = itemStorageUI->itemName;

    char priceText[10];
    sprintf(priceText, "%d\\", itemStorageUI->price);
    globals->toolTipText = strdup(priceText);
}


// 새 버튼 만들고 반환
ButtonUI NewButton(char* objectName, Rectangle hitBox, Texture normalTexture, Texture pressedTexture, Texture hoveredTexture, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText){
    ButtonUI newButton = (ButtonUI){
        objectName, 
        hitBox,
        normalTexture,
        pressedTexture,
        hoveredTexture,
        NULL,
        clickedEvent, OnButtonUIHovered,
        (ButtonTooltip){
            showTooltip, toolTipName, toolTipText
        }
    };
    newButton.currentTexture = &newButton.normalTexture;
    return newButton;
}

// 새 투명 버튼 만들고 반환
TransparentButton NewTransparentButton(char* objectName, Rectangle hitBox, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText){
    TransparentButton newButton = (TransparentButton){
        objectName, hitBox, 
        clickedEvent, OnTransparentButtonUIHovered,
        (ButtonTooltip){
            showTooltip, toolTipName, toolTipText
        }
    };
    return newButton;
}

// 아이템 버튼 체인지
void MakeItemStorageButton(ItemStorage* itemBtns){
    cJSON* itemsData = GetJsonData("Assets/Data/Items.json");
    char* itemBar = GetItemCategoryFolderPath();
    cJSON* currentBar = cJSON_GetObjectItem(itemsData, itemBar);

    char itemPriceObjectName[50];
    sprintf(itemPriceObjectName, "%s Price", itemBar);
    cJSON* itemPriceObject = cJSON_GetObjectItem(itemsData, itemPriceObjectName);

    for (int i = 0; i < 10; i++){
        char temp[50];
        sprintf(temp, "Assets/Images/Items/%s/%d.png", itemBar, i);
        char *itemName = cJSON_GetArrayItem(currentBar, i)->valuestring;
        int itemPrice = cJSON_GetObjectItem(itemPriceObject, itemName)->valueint;

        printf("%d\n", itemPrice);

        itemBtns[i] = (ItemStorage){
            (Rectangle){itemUiPos[i][0], itemUiPos[i][1], 100, 100},
            LoadTexture(temp),
            itemName,
            itemPrice,
            OnItemStorageItemLeftClicked,
            OnItemStorageItemRightClicked,
            OnItemStorageUIHovered
        };
    }
}