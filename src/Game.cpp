#include "Game.hpp"
#include "Constants.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <ctime>

static const float COIN_HEIGHTS[3] = {
    GROUND_LEVEL - 20.f,
    GROUND_LEVEL - 110.f,
    GROUND_LEVEL - 200.f
};

static std::string formatTime(int secs)
{
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << secs / 60
        << ":" << std::setw(2) << std::setfill('0') << secs % 60;
    return oss.str();
}

static void centerText(sf::Text& txt, float x, float y)
{
    const sf::FloatRect r = txt.getLocalBounds();
    txt.setOrigin({r.position.x + r.size.x / 2.f, r.position.y + r.size.y / 2.f});
    txt.setPosition({x, y});
}

Game::Game()
    : window(sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Forest Runner")
    , state(GameState::MENU)
    , timeRemaining(GAME_DURATION)
    , distanceTraveled(0.f)
    , spawnTimer(0.f)
    , spawnInterval(SPAWN_INTERVAL_INIT)
    , player()
    , menu(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT))
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    window.setFramerateLimit(60);

    // Vue logique fixe 900×500 : tout le contenu est rendu dans cet espace,
    // SFML s'occupe du mapping vers la résolution réelle (windowed ou fullscreen).
    window.setView(sf::View(sf::FloatRect(
        {0.f, 0.f},
        {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}
    )));

    // Window icon
    sf::Image icon;
    if (icon.loadFromFile("assets/images/game_icon.png"))
        window.setIcon(icon);

    loadResources();
    render(); // show first frame immediately — avoids blank flash on startup
}

void Game::loadResources()
{
    if (!font.openFromFile("assets/fonts/font.ttf"))
        throw std::runtime_error("Cannot load font.");

    if (!bgTexture.loadFromFile("assets/images/background.jpg"))
        throw std::runtime_error("Cannot load background.");

    const sf::Vector2u texSize = bgTexture.getSize();
    const float bgScaleX = static_cast<float>(WINDOW_WIDTH)  / static_cast<float>(texSize.x);
    const float bgScaleY = static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(texSize.y);

    bgSprite1.emplace(bgTexture);
    bgSprite1->setScale({bgScaleX, bgScaleY});
    bgSprite2.emplace(bgTexture);
    bgSprite2->setScale({bgScaleX, bgScaleY});
    bgSprite2->setPosition({static_cast<float>(WINDOW_WIDTH), 0.f});

    if (!coinTexture.loadFromFile("assets/images/coin.png"))
        throw std::runtime_error("Cannot load coin.png.");
    coinTexture.setSmooth(true);

    const sf::Vector2u cs = coinTexture.getSize();
    const float iconScale = 22.f / static_cast<float>(std::max(cs.x, cs.y));
    hudCoinIcon.emplace(coinTexture);
    hudCoinIcon->setScale({iconScale, iconScale});
    hudCoinIcon->setPosition({20.f, 40.f});

    txtTime.emplace(font, "", 22);
    txtTime->setFillColor(sf::Color::White);
    txtTime->setPosition({20.f, 12.f});

    txtCoinScore.emplace(font, "x 0", 20);
    txtCoinScore->setFillColor(sf::Color(255, 220, 50));
    txtCoinScore->setPosition({48.f, 38.f});

    progressBarOutline.setSize({PROGRESS_BAR_W, PROGRESS_BAR_H});
    progressBarOutline.setFillColor(sf::Color::Transparent);
    progressBarOutline.setOutlineColor(sf::Color::White);
    progressBarOutline.setOutlineThickness(2.f);
    progressBarOutline.setPosition({static_cast<float>(WINDOW_WIDTH) - PROGRESS_BAR_W - 20.f, 14.f});

    progressBarFill.setSize({0.f, PROGRESS_BAR_H});
    progressBarFill.setFillColor(sf::Color(50, 200, 80));
    progressBarFill.setPosition(progressBarOutline.getPosition());

    // === LEVEL BAR (en haut au centre, alignee avec le HUD) ===
    levelBarOutline.setSize({LEVEL_BAR_W, LEVEL_BAR_H});
    levelBarOutline.setFillColor(sf::Color::Transparent);
    levelBarOutline.setOutlineColor(sf::Color(180, 180, 180));
    levelBarOutline.setOutlineThickness(2.f);
    levelBarOutline.setPosition({(static_cast<float>(WINDOW_WIDTH) - LEVEL_BAR_W) / 2.f, 36.f});

    levelBarFill.setSize({0.f, LEVEL_BAR_H});
    levelBarFill.setFillColor(sf::Color(80, 180, 255));
    levelBarFill.setPosition(levelBarOutline.getPosition());

    txtLevel.emplace(font, "LEVEL 1", 10u);
    txtLevel->setFillColor(sf::Color::White);
    txtLevel->setStyle(sf::Text::Bold);

    if (gameOverBuffer.loadFromFile("assets/sounds/game_over.wav"))
        gameOverSound.emplace(gameOverBuffer);

    if (coinBuffer.loadFromFile("assets/sounds/coin_collect.mp3"))
        coinSound.emplace(coinBuffer);

    if (bgMusic.openFromFile("assets/music/background.wav")) {
        bgMusic.setLooping(true);
        bgMusicLoaded = true;
        bgMusic.play(); // play from the very first frame (menu)
    }

    if (deadTexture.loadFromFile("assets/images/player_dead.png")) {
        deadTexture.setSmooth(true);
        deadSprite.emplace(deadTexture);
        const sf::Vector2u ds = deadTexture.getSize();
        const float scale = std::min(300.f / static_cast<float>(ds.x),
                                     300.f / static_cast<float>(ds.y));
        deadSprite->setScale({scale, scale});
        deadSprite->setOrigin({ds.x / 2.f, ds.y / 2.f});
        deadSprite->setPosition({static_cast<float>(WINDOW_WIDTH)  / 2.f,
                                 static_cast<float>(WINDOW_HEIGHT) / 2.f});
    }

    if (winMusic.openFromFile("assets/sounds/win.mp3")) {
        winMusic.setLooping(false);
        winMusicLoaded = true;
    }
}

