#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <SFML/Graphics.hpp>

// Classe de base abstraite pour toutes les entités du jeu.
// Définit l'interface commune : mise à jour, rendu, collision, hors-écran.
class Entity {
public:
    virtual ~Entity() = default;

    // Mise à jour physique/logique (dt = temps écoulé en secondes)
    virtual void update(float dt) = 0;

    // Dessin dans la fenêtre (entité non modifiée par le rendu)
    virtual void draw(sf::RenderWindow& window) const = 0;

    // Boîte de collision (hitbox)
    virtual sf::FloatRect getBounds() const = 0;

    // Vrai si l'entité est sortie de l'écran et peut être supprimée
    virtual bool isOffScreen() const = 0;
};

#endif
