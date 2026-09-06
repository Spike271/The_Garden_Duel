#pragma once
#include "raylib.h"

struct Particle {
    Vector2 pos, vel;
    float   life, maxLife, radius;
    Color   col;
};