void Game::run()
{
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        handleEvents();
        if (state == GameState::PLAYING || state == GameState::DYING) update(dt);
        render();
    }
}

void Game::handleEvents()
{
    while (const std::optional<sf::Event> event = window.pollEvent()) {

        if (event->is<sf::Event::Closed>()) { window.close(); return; }

        if (state == GameState::MENU) {
            int choice = menu.handleEvent(window, *event);
            if      (choice == 0) { resetGame(); state = GameState::PLAYING; }
            else if (choice == 1) { showAboutScreen(); }
            else if (choice == 2) { window.close(); }
        }

        if (state == GameState::PLAYING) {
            if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
                if (k->code == sf::Keyboard::Key::Space ||
                    k->code == sf::Keyboard::Key::Up)
                    player.jump();
                if (k->code == sf::Keyboard::Key::Escape) {
                    state = GameState::MENU;
                    if (bgMusicLoaded) bgMusic.stop();
                }
                if (k->code == sf::Keyboard::Key::Down)
                    player.crouch(true);
            }
            if (const auto* k = event->getIf<sf::Event::KeyReleased>()) {
                if (k->code == sf::Keyboard::Key::Down)
                    player.crouch(false);
            }
        }

        if (state == GameState::GAME_OVER || state == GameState::VICTORY) {
            if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
                if (k->code == sf::Keyboard::Key::Enter) {
                    if (state == GameState::VICTORY && winMusicLoaded) winMusic.stop();
                    state = GameState::MENU;
                    if (bgMusicLoaded) bgMusic.play();
                }
            }
        }
    }
}

void Game::resetGame()
{
    obstacles.clear();
    coins.clear();
    player.reset();
    timeRemaining    = GAME_DURATION;
    distanceTraveled = 0.f;
    spawnTimer       = 0.f;
    spawnInterval    = SPAWN_INTERVAL_INIT;
    coinSpawnTimer   = 0.f;
    coinScore        = 0;

    // === RESET LEVEL ===
    currentLevel   = 1;
    levelSpeedMult = LEVEL_SPEED_MULT[0];
    levelUpFlash   = false;
    levelUpTimer   = 0.f;
    levelBarFill.setFillColor(sf::Color(80, 180, 255));
    levelBarFill.setSize({0.f, LEVEL_BAR_H});
    if (bgMusicLoaded) {
        if (bgMusic.getStatus() != sf::Music::Status::Playing)
            bgMusic.play();
    }
}

