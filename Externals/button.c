#include "button.h"


void UpdateButtonUI(ButtonUI* btn){
    bool buttonAction = false;
    if (CheckCollisionPointRec(GetMousePosition(), btn->rect)){
        btn->OnHovered(btn);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            btn->currentTexture = &btn->pressedTexture;
        }
        else{
            btn->currentTexture = &btn->hoveredTexture;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            buttonAction = true;
        }
    }
    else{
        btn->currentTexture = &btn->normalTexture;
    }
    if (buttonAction){
        btn->OnClick(btn);
    }
}

void RenderButtonUI(ButtonUI* btn){
    DrawTexture(*(btn->currentTexture), btn->rect.x, btn->rect.y, WHITE);
}


void UpdateItemStorage(ItemStorage* storage){
    bool leftAction = false;
    bool rightAction = false;
    if (CheckCollisionPointRec(GetMousePosition(), storage->rect)){
        storage->OnHovered(storage);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            leftAction = true;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            rightAction = true;
        }
    }
    if (leftAction){
        storage->OnLeftClick(storage);
    }
    if (rightAction){
        storage->OnRightClick(storage);
    }
}

void RenderItemStorage(ItemStorage* storage){
    DrawTexture(storage->texture, storage->rect.x, storage->rect.y, WHITE);
}