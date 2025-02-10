#ifndef BUTTON_H
#define BUTTON_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wchar.h"

#include "raylib.h"
#include "raymath.h"

typedef struct {
    Rectangle rect;
    Texture normalTexture;
    Texture pressedTexture;
    Texture hoveredTexture;
    Texture* currentTexture;

    void (*OnClick)();
} ButtonUI;

void UpdateButtonUI(ButtonUI* btn);

void RenderButtonUI(ButtonUI* btn);

#endif