void Game::update(float dt)
{
    // Handle dying transition (frozen game, show dead image for 2s)
    if (state == GameState::DYING) {
        if (dyingClock.getElapsedTime().asSeconds() >= 2.f) {
            state = GameState::GAME_OVER;
            endScreenClock.restart();
        }
        return;
    }

    timeRemaining -= dt;
    if (timeRemaining <= 0.f) {
        timeRemaining = 0.f;
        state = GameState::DYING;
        dyingClock.restart();
        if (bgMusicLoaded) bgMusic.stop();
        if (gameOverSound) gameOverSound->play();
        return;
    }

    distanceTraveled += BG_SCROLL_SPEED * dt;
    const float bgW    = static_cast<float>(WINDOW_WIDTH);
    const float offset = std::fmod(distanceTraveled, bgW);

    bgSprite1->setPosition({-offset, 0.f});
    bgSprite2->setPosition({bgW - offset, 0.f});

    player.update(dt);
    if (player.isDead()) {
        state = GameState::DYING;
        dyingClock.restart();
        if (bgMusicLoaded) bgMusic.stop();
        if (gameOverSound) gameOverSound->play();
        return;
    }

    spawnTimer     += dt;
    coinSpawnTimer += dt;
    spawnObstacle();
    spawnCoin();

    // Mise à jour + suppression hors-écran
    for (auto& obs : obstacles) obs->update(dt * levelSpeedMult);
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
                    [](const std::unique_ptr<Obstacle>& o) { return o->isOffScreen(); }),
                    obstacles.end());

    for (auto& c : coins) c->update(dt * levelSpeedMult);
    coins.erase(std::remove_if(coins.begin(), coins.end(),
                [](const std::unique_ptr<Coin>& c) { return c->isOffScreen(); }),
                coins.end());

    checkCollisions();

    // === LEVEL PROGRESSION ===
    {
        int newLevel = 1;
        if      (distanceTraveled >= LEVEL3_DISTANCE) newLevel = 3;
        else if (distanceTraveled >= LEVEL2_DISTANCE) newLevel = 2;

        if (newLevel != currentLevel) {
            currentLevel   = newLevel;
            levelSpeedMult = LEVEL_SPEED_MULT[currentLevel - 1];
            levelUpFlash   = true;
            levelUpTimer   = 0.f;
            if      (currentLevel == 2) levelBarFill.setFillColor(sf::Color(255, 180, 30));
            else if (currentLevel == 3) levelBarFill.setFillColor(sf::Color(255, 60,  60));
        }

        if (levelUpFlash) {
            levelUpTimer += dt;
            if (levelUpTimer >= 2.0f) levelUpFlash = false;
        }

        // Remplissage barre : progression vers le prochain level
        float barRatio = 0.f;
        if (currentLevel == 1)
            barRatio = distanceTraveled / LEVEL2_DISTANCE;
        else if (currentLevel == 2)
            barRatio = (distanceTraveled - LEVEL2_DISTANCE) / (LEVEL3_DISTANCE - LEVEL2_DISTANCE);
        else
            barRatio = (distanceTraveled - LEVEL3_DISTANCE) / (VICTORY_DISTANCE - LEVEL3_DISTANCE);
        barRatio = std::min(barRatio, 1.f);
        levelBarFill.setSize({LEVEL_BAR_W * barRatio, LEVEL_BAR_H});
    }

    if (distanceTraveled >= VICTORY_DISTANCE) {
        state = GameState::VICTORY;
        endScreenClock.restart();
        if (bgMusicLoaded) bgMusic.stop();
        if (winMusicLoaded) winMusic.play();
    }

    const float ratio = std::min(distanceTraveled / VICTORY_DISTANCE, 1.f);
    progressBarFill.setSize({PROGRESS_BAR_W * ratio, PROGRESS_BAR_H});
}

void Game::render()
{
    window.clear(sf::Color(20, 20, 30));

    switch (state) {
        case GameState::MENU:
            // Background visible behind the menu
            window.draw(*bgSprite1);
            window.draw(*bgSprite2);
            menu.draw(window);
            break;
        case GameState::PLAYING:
            window.draw(*bgSprite1);
            window.draw(*bgSprite2);
            for (auto& c : coins)     c->draw(window);
            for (auto& o : obstacles) o->draw(window);
            player.draw(window);
            renderHUD();
            break;
        case GameState::DYING:     showDyingScreen();    break;
        case GameState::GAME_OVER: showGameOverScreen(); break;
        case GameState::VICTORY:   showVictoryScreen();  break;
        default: break;
    }

    window.display();
}

