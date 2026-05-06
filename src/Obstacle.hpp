#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <SFML/Graphics.hpp>
#include <optional>
#include "Constants.hpp"
#include "Entity.hpp"

// Alloué sur le tas via unique_ptr dans Game :
// l'adresse mémoire est stable → pas besoin de move-constructor personnalisé.
class Obstacle : public Entity {
public:
    explicit Obstacle(float spawnX);

    // Non copiable / non déplaçable (unique_ptr gère la durée de vie)
    Obstacle(const Obstacle&)            = delete;
    Obstacle& operator=(const Obstacle&) = delete;
    Obstacle(Obstacle&&)                 = delete;
    Obstacle& operator=(Obstacle&&)      = delete;

    // --- Interface Entity ---
    void          update(float dt)                  override;
    void          draw(sf::RenderWindow& w)   const override;
    sf::FloatRect getBounds()                 const override;
    bool          isOffScreen()               const override;

private:
    sf::Texture               texture;
    std::optional<sf::Sprite> sprite;
    int                       type;
    float                     speed;
};

#endif
