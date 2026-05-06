#include "Menu.hpp"
#include <cmath>
#include <stdexcept>

// ── Palette ────────────────────────────────────────────────────────────────
static const sf::Color COL_SELECTED   = sf::Color(255, 210, 50);
static const sf::Color COL_UNSELECTED = sf::Color(190, 190, 200);
static const sf::Color COL_OVERLAY    = sf::Color(8, 8, 18, 185);

static void centerText(sf::Text& txt, float x, float y)
{
    const sf::FloatRect r = txt.getLocalBounds();
    txt.setOrigin({r.position.x + r.size.x / 2.f, r.position.y + r.size.y / 2.f});
    txt.setPosition({x, y});
}

Menu::Menu(float w, float h) : width(w), height(h)
{
    if (!font.openFromFile("assets/fonts/font.ttf"))
        throw std::runtime_error("Cannot load font: assets/fonts/font.ttf");

    // Title
    titleText.emplace(font, "Forest Runner", 54u);
    titleText->setStyle(sf::Text::Bold);

    // Subtitle
    subtitleText.emplace(font, "Survivez jusqu'au bunker !", 17u);
    subtitleText->setFillColor(sf::Color(170, 195, 170));
    centerText(*subtitleText, w / 2.f, h * 0.34f);

    // Menu items (French labels kept)
    const std::vector<std::string> labels = { "Jouer", "A propos", "Quitter" };
    const float startY = h * 0.53f;
    const float stepY  = 58.f;

    for (std::size_t i = 0; i < labels.size(); ++i) {
        sf::Text item(font, labels[i], 28u);
        item.setFillColor(i == 0 ? COL_SELECTED : COL_UNSELECTED);
        centerText(item, w / 2.f, startY + static_cast<float>(i) * stepY);
        items.push_back(std::move(item));
    }
}

void Menu::moveUp()
{
    if (selectedIndex > 0) {
        items[static_cast<std::size_t>(selectedIndex)].setFillColor(COL_UNSELECTED);
        --selectedIndex;
        items[static_cast<std::size_t>(selectedIndex)].setFillColor(COL_SELECTED);
    }
}

void Menu::moveDown()
{
    if (selectedIndex < static_cast<int>(items.size()) - 1) {
        items[static_cast<std::size_t>(selectedIndex)].setFillColor(COL_UNSELECTED);
        ++selectedIndex;
        items[static_cast<std::size_t>(selectedIndex)].setFillColor(COL_SELECTED);
    }
}

int Menu::handleEvent(sf::RenderWindow& window, const sf::Event& event)
{
    if (const auto* k = event.getIf<sf::Event::KeyPressed>()) {
        if (k->code == sf::Keyboard::Key::Up)    { moveUp();   return -2; }
        if (k->code == sf::Keyboard::Key::Down)  { moveDown(); return -2; }
        if (k->code == sf::Keyboard::Key::Enter) { return selectedIndex; }
    }

    // mapPixelToCoords convertit les coordonnées pixel brutes en coordonnées
    // logiques de la vue courante — indispensable en plein écran où le ratio
    // résolution physique / taille de vue peut différer de 1:1.
    if (const auto* m = event.getIf<sf::Event::MouseMoved>()) {
        const sf::Vector2f pos = window.mapPixelToCoords(m->position);
        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            if (items[static_cast<std::size_t>(i)].getGlobalBounds().contains(pos) &&
                i != selectedIndex) {
                items[static_cast<std::size_t>(selectedIndex)].setFillColor(COL_UNSELECTED);
                selectedIndex = i;
                items[static_cast<std::size_t>(selectedIndex)].setFillColor(COL_SELECTED);
                return -2;
            }
        }
    }

    if (const auto* m = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (m->button == sf::Mouse::Button::Left) {
            const sf::Vector2f pos = window.mapPixelToCoords(m->position);
            for (int i = 0; i < static_cast<int>(items.size()); ++i) {
                if (items[static_cast<std::size_t>(i)].getGlobalBounds().contains(pos))
                    return i;
            }
        }
    }

    return -1;
}