void Game::renderHUD()
{
    txtTime->setString("Temps : " + formatTime(static_cast<int>(timeRemaining)));
    window.draw(*txtTime);

    txtCoinScore->setString("x " + std::to_string(coinScore));
    window.draw(*hudCoinIcon);
    window.draw(*txtCoinScore);

    window.draw(progressBarOutline);
    window.draw(progressBarFill);

    // === BARRE DE LEVEL (centree en haut) ===
    window.draw(levelBarOutline);
    window.draw(levelBarFill);

    // Label "LEVEL X" centre dans la barre
    txtLevel->setString("LEVEL " + std::to_string(currentLevel));
    {
        const sf::FloatRect r  = txtLevel->getLocalBounds();
        const sf::Vector2f  bp = levelBarOutline.getPosition();
        txtLevel->setOrigin({r.position.x + r.size.x / 2.f, r.position.y + r.size.y / 2.f});
        txtLevel->setPosition({bp.x + LEVEL_BAR_W / 2.f, bp.y + LEVEL_BAR_H / 2.f});
    }
    window.draw(*txtLevel);

    // === FLASH "LEVEL X !" au centre de l'écran ===
    if (levelUpFlash) {
        const float alpha = std::max(0.f, 1.f - levelUpTimer / 2.0f);
        const auto  a     = static_cast<std::uint8_t>(alpha * 255.f);
        const float scale = 1.f + 0.3f * (1.f - levelUpTimer / 2.0f);

        sf::Text flash(font, "LEVEL " + std::to_string(currentLevel) + " !", 46u);
        flash.setStyle(sf::Text::Bold);

        sf::Color col;
        if      (currentLevel == 2) col = sf::Color(255, 200, 30,  a);
        else if (currentLevel == 3) col = sf::Color(255, 80,  50,  a);
        else                        col = sf::Color(100, 200, 255, a);
        flash.setFillColor(col);
        flash.setScale({scale, scale});

        const sf::FloatRect r = flash.getLocalBounds();
        flash.setOrigin({r.position.x + r.size.x / 2.f, r.position.y + r.size.y / 2.f});
        flash.setPosition({static_cast<float>(WINDOW_WIDTH)  / 2.f,
                           static_cast<float>(WINDOW_HEIGHT) / 2.f - 80.f});
        window.draw(flash);
    }
}

