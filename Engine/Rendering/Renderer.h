#pragma once
#include <SFML/Graphics.hpp>

namespace Engine {

// thin helper over sf::RenderTarget. can point at either a window OR an
// offscreen texture — the editor uses the texture path bc ImGui draws the
// viewport as an image widget inside the dockspace. just wraps the common
// draw calls (rect/sprite/text) so callers dont re-type the boilerplate.
// also: ignore the "standalone mode" comment — theres no standalone mode
class Renderer {
    sf::RenderTarget* m_target;
    sf::RenderWindow* m_window;
    sf::View m_view;

public:
    explicit Renderer(sf::RenderWindow& w) : m_target(&w), m_window(&w), m_view(w.getDefaultView()) {}
    explicit Renderer(sf::RenderTexture& t) : m_target(&t), m_window(nullptr), m_view(t.getDefaultView()) {}

    void setTarget(sf::RenderTarget& t) { m_target = &t; }
    sf::RenderTarget* getTarget() { return m_target; }

    void clear(sf::Color c = sf::Color::Black) { if (m_target) m_target->clear(c); }

    void drawRectangle(const sf::Vector2f& pos, const sf::Vector2f& size, sf::Color color) {
        if (!m_target) return;
        sf::RectangleShape s(size);
        s.setPosition(pos);
        s.setFillColor(color);
        m_target->draw(s);
    }

    // stretches the tex to match the entity's box. not pixel-perfect but it
    // means every entity looks the same whatever size it is. texturePath is
    // now set via PropertiesPanel — no more manual JSON editing lol
    void drawSprite(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Texture& texture) {
        if (!m_target) return;
        sf::Sprite sprite(texture);
        sprite.setPosition(pos);
        sf::Vector2u texSize = texture.getSize();
        if (texSize.x > 0 && texSize.y > 0)
            sprite.setScale(size.x / texSize.x, size.y / texSize.y);
        m_target->draw(sprite);
    }

    void drawRectangleOutline(const sf::Vector2f& pos, const sf::Vector2f& size,
                              sf::Color fill, sf::Color outline, float thickness = 1.f) {
        if (!m_target) return;
        sf::RectangleShape s(size);
        s.setPosition(pos);
        s.setFillColor(fill);
        s.setOutlineColor(outline);
        s.setOutlineThickness(thickness);
        m_target->draw(s);
    }

    void drawText(const std::string& text, const sf::Vector2f& pos, sf::Font& font,
                  unsigned int sz = 24, sf::Color color = sf::Color::White) {
        if (!m_target) return;
        sf::Text t(text, font, sz);
        t.setPosition(pos);
        t.setFillColor(color);
        m_target->draw(t);
    }

    void display() { if (m_window) m_window->display(); }

    void setView(const sf::View& v) { m_view = v; if (m_target) m_target->setView(v); }
    const sf::View& getView() const { return m_view; }
    void resetView() { if (m_target) { m_view = m_target->getDefaultView(); m_target->setView(m_view); } }
    sf::Vector2u getSize() const { return m_target ? m_target->getSize() : sf::Vector2u(0, 0); }
};

}
