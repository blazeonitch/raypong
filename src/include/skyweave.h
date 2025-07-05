#ifndef SKYWEAVE_H
#define SKYWEAVE_H

#include "raylib.h"
#include <stdbool.h>

bool CheckCollision(Vector2 pos1, Vector2 pos2, float radius1, float radius2)
{
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float distance = sqrt(dx * dx + dy * dy);

    return distance <= (radius1 + radius2);
}

#endif
