#include "Coin.hpp"
#include <cmath>
#include <algorithm>

static constexpr float DISPLAY_SIZE   = 40.f;
static constexpr float COIN_SPEED     = 320.f;
static constexpr float BOB_AMPLITUDE  = 7.f;
static constexpr float BOB_FREQUENCY  = 5.f;

Coin::Coin(const sf::Texture& tex, float x, float y)
    : sprite(tex)
    , speed(COIN_SPEED)
    , baseY(y)
    , bobTimer(0.f)
{
    const sf::Vector2u s = tex.getSize();
    const float scale = DISPLAY_SIZE / static_cast<float>(std::max(s.x, s.y));
    sprite.setScale({scale, scale});
    const sf::FloatRect b = sprite.getLocalBounds();
    sprite.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
    sprite.setPosition({x, y});
}

void Coin::update(float dt)
{
    bobTimer += dt;
    const float x = sprite.getPosition().x - speed * dt;
    sprite.setPosition({x, baseY + BOB_AMPLITUDE * std::sin(bobTimer * BOB_FREQUENCY)});
}

void Coin::draw(sf::RenderWindow& w) const { w.draw(sprite); }

sf::FloatRect Coin::getBounds() const { return sprite.getGlobalBounds(); }

bool Coin::isOffScreen() const { return sprite.getPosition().x < -60.f; }
