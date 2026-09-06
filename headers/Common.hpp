#pragma once
#include <numbers>

#include "raylib.h"

// Constants
inline constexpr int SW = 1280;
inline constexpr int SH = 720;
inline constexpr int GY = 460; // garden centre Y
inline constexpr int RLX = 68; // left-river X
inline constexpr int RRX = 1212; // right-river X
inline constexpr float MOVE_CD = 0.13f;
inline constexpr float ACT_CD = 0.38f;

inline constexpr float TIME_LIMIT[3] = {10.f, 15.f, 25.f};
inline constexpr float MY_PI = std::numbers::pi_v<float>;
inline Font customFont;

enum GameState { GS_MENU, GS_PLAY, GS_OVER };

enum GameMode { GM_PVP, GM_PVC };

enum PlantKind { PK_NORMAL, PK_WILTED, PK_WEED, PK_BONUS, PK_SABOTAGE };

enum PUKind { PU_NONE, PU_BIGCAN, PU_SKIP, PU_SABOTAGE };
