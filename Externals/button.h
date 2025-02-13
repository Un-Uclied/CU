#ifndef BUTTON_H
#define BUTTON_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"

typedef struct {
    bool showToolTip;
    char* toolTipName;
    char* toolTipText;
} ButtonTooltip;

typedef struct ButtonUI ButtonUI;
typedef struct ButtonUI{
    const char* objectName;

    Rectangle rect;
    Texture normalTexture;
    Texture pressedTexture;
    Texture hoveredTexture;
    Texture* currentTexture;

    void (*OnClick)();

    void (*OnHovered)();

    ButtonTooltip toolTip;
} ButtonUI;

typedef struct ItemStorage ItemStorage;
typedef struct ItemStorage{
    Rectangle rect;
    Texture texture;

    char* itemName;
    int price;

    void (*OnLeftClick)();
    void (*OnRightClick)();

    void (*OnHovered)();
} ItemStorage;

typedef struct TransparentButton TransparentButton;
typedef struct TransparentButton{
    const char* objectName;
    Rectangle rect;

    void (*OnClick)();

    void (*OnHovered)();

    ButtonTooltip toolTip;
} TransparentButton;

void UpdateButtonUI(ButtonUI* btn);

void RenderButtonUI(ButtonUI* btn);

void UpdateItemStorage(ItemStorage* storage);

void RenderItemStorage(ItemStorage* storage);

void UpdateTransparentButton(TransparentButton* btn);

#endif