#include "Application.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

namespace Engine {

bool Application::initialize(sf::RenderWindow* window) {
    if (!window) return false;
    m_window = window;
    m_width = window->getSize().x;
    m_height = window->getSize().y;
    m_initialized = true;
    return true;
}

void Application::render(sf::RenderTarget& target) {
    if (!m_initialized) return;

    sf::Vector2f viewSize = target.getView().getSize();
    sf::Vector2f viewCenter = target.getView().getCenter();
    sf::Vector2f viewMin = viewCenter - viewSize * 0.5f;
    sf::Vector2f viewMax = viewCenter + viewSize * 0.5f;
    m_tiles.draw(target, viewMin, viewMax);

    for (auto* e : m_entities.getAllEntities()) {
        if (e->texture) {
            sf::Sprite sprite(*e->texture);
            sprite.setPosition(e->position);
            sf::Vector2u texSize = e->texture->getSize();
            if (texSize.x > 0 && texSize.y > 0)
                sprite.setScale(e->size.x / texSize.x, e->size.y / texSize.y);
            target.draw(sprite);
        } else {
            sf::RectangleShape shape(e->size);
            shape.setPosition(e->position);
            shape.setFillColor(e->color);
            target.draw(shape);
        }
    }
}

void Application::shutdown() {
    m_entities.clear();
    m_window = nullptr;
    m_initialized = false;
}

}