void Game::showVictoryScreen()
{
    const float W  = static_cast<float>(WINDOW_WIDTH);
    const float H  = static_cast<float>(WINDOW_HEIGHT);
    const float cx = W / 2.f;
    const float cy = H / 2.f;
    const float t  = endScreenClock.getElapsedTime().asSeconds();

    window.draw(*bgSprite1);
    window.draw(*bgSprite2);

    sf::RectangleShape overlay({W, H});
    overlay.setFillColor(sf::Color(5, 10, 30, 210));
    window.draw(overlay);

    for (int i = 0; i < 18; ++i) {
        const float fi    = static_cast<float>(i);
        const float phase = fi * 0.72f;
        const float sx    = W * (fi / 17.f);
        const float sy    = std::fmod(H * (fi / 17.f) + t * (25.f + (i % 5) * 8.f), H);
        const float r     = 3.f + 2.f * std::sin(t * 3.f + phase);
        const auto  alpha = static_cast<std::uint8_t>(130.f + 100.f * std::sin(t * 2.f + phase));
        sf::CircleShape p(r);
        p.setFillColor((i % 2 == 0) ? sf::Color(255, 215, 0, alpha) : sf::Color(100, 255, 130, alpha));
        p.setOrigin({r, r});
        p.setPosition({sx, sy});
        window.draw(p);
    }

    sf::RectangleShape card({520.f, 340.f});
    card.setFillColor(sf::Color(8, 14, 40, 230));
    card.setOutlineColor(sf::Color(255, 200, 50));
    card.setOutlineThickness(2.f + 1.5f * std::sin(t * 3.f));
    card.setOrigin({260.f, 170.f});
    card.setPosition({cx, cy});
    window.draw(card);

    const float glow  = (1.f + std::sin(t * 2.2f)) / 2.f;
    sf::Text title(font, "VICTOIRE !", 50);
    title.setFillColor(sf::Color(255,
        static_cast<std::uint8_t>(200 + static_cast<int>(glow * 55.f)),
        static_cast<std::uint8_t>(static_cast<int>(glow * 80.f))));
    title.setStyle(sf::Text::Bold);
    title.setScale({1.f + 0.05f * std::sin(t * 3.f), 1.f + 0.05f * std::sin(t * 3.f)});
    centerText(title, cx, cy - 120.f);
    window.draw(title);

    sf::Text sub(font, "Bunker atteint ! Bien joue !", 17);
    sub.setFillColor(sf::Color(170, 230, 170));
    centerText(sub, cx, cy - 78.f);
    window.draw(sub);

    sf::RectangleShape sep({380.f, 1.f});
    sep.setFillColor(sf::Color(255, 200, 50, 180));
    sep.setOrigin({190.f, 0.5f});
    sep.setPosition({cx, cy - 55.f});
    window.draw(sep);

    sf::Text statTime(font, "Temps restant   " + formatTime(static_cast<int>(timeRemaining)), 19);
    statTime.setFillColor(sf::Color(210, 220, 255));
    centerText(statTime, cx, cy - 25.f);
    window.draw(statTime);

    sf::Text statCoins(font, "Pieces collectees   " + std::to_string(coinScore), 19);
    statCoins.setFillColor(sf::Color(255, 220, 80));
    centerText(statCoins, cx, cy + 10.f);
    window.draw(statCoins);

    std::string ratingStr;
    sf::Color   ratingColor;
    if      (timeRemaining >= 35.f) { ratingStr = "Note  :  PARFAIT !";     ratingColor = sf::Color(255, 215, 0);   }
    else if (timeRemaining >= 25.f) { ratingStr = "Note  :  EXCELLENT !";   ratingColor = sf::Color(255, 200, 50);  }
    else if (timeRemaining >= 10.f) { ratingStr = "Note  :  BIEN !";        ratingColor = sf::Color(100, 255, 120); }
    else                            { ratingStr = "Note  :  DE JUSTESSE !"; ratingColor = sf::Color(255, 160, 40);  }
    sf::Text statRating(font, ratingStr, 19);
    statRating.setFillColor(ratingColor);
    centerText(statRating, cx, cy + 45.f);
    window.draw(statRating);

    sf::Text hint(font, "[ Entree ]   Retour au menu", 16);
    hint.setFillColor(sf::Color(200, 200, 200,
        static_cast<std::uint8_t>(128.f + 120.f * std::sin(t * 2.f))));
    centerText(hint, cx, cy + 118.f);
    window.draw(hint);
}

void Game::spawnObstacle()
{
    spawnInterval = SPAWN_INTERVAL_INIT -
                    (GAME_DURATION - timeRemaining) / GAME_DURATION * SPAWN_ACCEL;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.f;
        obstacles.push_back(std::make_unique<Obstacle>(
            static_cast<float>(WINDOW_WIDTH) + 20.f));
    }
}

void Game::spawnCoin()
{
    if (coinSpawnTimer >= COIN_SPAWN_INTERVAL) {
        coinSpawnTimer = 0.f;
        coins.push_back(std::make_unique<Coin>(
            coinTexture,
            static_cast<float>(WINDOW_WIDTH) + 30.f,
            COIN_HEIGHTS[std::rand() % 3]));
    }
}

void Game::checkCollisions()
{
    // Obstacle collision — un seul choc = mort
    for (auto it = obstacles.begin(); it != obstacles.end(); ++it) {
        if (player.getBounds().findIntersection((*it)->getBounds())) {
            player.hurt();
            obstacles.erase(it);
            return; // évite d'itérer sur un conteneur modifié
        }
    }

    // Collecte de pièces
    for (auto it = coins.begin(); it != coins.end(); ++it) {
        if (player.getBounds().findIntersection((*it)->getBounds())) {
            ++coinScore;
            if (coinSound) coinSound->play();
            coins.erase(it);
            return;
        }
    }
}

