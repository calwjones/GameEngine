#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>

namespace sf {
class Font;
class Texture;
}

namespace Engine {

class Renderer {
    sf::RenderTarget* m_target;
    sf::RenderWindow* m_window;
    sf::View m_view;

public:
    explicit Renderer(sf::RenderWindow& w) : m_target(&w), m_window(&w), m_view(w.getDefaultView()) {}
    explicit Renderer(sf::RenderTexture& t) : m_target(&t), m_window(nullptr), m_view(t.getDefaultView()) {}

    void setTarget(sf::RenderTarget& t) { m_target = &t; }
    sf::RenderTarget* getTarget() { return m_target; }

    void clear(sf::Color c = sf::Color::Black);

    void drawRectangle(const sf::Vector2f& pos, const sf::Vector2f& size, sf::Color color);
    void drawSprite(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Texture& texture);
    void drawRectangleOutline(const sf::Vector2f& pos, const sf::Vector2f& size,
                              sf::Color fill, sf::Color outline, float thickness = 1.f);
    void drawText(const std::string& text, const sf::Vector2f& pos, sf::Font& font,
                  unsigned int sz = 24, sf::Color color = sf::Color::White);

    void display();

    void setView(const sf::View& v);
    const sf::View& getView() const { return m_view; }
    void resetView();
    sf::Vector2u getSize() const;
};

}
