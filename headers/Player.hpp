#pragma once
#include "Common.hpp"
#include "raylib.h"
#include <string>

struct Player
{
    std::string name;
    Color       col{};
    int   side       = 0;    // 0=left, 1=right
    int   gridPos    = -1;   // -1=left river, N=right river
    float vx         = 0.f;  // smooth visual x
    float vy         = 0.f;
    float tx         = 0.f;  // lerp target x
    int   water      = 0;
    int   maxWater   = 0;
    int   refills    = 0;
    int   score      = 0;
    PUKind pu        = PU_NONE;
    int   puCharges  = 0;
    float moveCd     = 0.f;
    float actCd      = 0.f;
    bool  watering   = false;
    float waterT     = 0.f;
};

inline const char* PUName(const PUKind k)
{
    switch (k)
    {
        case PU_BIGCAN:   return "Big Can";
        case PU_SKIP:     return "Skip";
        case PU_SABOTAGE: return "Sabotage";
        default:          return "None";
    }
}

inline Color PUColor(const PUKind k)
{
    switch (k)
    {
        case PU_BIGCAN:   return SKYBLUE;
        case PU_SKIP:     return YELLOW;
        case PU_SABOTAGE: return Color{.r = 240, .g = 80, .b = 80, .a = 255};
        default:          return Color{.r = 120, .g = 120, .b = 120, .a = 255};
    }
}
