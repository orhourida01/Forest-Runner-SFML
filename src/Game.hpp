#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <optional>

#include "Menu.hpp"
#include "Joueur.hpp"
#include "Obstacle.hpp"
#include "Coin.hpp"

enum class GameState { MENU, PLAYING, DYING, GAME_OVER, VICTORY };

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;
    GameState        state;
    sf::Font         font;

    // Background
    sf::Texture               bgTexture;
    std::optional<sf::Sprite> bgSprite1;
    std::optional<sf::Sprite> bgSprite2;

    // Audio
    sf::Music                 bgMusic;
    bool                      bgMusicLoaded{false};
    sf::Music                 winMusic;
    bool                      winMusicLoaded{false};
    sf::SoundBuffer           gameOverBuffer;
    std::optional<sf::Sound>  gameOverSound;
    sf::SoundBuffer           coinBuffer;
    std::optional<sf::Sound>  coinSound;

    // HUD
    std::optional<sf::Text>   txtTime;
    std::optional<sf::Text>   txtCoinScore;
    std::optional<sf::Sprite> hudCoinIcon;
    sf::RectangleShape        progressBarOutline;
    sf::RectangleShape        progressBarFill;

    // Game state
    float  timeRemaining;
    float  distanceTraveled;
    float  spawnTimer;
    float  spawnInterval;
    float  coinSpawnTimer{0.f};
    int    coinScore{0};

    // === LEVEL SYSTEM ===
    int   currentLevel{1};
    float levelSpeedMult{1.0f};
    bool  levelUpFlash{false};
    float levelUpTimer{0.f};
    sf::RectangleShape      levelBarOutline;
    sf::RectangleShape      levelBarFill;
    std::optional<sf::Text> txtLevel;

    Player                                    player;
    std::vector<std::unique_ptr<Obstacle>>    obstacles;
    std::vector<std::unique_ptr<Coin>>        coins;
    Menu                                      menu;
    sf::Texture                               coinTexture;

    sf::Texture               deadTexture;
    std::optional<sf::Sprite> deadSprite;
    sf::Clock                 dyingClock;
    sf::Clock                 endScreenClock;

    void loadResources();
    void resetGame();
    void handleEvents();
    void update(float dt);
    void render();

    // Sous-systèmes de update()
    void spawnObstacle();
    void spawnCoin();
    void checkCollisions();

    // Sous-systèmes de render()
    void renderHUD();
    void showDyingScreen();
    void showVictoryScreen();
    void showGameOverScreen();
    void showAboutScreen();
};

#endif
