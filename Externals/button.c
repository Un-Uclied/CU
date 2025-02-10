#include "button.h"

void UpdateButtonUI(ButtonUI* btn){
    bool buttonAction = false;
    if (CheckCollisionPointRec(GetMousePosition(), btn->rect)){
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
        btn->OnClick();
    }
}

void RenderButtonUI(ButtonUI* btn){
    DrawTexture(*(btn->currentTexture), btn->rect.x, btn->rect.y, WHITE);
}