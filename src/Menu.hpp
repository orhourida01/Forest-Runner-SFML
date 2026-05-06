#ifndef MENU_HPP
#define MENU_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>

class Menu {
public:
    Menu(float width, float height);

    int  handleEvent(sf::RenderWindow& window, const sf::Event& event);
    void draw(sf::RenderWindow& window);

private:
    void moveUp();
    void moveDown();

    float width, height;

    sf::Font                  font;
    std::vector<sf::Text>     items;
    int                       selectedIndex{0};

    std::optional<sf::Text>   titleText;
    std::optional<sf::Text>   subtitleText;

    sf::Clock                 animClock;
};

#endif
