#ifndef COIN_HPP
#define COIN_HPP

#include <SFML/Graphics.hpp>
#include "Entity.hpp"

// tex doit survivre à cet objet (pointe vers Game::coinTexture — adresse stable)
class Coin : public Entity {
public:
    Coin(const sf::Texture& tex, float x, float y);

    // Non copiable / non déplaçable
    Coin(const Coin&)            = delete;
    Coin& operator=(const Coin&) = delete;
    Coin(Coin&&)                 = delete;
    Coin& operator=(Coin&&)      = delete;

    // --- Interface Entity ---
    void          update(float dt)                  override;
    void          draw(sf::RenderWindow& w)   const override;
    sf::FloatRect getBounds()                 const override;
    bool          isOffScreen()               const override;

private:
    sf::Sprite sprite;
    float      speed;
    float      baseY;
    float      bobTimer;
};

#endif
