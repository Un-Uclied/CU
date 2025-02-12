#ifndef BUTTON_H
#define BUTTON_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"

typedef struct ButtonUI ButtonUI;
typedef struct ButtonUI{
    char* objectName;

    Rectangle rect;
    Texture normalTexture;
    Texture pressedTexture;
    Texture hoveredTexture;
    Texture* currentTexture;

    void (*OnClick)();

    void (*OnHovered)();
} ButtonUI;

typedef struct ItemStorage ItemStorage;
typedef struct ItemStorage{
    Rectangle rect;
    Texture texture;
    char* itemName;

    void (*OnLeftClick)();
    void (*OnRightClick)();

    void (*OnHovered)();
} ItemStorage;

void UpdateButtonUI(ButtonUI* btn);

void RenderButtonUI(ButtonUI* btn);

void UpdateItemStorage(ItemStorage* storage);

void RenderItemStorage(ItemStorage* storage);

#endif