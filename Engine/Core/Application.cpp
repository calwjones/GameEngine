#include "Application.h"
#include <SFML/Graphics/RectangleShape.hpp>

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
    target.clear(sf::Color(40, 44, 52));
    for (auto* e : m_entities.getAllEntities()) {
        sf::RectangleShape shape(e->size);
        shape.setPosition(e->position);
        shape.setFillColor(e->color);
        target.draw(shape);
    }
}

void Application::shutdown() {
    m_entities.clear();
    m_window = nullptr;
    m_initialized = false;
}

}
