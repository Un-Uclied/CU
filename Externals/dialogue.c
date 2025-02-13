#include "dialogue.h"

// 백엔드

char *ReadFileToString(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("파일을 열 수 없습니다: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';  // 문자열 종료

    fclose(file);
    return buffer;
}

cJSON* GetJson(char* dialogueFileName){
    // JSON 파일 읽기
    char *jsonString = ReadFileToString(dialogueFileName);
    if (!jsonString) return NULL;

    // JSON 파싱
    cJSON *json = cJSON_Parse(jsonString);
    free(jsonString);  // 문자열 메모리 해제

    if (!json) {
        printf("JSON 파싱 오류!\n");
        return NULL;
    }

    return json;
}

cJSON* GetJsonData(char* fileName){
    cJSON* jsonData = GetJson(fileName);
    if (jsonData == NULL){
        exit(-1);
    }
    else{
        return jsonData;
    }
}

int GetDialogueLen(cJSON* json, char* dialogueKey){
    cJSON *dialogueArray = cJSON_GetObjectItem(json, dialogueKey);
    return cJSON_GetArraySize(dialogueArray);
}

char*** GetDialogueData(cJSON* json, char* dialogueKey){
    cJSON *dialogueArray = cJSON_GetObjectItem(json, dialogueKey);

    int dialogueLength = cJSON_GetArraySize(dialogueArray);

    // char dialogue[dialougeLength][2][20];
    //char dialogue[2][2][20] = {{"Player", "Wario"}, {"Customer", "no fuck ur mom bro"}};
    char ***dialogue = malloc(sizeof(char**) * dialogueLength);
    for (int i = 0; i < dialogueLength; i++) {
        dialogue[i] = malloc(sizeof(char*) * 2);
        dialogue[i][0] = malloc(20);  // 플레이어 이름 (최대 20글자)
        dialogue[i][1] = malloc(200); // 대화 내용 (최대 200글자)
    }
    if (!cJSON_IsArray(dialogueArray)) {
        printf("JSON 데이터가 배열이 아님!\n");
        return NULL;
    }

    for (int i = 0; i < dialogueLength; i++) {
        cJSON *dialogueData = cJSON_GetArrayItem(dialogueArray, i);
        if (!cJSON_IsObject(dialogueData)) continue;

        cJSON *speaker = cJSON_GetObjectItem(dialogueData, "speaker");
        cJSON *text = cJSON_GetObjectItem(dialogueData, "text");

        if (cJSON_IsString(speaker) && cJSON_IsString(text)) {
            strcpy(dialogue[i][0], speaker->valuestring);
            strcpy(dialogue[i][1], text->valuestring);
        }
    }

    return dialogue;
}

char** GetNeededItemsFromDialogue(cJSON* jsonData){
    cJSON* neededItemArray = cJSON_GetObjectItem(jsonData, "item");
    int neededItemCount = cJSON_GetArraySize(neededItemArray);
    char** neededItems = (char**)malloc(sizeof(char*) * neededItemCount);
   
    for (int i = 0; i < neededItemCount; i++){
        neededItems[i] = cJSON_GetArrayItem(neededItemArray, i)->valuestring;
    }

    return neededItems;
}

int GetNeededItemsLengthFromDialogue(cJSON* jsonData){
    cJSON* neededItemArray = cJSON_GetObjectItem(jsonData, "item");
    return cJSON_GetArraySize(neededItemArray);
}

// 프론트엔드

void StartDialogue(DialogueUI* dialogueUI, char* dialogueFilePath, char* dialogueKey){     
    dialogueUI->dialogueJson = GetJsonData(dialogueFilePath);
    dialogueUI->dialogue = GetDialogueData(dialogueUI->dialogueJson, dialogueKey);
    dialogueUI->dialogueLen = GetDialogueLen(dialogueUI->dialogueJson, dialogueKey);
    dialogueUI->currentDialogueIndex = 0;

    dialogueUI->currentSpeaker = dialogueUI->dialogue[dialogueUI->currentDialogueIndex][0];
    dialogueUI->currentText = dialogueUI->dialogue[dialogueUI->currentDialogueIndex][1];
}

void UpdateDialogueUI(DialogueUI* dialogueUI, void (*OnNextKeyPressed)()){
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_SPACE)){
        OnNextKeyPressed(dialogueUI);
    }
}

void RenderDialogueUI(DialogueUI* dialogueUI){
    DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);
    DrawTextEx(*dialogueUI->font, dialogueUI->currentSpeaker, (Vector2){30, GetScreenHeight() - 180}, 50, 1, WHITE);
    DrawTextEx(*dialogueUI->font, dialogueUI->currentText, (Vector2){80, GetScreenHeight() - 120}, 40, 1, WHITE);
}

void OnPlayerDidNotBringItems(DialogueUI* dialogueUI){
    dialogueUI->currentSpeaker = "당신";
    dialogueUI->currentText = "(아직 손님이 요청한\n물건들을 갖고 오지 않은것같다.)";
}

void OnPlayerStoredItem(DialogueUI* dialogueUI, char* itemName){
    dialogueUI->currentSpeaker = "당신";
    dialogueUI->currentText = strdup(TextFormat("(%s을(를) 바구니에 담았다.)", itemName));
}
