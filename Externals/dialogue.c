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

cJSON* GetDialogueJson(char* dialogueFileName){
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

int GetDialogueLen(cJSON* json){
    return cJSON_GetObjectItem(json, "dialogue length")->valueint;
}

char*** GetDialogueData(cJSON* json){
    int dialogueLength = cJSON_GetObjectItem(json, "dialogue length")->valueint;
    // char dialogue[dialougeLength][2][20];
    //char dialogue[2][2][20] = {{"Player", "Wario"}, {"Customer", "no fuck ur mom bro"}};
    char ***dialogue = malloc(sizeof(char**) * dialogueLength);
    for (int i = 0; i < dialogueLength; i++) {
        dialogue[i] = malloc(sizeof(char*) * 2);
        dialogue[i][0] = malloc(20);  // 플레이어 이름 (최대 20글자)
        dialogue[i][1] = malloc(200); // 대화 내용 (최대 200글자)
    }
    cJSON *dialogueArray = cJSON_GetObjectItem(json, "dialogues");
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

void DialogueGenerateFontAtlas(Font* dialogueFont, char*** dialogue, int dialogueLen){
    char str[1000];
    for (int i = 0; i < dialogueLen; i++){
        strcat(str, dialogue[i][1]);
    }
    for (int i = 0; i < dialogueLen; i++){
        strcat(str, dialogue[i][0]);
    }
    int codepointCount = 0;
    int *codepoints = LoadCodepoints(str, &codepointCount);
    *dialogueFont = LoadFontEx("Assets/Fonts/SB Aggro M.ttf", 70, codepoints, codepointCount);
    SetTextureFilter(dialogueFont->texture, TEXTURE_FILTER_BILINEAR);
    //GenTextureMipmaps(&(dialogueFont->texture));
}