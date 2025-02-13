#include "dialogue.h"

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
    printf("%s\n", dialogueFileName);
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

int GetDialogueLen(cJSON* json){
    cJSON *dialogueArray = cJSON_GetObjectItem(json, "dialogues");
    return cJSON_GetArraySize(dialogueArray);
}

char*** GetDialogueData(cJSON* json){
    cJSON *dialogueArray = cJSON_GetObjectItem(json, "dialogues");

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
        printf("%s\n", neededItems[i]);
    }

    return neededItems;
}

int GetNeededItemsLengthFromDialogue(cJSON* jsonData){
    cJSON* neededItemArray = cJSON_GetObjectItem(jsonData, "item");
    return cJSON_GetArraySize(neededItemArray);
}