void Game::showDyingScreen()
{
    const float W  = static_cast<float>(WINDOW_WIDTH);
    const float H  = static_cast<float>(WINDOW_HEIGHT);
    const float t  = dyingClock.getElapsedTime().asSeconds();
    const auto  fadeIn = static_cast<std::uint8_t>(std::min(t / 0.5f, 1.f) * 180.f);

    window.draw(*bgSprite1);
    window.draw(*bgSprite2);
    for (auto& c : coins)     c->draw(window);
    for (auto& o : obstacles) o->draw(window);
    player.draw(window);

    sf::RectangleShape overlay({W, H});
    overlay.setFillColor(sf::Color(0, 0, 0, fadeIn));
    window.draw(overlay);

    if (deadSprite && fadeIn > 20) {
        deadSprite->setColor(sf::Color(255, 255, 255, fadeIn));
        window.draw(*deadSprite);
    }
}

void Game::showGameOverScreen()
{
    const float W  = static_cast<float>(WINDOW_WIDTH);
    const float H  = static_cast<float>(WINDOW_HEIGHT);
    const float cx = W / 2.f;
    const float cy = H / 2.f;
    const float t  = endScreenClock.getElapsedTime().asSeconds();

    window.draw(*bgSprite1);
    window.draw(*bgSprite2);

    sf::RectangleShape overlay({W, H});
    overlay.setFillColor(sf::Color(30, 0, 0, 200));
    window.draw(overlay);

    sf::RectangleShape card({460.f, 270.f});
    card.setFillColor(sf::Color(25, 5, 5, 230));
    card.setOutlineColor(sf::Color(200, 40, 40));
    card.setOutlineThickness(2.f + 1.5f * std::sin(t * 3.f));
    card.setOrigin({230.f, 135.f});
    card.setPosition({cx, cy});
    window.draw(card);

    const float scale = 1.f + 0.04f * std::sin(t * 4.f);
    sf::Text title(font, "GAME OVER", 46);
    title.setFillColor(sf::Color(255, 50, 50));
    title.setStyle(sf::Text::Bold);
    title.setScale({scale, scale});
    centerText(title, cx, cy - 80.f);
    window.draw(title);

    sf::Text sub(font, "Vous n'avez pas atteint le bunker...", 17);
    sub.setFillColor(sf::Color(200, 130, 130));
    centerText(sub, cx, cy - 28.f);
    window.draw(sub);

    sf::RectangleShape sep({340.f, 1.f});
    sep.setFillColor(sf::Color(200, 40, 40, 150));
    sep.setOrigin({170.f, 0.5f});
    sep.setPosition({cx, cy - 5.f});
    window.draw(sep);

    sf::Text statCoins(font, "Pieces collectees   " + std::to_string(coinScore), 19);
    statCoins.setFillColor(sf::Color(255, 220, 80));
    centerText(statCoins, cx, cy + 28.f);
    window.draw(statCoins);

    sf::Text hint(font, "[ Entree ]   Retour au menu", 16);
    hint.setFillColor(sf::Color(200, 160, 160,
        static_cast<std::uint8_t>(128.f + 120.f * std::sin(t * 2.f))));
    centerText(hint, cx, cy + 90.f);
    window.draw(hint);
}

void Game::showAboutScreen()
{
    sf::RenderWindow aboutWin(sf::VideoMode({ 560u, 340u }), "A propos");
    sf::Font f;
    if (!f.openFromFile("assets/fonts/font.ttf")) return;

    sf::Text t(f,
        "Forest Runner\n\n"
        "Objectif : courir et atteindre le bunker avant la fin du compte a rebours.\n\n\n"
        
        "Commandes :\n\n"
        "  ESPACE / Haut  :  Sauter\n"
        "  Bas            :  Se baisser\n"
        "  Echap          :  Menu\n\n\n"
        "Evitez les obstacles, collectez les pieces !\n"
        "Un choc = mort", 16);
    t.setFillColor(sf::Color::White);
    t.setPosition({30.f, 20.f});

    while (aboutWin.isOpen()) {
        while (const std::optional<sf::Event> ev = aboutWin.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) aboutWin.close();
            if (const auto* k = ev->getIf<sf::Event::KeyPressed>())
                if (k->code == sf::Keyboard::Key::Escape) aboutWin.close();
        }
        aboutWin.clear(sf::Color(15, 15, 25));
        aboutWin.draw(t);
        aboutWin.display();
    }
}