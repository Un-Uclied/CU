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

#define ITEM_JSON_DATA_PATH "Assets/Data/Items.json"
#define STAT_CHANGE_AMOUNT_JSON_DATA_PATH "Assets/Data/Stat Changes.json"

#define PAYMENT_CARD "Card"
#define PAYMENT_CASH "Cash"

typedef enum{
    TIME_DAY, TIME_NIGHT
} DayTime;

typedef struct Globals {
    char* currentDifficulty;
    int currentSceneIndex;
    int currentCornerIndex;
    bool canMoveCorner;

    DialogueUI dialogueUI;
    
    char* customerName;
    char customerDialoguePath[256];
    
    bool isCustomerAngry;
    bool isCustomerHappy;

    ItemStorage itemsButtonUIs[10];
    char** neededItems;
    int neededItemLength;

    // 플레이어 현재 스탯
    int playerHealth;
    int playerRating;

    bool isRingup;
    bool isInventoryOpened;
    bool isPosMachineOpened;
    bool isCardMachineOpened;

    //인벤토리
    char* currentInventory[INVENTORY_MAX_LEN]; // 연결 리스트 구현하기 귀찮;
    Texture currentInventoryItemTextures[INVENTORY_MAX_LEN];
    int currentInventoryItemPrice[INVENTORY_MAX_LEN];
    int currentInventoryLen;

    //툴팁
    Rectangle toolTipRect;
    char* toolTipName;
    char* toolTipText;
    bool isToolTipShow;

    DayTime currentTime;

    int customerSpeed;
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

void InitGlobalVariables(){
    Globals * globals = GetGlobalVariables();
    globals->currentSceneIndex = SCENE_MAIN_TITLE;
    globals->currentDifficulty = DIFFICULTY_NORMAL;
    globals->currentCornerIndex = CORNER_COUNTER;
    globals->canMoveCorner = false;

    globals->dialogueUI = (DialogueUI){ 0 };

    globals->isInventoryOpened = false;
    globals->isCardMachineOpened = false;
    globals->isPosMachineOpened = false;

    for (int i = 0; i < INVENTORY_MAX_LEN; i++){
        globals->currentInventory[i] = "";
        globals->currentInventoryItemTextures[i] = (Texture){0};
        globals->currentInventoryItemPrice[i] = 0;
    }
    globals->currentInventoryLen = 0;
    
    globals->neededItems = (char**)malloc(sizeof(char*) * INVENTORY_MAX_LEN);
    globals->toolTipRect = (Rectangle){0, 0, 200, 70};

    globals->currentTime = TIME_DAY;

    globals->playerHealth = 100;
    globals->playerRating = 100;

    globals->customerSpeed = 1500;
}

char* GetCurrentCornerName();

char* GetItemCategoryFolderPath();

void LoadFontAll(Font* font);

bool IsPopUpOpened();

// 버튼은 main함수에서 만들기에 선언을 미리 하고 구현은 아래에서 함
ButtonUI NewButton(const char* objectName, Rectangle hitBox, Texture normalTexture, Texture pressedTexture, Texture hoveredTexture, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText);
TransparentButton NewTransparentButton(const char* objectName, Rectangle hitBox, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText);

// CORNER MOVE
void OnPrevSceneButtonClicked(ButtonUI* btn);
void OnNextSceneButtonClicked(ButtonUI* btn);
// CORNER MOVE

// INVENTORY
void OnInventoryButtonClicked(ButtonUI* btn);
void OnItemDeleteButtonClicked(ButtonUI* btn);
// INVENTORY

// COUNTER CORNER
void OnCustomerButtonClicked(TransparentButton* btn);
void OnCardReaderButtonClicked(TransparentButton* btn);
void OnPosMachineButtonClicked(TransparentButton* btn);
// COUNTER CORNER

void OnDialogueNextKeyPressed(DialogueUI* dialogueUI){
    if (IsPopUpOpened()) return;
    Globals* globals = GetGlobalVariables();
    int prevIndex = dialogueUI->currentDialogueIndex;
    dialogueUI->currentDialogueIndex = fmin(dialogueUI->dialogueLen - 1, dialogueUI->currentDialogueIndex + 1);
    if (prevIndex == dialogueUI->currentDialogueIndex){
        if (globals->isRingup == false || globals->isCustomerAngry == false || globals->isCustomerHappy == false){
            globals->canMoveCorner = true;
        }
                        
        // 손님이 화나서 한말 끝나면 새 고객이 들어옴.
        if (globals->isCustomerAngry == true){
            globals->customerSpeed = -1500;
            globals->canMoveCorner = false;
        }
        if (globals->isCustomerHappy == true){
            globals->isRingup = true;
            globals->canMoveCorner = false;
        }
        dialogueUI->isDialogueEnd = true;
    }
    else{
        dialogueUI->currentSpeaker = dialogueUI->dialogue[dialogueUI->currentDialogueIndex][0];
        dialogueUI->currentText = dialogueUI->dialogue[dialogueUI->currentDialogueIndex][1];
    }
}

void NewCustomerIn(char* customerName, Texture* currentCustomerTexture, Texture* customerPaymentTexture){
    Globals* globals = GetGlobalVariables();

    globals->isCustomerAngry = false;
    globals->isCustomerHappy = false;

    globals->customerName = customerName;
    globals->customerSpeed = 1500;

    sprintf(globals->customerDialoguePath, "Assets/Data/Customer Dialogues/%s/%s.json", globals->customerName, globals->currentDifficulty);

    *currentCustomerTexture = LoadTexture(TextFormat("Assets/Images/Customers/%s/Idle.png", globals->customerName));

    StartDialogue(&globals->dialogueUI, globals->customerDialoguePath, "dialogues");
    globals->neededItems = GetNeededItemsFromDialogue(globals->dialogueUI.dialogueJson);
    globals->neededItemLength = GetNeededItemsLengthFromDialogue(globals->dialogueUI.dialogueJson);

    *customerPaymentTexture = LoadTexture(cJSON_GetObjectItem(globals->dialogueUI.dialogueJson, "payment_texture_path")->valuestring);
}

int main(void){
    InitWindow(1500, 900, "CU");
    //SetTargetFPS(500);

    // 폰트 (한글도 되긴되게 했는데 FPS 2000대로 뚝떨어짐;)
    Font font;
    LoadFontAll(&font);
    
    // 글로벌 변수 초기화!!
    Globals * globals = GetGlobalVariables();
    InitGlobalVariables();
    globals->dialogueUI.font = &font;
 
    // Dialogue
    Texture currentCustomerTexture = LoadTexture("Assets/Images/Customers/Normal Customer/Idle.png");
    Texture customerPaymentTexture = (Texture){ 0 };
    float customerPosX = -currentCustomerTexture.width;

    NewCustomerIn("Normal Customer", &currentCustomerTexture, &customerPaymentTexture);
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

    // Scene Move Start
    ButtonUI sceneMoveBtns[2] = {
        NewButton("", (Rectangle){860, GetScreenHeight() - 120, 100, 100},
            LoadTexture("Assets/Images/UI/Go Left.png"),
            LoadTexture("Assets/Images/UI/Go Left Pressed.png"),
            LoadTexture("Assets/Images/UI/Go Left Hovered.png"),
            OnPrevSceneButtonClicked,
            false, "", ""
        ),

        NewButton("", (Rectangle){990, GetScreenHeight() - 120, 100, 100},
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
        (Rectangle) {GetScreenWidth()-400, GetScreenHeight() -120, 400, 120},
        LoadTexture("Assets/Images/UI/Inventory.png"),
        LoadTexture("Assets/Images/UI/Inventory Pressed.png"),
        LoadTexture("Assets/Images/UI/Inventory Hovered.png"),
        OnInventoryButtonClicked,
        true, "인벤토리", "담은 물건 확인"
    );
    
    Rectangle inventoryBG = (Rectangle){20, 20, 1060, GetScreenHeight() - 240};

    Texture inventoryTexture[5];

    ButtonUI inventoryDeleteButtons[INVENTORY_MAX_LEN];
    for (int i = 0; i < INVENTORY_MAX_LEN; i++){
        inventoryDeleteButtons[i] = NewButton(
            strdup(TextFormat("%d", i)), 
            (Rectangle) {950, i * 120 + 90, 100, 100},
            LoadTexture("Assets/Images/UI/Close Inventory.png"),
            LoadTexture("Assets/Images/UI/Close Inventory Pressed.png"),
            LoadTexture("Assets/Images/UI/Close Inventory Hovered.png"), 
            OnItemDeleteButtonClicked,
            true, "리턴", "다시 진열장에 두기"
        );
    }
    // Inventory End

    // Card Reader Start
    Rectangle cardReaderBG = (Rectangle){60, 60, 1020, GetScreenHeight() - 280};

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
                UpdateDialogueUI(&globals->dialogueUI, OnDialogueNextKeyPressed);
                
                // 고객은 항상 이동하려는중
                customerPosX = fmax(-600, fmin(270, customerPosX + globals->customerSpeed * deltaTime));
                if (customerPosX == -600 && (globals->isCustomerAngry || globals->isCustomerHappy)){
                    // 새 고객
                    NewCustomerIn("Kid Customer", &currentCustomerTexture, &customerPaymentTexture);
                }
                
                if (globals->isRingup && globals->currentCornerIndex == CORNER_COUNTER && globals->isInventoryOpened == false){
                    UpdateTransparentButton(&posMachineButton);
                    UpdateTransparentButton(&cardMachineButton);
                }

                // 씬을 움직일수 있을때 버튼 업데이트
                if (globals->canMoveCorner){
                    UpdateTransparentButton(&customerButton);
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

                    if (globals->isCustomerHappy && globals->dialogueUI.currentDialogueIndex == 3){
                        DrawTexture(customerPaymentTexture, 60, 60, WHITE);
                    }

                    // Dialogue UI
                    RenderDialogueUI(&globals->dialogueUI);
                    // Dialogue UI End

                    // Player Stats UI
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, GetScreenHeight()}, BLACK); // 
                    DrawRectangleRec((Rectangle){GetScreenWidth()-400, 0, 400, 100}, WHITE);
                    DrawTexture(playerNormalTexture, GetScreenWidth()-400, 0, WHITE);

                    DrawTextEx(font, TextFormat("Health : %d", globals->playerHealth), (Vector2){GetScreenWidth()-390, 10}, 25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 35, 300, 5}, RED);
                    
                    DrawTextEx(font, TextFormat("Store Rating : %d", globals->playerRating), (Vector2){GetScreenWidth()-390, 50},25, 2, BLACK);
                    DrawRectangleRec((Rectangle){GetScreenWidth()-390, 80, 300, 5}, BLUE);
                    // Player Stats UI End

                    // 현재 시간 보여줌
                    if (globals->currentCornerIndex == CORNER_COUNTER){
                        DrawTexture(clock, 40, 600, WHITE);
                    }

                    // Scene Move UI Start
                    DrawRectangleRec((Rectangle){850, GetScreenHeight() - 200, 250, 200}, BLUE);
                    if (globals->canMoveCorner){
                        for (int i = 0; i < 2; i++){
                            RenderButtonUI(&sceneMoveBtns[i]);
                        }
                        RenderButtonUI(&inventoryButtonUI);
                        DrawTextEx(font, TextFormat("%d", globals->currentInventoryLen), (Vector2){0, 0}, 100, 2, WHITE);
                    }
                    DrawTextEx(font, TextFormat("> %s", GetCurrentCornerName()), (Vector2){870, 725}, 35, 2, WHITE);
                    // Scene Move UI End

                    // Inventory UI Start
                    if (globals->isInventoryOpened){
                        DrawRectangleRec(inventoryBG, BLUE);
                        DrawRectangleRec((Rectangle){20, 20, 1060, 60}, BLACK); // 나중에 UI 그래픽으로 바꿔야함;

                        DrawTextEx(font, TextFormat("인벤토리 %d / 5", globals->currentInventoryLen), (Vector2){35, 35}, 40, 2, WHITE);
                        
                        for (int i = 0; i < INVENTORY_MAX_LEN; i++){
                            DrawRectangleGradientV(20, i * 120 + 100, 1060, 100, BLUE, (Color){ 0, 101, 211, 255 } );
                        }
                        
                        // 인벤토리에 있는 아이템 정보 렌더
                        for (int i = 0; i < globals->currentInventoryLen; i++){
                            DrawTextEx(font, globals->currentInventory[i], (Vector2){45, i * 120 + 100}, 40, 2, WHITE);
                            DrawTextEx(font, TextFormat("%d\\", globals->currentInventoryItemPrice[i]), (Vector2){505, i * 120 + 100}, 40, 2, WHITE);
                            DrawTextureV(globals->currentInventoryItemTextures[i], (Vector2){820, i * 120 + 90}, WHITE);
                            RenderButtonUI(&inventoryDeleteButtons[i]); // 아이템 지우기 버튼
                        }
                    }

                    if (globals->isCardMachineOpened){
                        DrawRectangleRec(cardReaderBG, BLACK);
                    }

                    if (globals->isPosMachineOpened){

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

// malloc 믿을게 못돼; 에잉쯧 나때는 말이야@@@~~~..;; 그냥 배열 써라 이게 낫다 ㅆ
void ApplyInventoryDeleted(){
    Globals* globals = GetGlobalVariables();

    int i, j;
    for (i = 0, j = 0; i < globals->currentInventoryLen; i++){
        if (globals->currentInventory[i] != "DESTROY"){
            globals->currentInventory[j] = globals->currentInventory[i];
            globals->currentInventoryItemPrice[j] = globals->currentInventoryItemPrice[i];
            globals->currentInventoryItemTextures[j] = globals->currentInventoryItemTextures[i];
            j++;
        }
    }
    globals->currentInventoryLen = j;
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
    
    globals->currentInventoryItemPrice[globals->currentInventoryLen] = itemStorage->price ;//strdup(TextFormat("%d\\", itemStorage->price));//= strdup(price);
    globals->currentInventoryLen ++;

    OnPlayerStoredItem(&globals->dialogueUI, itemStorage->itemName);
}

// 아이템 얻는 버튼 오른쪽 클릭 구현
void OnItemStorageItemRightClicked(ItemStorage* itemStorage){

}
 
// 아이템 지우는 버튼 구현
void OnItemDeleteButtonClicked(ButtonUI* btn){
    Globals * globals = GetGlobalVariables();
    globals->currentInventory[atoi(btn->objectName)] = "DESTROY";
    ApplyInventoryDeleted();
}

// 손님 클릭
void OnCustomerButtonClicked(TransparentButton* btn){
    Globals* globals = GetGlobalVariables();
    if (globals->currentInventoryLen == 0){
        OnPlayerDidNotBringItems(&globals->dialogueUI);
    }
    else{
        bool isCorrect = true;
        if (globals->neededItemLength != globals->currentInventoryLen){
            isCorrect = false;
        }
        else{
            bool* gotItems = (bool*)malloc(sizeof(bool) * globals->neededItemLength);
            for (int i = 0; i < globals->neededItemLength; i++){
                gotItems[i] = false;
            }
            for (int i = 0; i < globals->neededItemLength; i++){
                for (int j = 0; j < globals->currentInventoryLen; j++){
                    if (strcmp(globals->neededItems[i], globals->currentInventory[j]) == 0){
                        gotItems[i] = true;
                    }
                }
            }

            for (int i = 0; i < globals->currentInventoryLen; i++){
                if (gotItems[i] == false){
                    isCorrect = false;
                    break;
                }
            }
    
            free(gotItems);
        }
        
        cJSON* jsonData = GetJsonData(STAT_CHANGE_AMOUNT_JSON_DATA_PATH);
        if (isCorrect){
            globals->playerRating = fmin(100, globals->playerRating + cJSON_GetObjectItem(jsonData, "increase when correct item")->valueint);
        
            globals->canMoveCorner = false;
            globals->isCustomerHappy = true;
            StartDialogue(&globals->dialogueUI, globals->customerDialoguePath, "dialogues_happy");
            
            // 인벤토리 초기화
            for (int i = 0; i < 5; i++){
                globals->currentInventory[i] = "DESTROY";
            }
            ApplyInventoryDeleted();
        }
        else{
            globals->playerRating = fmax(0, globals->playerRating + cJSON_GetObjectItem(jsonData, "decrease when wrong item")->valueint);

            globals->canMoveCorner = false;
            globals->isCustomerAngry = true;
            StartDialogue(&globals->dialogueUI, globals->customerDialoguePath, "dialogues_angry");
            
            // 인벤토리 초기화
            for (int i = 0; i < 5; i++){
                globals->currentInventory[i] = "DESTROY";
            }
            ApplyInventoryDeleted();
        }
    }
}

// 카드 리더기 버튼 구현
void OnCardReaderButtonClicked(TransparentButton* btn){
    Globals* globals = GetGlobalVariables();
    globals->isCardMachineOpened = true;
}

// 포스기 버튼 구현
void OnPosMachineButtonClicked(TransparentButton* btn){
    Globals* globals = GetGlobalVariables();

    globals->isPosMachineOpened = true;
}

void OnButtonUIHovered(ButtonUI* buttonUI){
    if (buttonUI->toolTip.showToolTip){// buttonUI중 어떤건 toolTip없어서 확인해야함
        Globals* globals = GetGlobalVariables();
        globals->isToolTipShow = true;
        globals->toolTipName = buttonUI->toolTip.toolTipName;
        globals->toolTipText = buttonUI->toolTip.toolTipText;
    }
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

    globals->toolTipText = strdup(TextFormat("%d\\", itemStorageUI->price));
}

// 새 버튼 만들고 반환
ButtonUI NewButton(const char* objectName, Rectangle hitBox, Texture normalTexture, Texture pressedTexture, Texture hoveredTexture, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText){
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
TransparentButton NewTransparentButton(const char* objectName, Rectangle hitBox, void* clickedEvent, bool showTooltip, char* toolTipName, char* toolTipText){
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
    cJSON* itemsData = GetJsonData(ITEM_JSON_DATA_PATH);
    char* itemBar = GetItemCategoryFolderPath();
    cJSON* currentBar = cJSON_GetObjectItem(itemsData, itemBar);

    cJSON* itemPriceObject = cJSON_GetObjectItem(itemsData, TextFormat("%s Price", itemBar));

    for (int i = 0; i < 10; i++){
        char *itemName = cJSON_GetArrayItem(currentBar, i)->valuestring;
        int itemPrice = cJSON_GetObjectItem(itemPriceObject, itemName)->valueint;

        itemBtns[i] = (ItemStorage){
            (Rectangle){itemUiPos[i][0], itemUiPos[i][1], 100, 100},
            LoadTexture(TextFormat("Assets/Images/Items/%s/%d.png", itemBar, i)),
            itemName,
            itemPrice,
            OnItemStorageItemLeftClicked,
            OnItemStorageItemRightClicked,
            OnItemStorageUIHovered
        };
    }
}