void Menu::draw(sf::RenderWindow& window)
{
    const float t  = animClock.getElapsedTime().asSeconds();
    const float cx = width / 2.f;

    // ── Semi-transparent dark overlay (background game scene already behind) ──
    sf::RectangleShape overlay({width, height});
    overlay.setFillColor(COL_OVERLAY);
    window.draw(overlay);

    // ── Ambient floating particles ────────────────────────────────────────────
    for (int i = 0; i < 12; ++i) {
        const float fi    = static_cast<float>(i);
        const float phase = fi * 0.85f;
        const float px    = width  * (fi / 11.f);
        const float py    = std::fmod(height * (fi / 11.f) + t * (18.f + (i % 4) * 7.f), height);
        const float rad   = 2.f + 1.5f * std::sin(t * 2.f + phase);
        const auto  alpha = static_cast<std::uint8_t>(40.f + 50.f * std::sin(t * 1.5f + phase));
        sf::CircleShape p(rad);
        p.setFillColor(sf::Color(255, 210, 80, alpha));
        p.setOrigin({rad, rad});
        p.setPosition({px, py});
        window.draw(p);
    }

    // ── Title "BUNKER RUSH" with animated glow ────────────────────────────────
    const float glow  = (1.f + std::sin(t * 1.8f)) / 2.f;
    const auto  titleR = static_cast<std::uint8_t>(230 + static_cast<int>(glow * 25.f));
    const auto  titleG = static_cast<std::uint8_t>(60  + static_cast<int>(glow * 100.f));
    titleText->setFillColor(sf::Color(titleR, titleG, 30));

    const float titleScale = 1.f + 0.025f * std::sin(t * 2.f);
    titleText->setScale({titleScale, titleScale});
    centerText(*titleText, cx, height * 0.22f);
    window.draw(*titleText);

    // Gold separator under title
    sf::RectangleShape sep1({340.f, 2.f});
    sep1.setFillColor(sf::Color(255, 200, 50, 200));
    sep1.setOrigin({170.f, 1.f});
    sep1.setPosition({cx, height * 0.22f + 46.f});
    window.draw(sep1);

    // Subtitle
    window.draw(*subtitleText);

    // ── Menu items ─────────────────────────────────────────────────────────────
    const float startY  = height * 0.53f;
    const float stepY   = 58.f;
    const float boxW    = 240.f;
    const float boxH    = 44.f;

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const float itemY = startY + static_cast<float>(i) * stepY;

        if (i == selectedIndex) {
            // Glowing selection box
            const float borderPulse = 1.5f + 1.f * std::sin(t * 4.f);
            sf::RectangleShape box({boxW, boxH});
            box.setFillColor(sf::Color(255, 210, 50, 25));
            box.setOutlineColor(sf::Color(255, 210, 50,
                static_cast<std::uint8_t>(180.f + 75.f * std::sin(t * 4.f))));
            box.setOutlineThickness(borderPulse);
            box.setOrigin({boxW / 2.f, boxH / 2.f});
            box.setPosition({cx, itemY});
            window.draw(box);

            // Arrow indicator ▶
            sf::Text arrow(font, ">", 22u);
            arrow.setFillColor(sf::Color(255, 210, 50));
            arrow.setPosition({cx - boxW / 2.f + 10.f, itemY - 11.f});
            window.draw(arrow);
        }

        window.draw(items[static_cast<std::size_t>(i)]);
    }

    // Separator above hint
    sf::RectangleShape sep2({340.f, 1.f});
    sep2.setFillColor(sf::Color(255, 255, 255, 40));
    sep2.setOrigin({170.f, 0.5f});
    sep2.setPosition({cx, height * 0.88f});
    window.draw(sep2);

    // Hint line
    const auto hintAlpha = static_cast<std::uint8_t>(100.f + 80.f * std::sin(t * 1.5f));
    sf::Text hint(font, "Haut/Bas  ou  Souris     Entree pour valider", 13u);
    hint.setFillColor(sf::Color(180, 180, 190, hintAlpha));
    centerText(hint, cx, height * 0.93f);
    window.draw(hint);
}
