#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// Window
constexpr unsigned int WINDOW_WIDTH  = 900u;
constexpr unsigned int WINDOW_HEIGHT = 500u;

// World
constexpr float GROUND_LEVEL         = 460.f;
constexpr float PLAYER_START_X       = 150.f;
constexpr float BG_SCROLL_SPEED      = 350.f;

// Game progression
constexpr float GAME_DURATION        = 50.f;
constexpr float VICTORY_DISTANCE     = 35000.f;

// Obstacle spawning
constexpr float SPAWN_INTERVAL_INIT  = 2.2f;
constexpr float SPAWN_ACCEL          = 1.4f;

// Coin spawning
constexpr float COIN_SPAWN_INTERVAL  = 2.0f;

// Hitbox shrink factors (shared by Joueur and Obstacle)
constexpr float HITBOX_H_SHRINK      = 0.25f;
constexpr float HITBOX_V_GAP         = 0.40f;

// HUD
constexpr float PROGRESS_BAR_W       = 200.f;
constexpr float PROGRESS_BAR_H       = 15.f;

// === LEVEL SYSTEM ===
constexpr int   MAX_LEVEL             = 3;
constexpr float LEVEL2_DISTANCE       = VICTORY_DISTANCE * 0.35f;   // 35% du trajet
constexpr float LEVEL3_DISTANCE       = VICTORY_DISTANCE * 0.70f;   // 70% du trajet
constexpr float LEVEL_SPEED_MULT[3]   = { 1.0f, 1.4f, 1.9f };      // multiplicateurs x1 / x1.4 / x1.9
constexpr float LEVEL_BAR_W           = 180.f;
constexpr float LEVEL_BAR_H           = 12.f;

#endif
