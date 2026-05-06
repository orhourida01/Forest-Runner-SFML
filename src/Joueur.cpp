#include "Joueur.hpp"
#include <iostream>

// Tuned for a smooth, forgiving jump arc
static constexpr float GRAVITY   = 1900.f;
static constexpr float JUMP_VELO = -750.f;

Player::Player()
    : velocityY(0.f)
    , onGround(true)
    , crouching(false)
    , state(PlayerState::ALIVE)
    , currentTexture(TextureState::RUNNING)
{
    if (!textureRun.loadFromFile("assets/images/player.png"))       std::cerr << "Error: player.png\n";
    if (!textureJump.loadFromFile("assets/images/player_jump.png")) std::cerr << "Error: player_jump.png\n";
    if (!textureDuck.loadFromFile("assets/images/player_duck.png")) std::cerr << "Error: player_duck.png\n";

    if (jumpBuffer.loadFromFile("assets/sounds/jump.wav"))
        jumpSound.emplace(jumpBuffer);

    sprite.emplace(textureRun);
    sprite->setScale({0.2f, 0.2f});
    sf::FloatRect b = sprite->getLocalBounds();
    sprite->setOrigin({b.size.x / 2.f, b.size.y});
    sprite->setPosition({PLAYER_START_X, GROUND_LEVEL});
}

void Player::update(float dt)
{
    if (state == PlayerState::DEAD) return;

    if (!onGround) {
        velocityY += GRAVITY * dt;
        sprite->move({0.f, velocityY * dt});
        if (sprite->getPosition().y >= GROUND_LEVEL) {
            velocityY = 0.f;
            onGround  = true;
        }
    }

    if (onGround)
        sprite->setPosition({PLAYER_START_X, GROUND_LEVEL});

    // Swap texture only when visual state changes (avoids redundant GPU calls per frame)
    const TextureState wanted = !onGround  ? TextureState::JUMPING
                              : crouching  ? TextureState::CROUCHING
                                           : TextureState::RUNNING;
    if (wanted != currentTexture) {
        currentTexture = wanted;
        const sf::Texture& tex = (wanted == TextureState::JUMPING)   ? textureJump
                               : (wanted == TextureState::CROUCHING) ? textureDuck
                                                                      : textureRun;
        sprite->setTexture(tex, true);
        sf::FloatRect b = sprite->getLocalBounds();
        sprite->setOrigin({b.size.x / 2.f, b.size.y});
    }
}

void Player::reset()
{
    velocityY      = 0.f;
    onGround       = true;
    crouching      = false;
    state          = PlayerState::ALIVE;
    currentTexture = TextureState::RUNNING;
    // Re-bind sprite to our own textureRun (safe: both live in the same Player object)
    sprite->setTexture(textureRun, true);
    sf::FloatRect b = sprite->getLocalBounds();
    sprite->setOrigin({b.size.x / 2.f, b.size.y});
    sprite->setPosition({PLAYER_START_X, GROUND_LEVEL});
}

void Player::jump()
{
    if (onGround && state != PlayerState::DEAD) {
        velocityY = JUMP_VELO;
        onGround  = false;
        if (jumpSound) jumpSound->play();
    }
}

sf::FloatRect Player::getBounds() const
{
    sf::FloatRect r = sprite->getGlobalBounds();
    // Shrink hitbox so near-misses feel fair (permissive collision)
    const float hShrink = r.size.x * HITBOX_H_SHRINK;
    const float topGap  = r.size.y * HITBOX_V_GAP;
    return sf::FloatRect(
        {r.position.x + hShrink, r.position.y + topGap},
        {r.size.x - 2.f * hShrink, r.size.y - topGap}
    );
}

void Player::crouch(bool active) { if (state != PlayerState::DEAD) crouching = active; }
void Player::hurt()              { state = PlayerState::DEAD; }
bool Player::isDead() const      { return state == PlayerState::DEAD; }
void Player::draw(sf::RenderWindow& window) const { if (sprite) window.draw(*sprite); }
