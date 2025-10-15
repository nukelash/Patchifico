/*
colors...

PANEL_TITLE_FONT
PANEL_TITLE_FONT_SIZE
PANEL_TITLE_FONT_SPACING

LABEL_FONT_SIZE
LABEL_FONT_SPACING

PATCH_POINT_RADIUS

LINE_THICKNESS

*/
#pragma once

#include "raylib.h"

#define PACIFICO_GOLD CLITERAL(Color){254, 224, 33, 255}
#define PACIFICO_BROWN CLITERAL(Color){182, 126, 12, 255}
#define PACIFICO_RED CLITERAL(Color){248, 48, 23, 255}
#define PACIFICO_BLUE CLITERAL(Color){105, 175, 249, 255}
#define PACIFICO_GREEN CLITERAL(Color){73, 165, 11, 255}

Font PANEL_FONT;
#define PANEL_FONT_SIZE 12
#define PANEL_FONT_SPACING 0.5
#define PANEL_TITLE_FONT_SIZE 14
#define PANEL_TITLE_FONT_SPACING 1

void InitVisualConfig(){
    PANEL_FONT = LoadFontEx("/Users/lukenash/Downloads/made_tommy/MADE TOMMY Medium_PERSONAL USE.otf", 128, NULL, 0); // does nullptr vs NULL cause issues?

    return;
}