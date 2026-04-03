#include "Renderer.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

namespace Engine {

void Renderer::clear(sf::Color c) {
    if (m_target) m_target->clear(c);
}

void Renderer::drawRectangle(const sf::Vector2f& pos, const sf::Vector2f& size, sf::Color color) {
    if (!m_target) return;
    sf::RectangleShape s(size);
    s.setPosition(pos);
    s.setFillColor(color);
    m_target->draw(s);
}

void Renderer::drawSprite(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Texture& texture) {
    if (!m_target) return;
    sf::Sprite sprite(texture);
    sprite.setPosition(pos);
    sf::Vector2u texSize = texture.getSize();
    if (texSize.x > 0 && texSize.y > 0)
        sprite.setScale(size.x / texSize.x, size.y / texSize.y);
    m_target->draw(sprite);
}

void Renderer::drawRectangleOutline(const sf::Vector2f& pos, const sf::Vector2f& size,
                                    sf::Color fill, sf::Color outline, float thickness) {
    if (!m_target) return;
    sf::RectangleShape s(size);
    s.setPosition(pos);
    s.setFillColor(fill);
    s.setOutlineColor(outline);
    s.setOutlineThickness(thickness);
    m_target->draw(s);
}

void Renderer::drawText(const std::string& text, const sf::Vector2f& pos, sf::Font& font,
                        unsigned int sz, sf::Color color) {
    if (!m_target) return;
    sf::Text t(text, font, sz);
    t.setPosition(pos);
    t.setFillColor(color);
    m_target->draw(t);
}

void Renderer::display() {
    if (m_window) m_window->display();
}

void Renderer::setView(const sf::View& v) {
    m_view = v;
    if (m_target) m_target->setView(v);
}

void Renderer::resetView() {
    if (m_target) {
        m_view = m_target->getDefaultView();
        m_target->setView(m_view);
    }
}

sf::Vector2u Renderer::getSize() const {
    return m_target ? m_target->getSize() : sf::Vector2u(0, 0);
}

}
