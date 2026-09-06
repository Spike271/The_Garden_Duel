#pragma once
#include "Common.hpp"

struct Plant {
    PlantKind kind   = PK_NORMAL;
    int  need        = 0;
    int  given       = 0;
    bool done        = false;
    bool revealed    = false;
    bool dying       = false;
    float wobble     = 0.f;   // 1→0 celebrate wobble
    float dieT       = 0.f;   // 0→1 death anim
};
