#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wchar.h"

#include "raylib.h"
#include "raymath.h"

#include "../Assets/cJson/cJSON.c"

typedef struct DialogueUI{
    Font* font;

    cJSON* dialogueJson;
    char*** dialogue;
    int dialogueLen;
    int currentDialogueIndex;

    char* currentSpeaker;
    char* currentText;

    bool isDialogueEnd;
} DialogueUI;

char *ReadFileToString(const char *filename);

//cJSON* GetDialogueJson(char* dialogueFileName);

int GetDialogueLen(cJSON* json, char* dialogueKey);

char*** GetDialogueData(cJSON* json, char* dialogueKey);

#endif