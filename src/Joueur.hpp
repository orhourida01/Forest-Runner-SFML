#ifndef JOUEUR_HPP
#define JOUEUR_HPP

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <optional>
#include "Constants.hpp"
#include "Entity.hpp"

enum class PlayerState  { ALIVE, DEAD };
enum class TextureState { RUNNING, JUMPING, CROUCHING };

class Player : public Entity {
public:
    Player();

    // Réinitialise l'état sur place (évite le pointeur dangling du move-assignment)
    void reset();

    // Contrôles
    void jump();
    void crouch(bool active);
    void hurt();
    bool isDead() const;

    // --- Interface Entity ---
    void          update(float dt)                   override;
    void          draw(sf::RenderWindow& window) const override;
    sf::FloatRect getBounds()                  const override;
    bool          isOffScreen()                const override { return false; }

private:
    std::optional<sf::Sprite> sprite;
    sf::Texture               textureRun, textureJump, textureDuck;
    sf::SoundBuffer           jumpBuffer;
    std::optional<sf::Sound>  jumpSound;

    float        velocityY;
    bool         onGround;
    bool         crouching;
    PlayerState  state;
    TextureState currentTexture;
};

// Backwards-compat alias so existing includes still compile during transition
using Joueur = Player;

#endif
