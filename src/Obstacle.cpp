#include "Obstacle.hpp"
#include <iostream>
#include <cstdlib>

static const char* TEXTURE_PATHS[3] = {
    "assets/images/obstacle_ground.png",
    "assets/images/obstacle_low.png",
    "assets/images/obstacle_high.png"
};

// Alloué sur le tas via std::unique_ptr dans Game :
// l'adresse de l'objet est fixe → le sprite peut pointer vers notre propre texture en toute sécurité.
Obstacle::Obstacle(float spawnX)
    : type(std::rand() % 3)
    , speed(400.f)
{
    if (!texture.loadFromFile(TEXTURE_PATHS[type]))
        std::cerr << "Erreur : impossible de charger " << TEXTURE_PATHS[type] << "\n";

    texture.setSmooth(true);
    sprite.emplace(texture);
    sprite->setScale({0.15f, 0.15f});

    const sf::FloatRect b = sprite->getLocalBounds();
    sprite->setOrigin({b.size.x / 2.f, b.size.y});
    sprite->setPosition({spawnX, GROUND_LEVEL});
}

void Obstacle::update(float dt)
{
    if (sprite)
        sprite->move({-speed * dt, 0.f});
}

void Obstacle::draw(sf::RenderWindow& w) const
{
    if (sprite)
        w.draw(*sprite);
}

sf::FloatRect Obstacle::getBounds() const
{
    if (!sprite) return {};

    const sf::FloatRect r = sprite->getGlobalBounds();
    const float hShrink   = r.size.x * HITBOX_H_SHRINK;
    const float vGap      = r.size.y * HITBOX_V_GAP;

    return sf::FloatRect(
        {r.position.x + hShrink, r.position.y + vGap},
        {r.size.x - 2.f * hShrink, r.size.y - vGap}
    );
}

bool Obstacle::isOffScreen() const
{
    if (!sprite) return true;
    return sprite->getPosition().x < -100.f;
}
