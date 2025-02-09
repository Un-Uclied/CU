#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "raylib.h"
#include "raymath.h"
#include "Assets/cJson/cJSON.c"

// JSON 파일 읽기 함수
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


char*** GetDialougeData(char* dialougeFileName){
    // JSON 파일 읽기
    char *jsonString = ReadFileToString(dialougeFileName);
    if (!jsonString) return -1;

    // JSON 파싱
    cJSON *json = cJSON_Parse(jsonString);
    free(jsonString);  // 문자열 메모리 해제

    if (!json) {
        printf("JSON 파싱 오류!\n");
        return -1;
    }

    int dialogueLength = cJSON_GetObjectItem(json, "dialogue length")->valueint;
    // char dialogue[dialougeLength][2][20];
    //char dialogue[2][2][20] = {{"Player", "Wario"}, {"Customer", "no fuck ur mom bro"}};
    char*** dialogue = malloc(sizeof(char***) * dialogueLength * sizeof(char**) * 2 * sizeof(char*) * 20);
    for (int i = 0; i < dialogueLength; i++){
        char index[2];
        cJSON *dialogueData = cJSON_GetObjectItem(json, sprintf(index, "%d", i)); // {"Player", "내용"}
        strcpy(dialogue[i][0], cJSON_GetArrayItem(dialogueData, 0)->valuestring);
        strcpy(dialogue[i][1], cJSON_GetArrayItem(dialogueData, 1)->valuestring);
    }

    return dialogue;
}

int main(void){

    InitWindow(1500, 900, "CU");

    Camera2D camera = (Camera2D){0};
    camera.zoom = 1;
    camera.target = (Vector2) {0, 0};

    Font font = LoadFont("Assets/Fonts/SB Aggro M.ttf");

    while (!WindowShouldClose()){
        ClearBackground(RAYWHITE);

        BeginDrawing();

            BeginMode2D(camera);

            EndMode2D();

            DrawRectangleRec((Rectangle){0, GetScreenHeight() - 200, GetScreenWidth() - 400, 200}, BLACK);

            DrawTextEx(font, dialouge2[0][0], (Vector2){0, 0}, 30, 1, BLACK);
            DrawFPS(1400, 10);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}