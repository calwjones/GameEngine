#include "Application.h"
#include <SFML/Graphics/RectangleShape.hpp>

namespace Engine {

// just grabs a borrowed window pointer from the editor. no loop, no ticking
bool Application::initialize(sf::RenderWindow* window) {
    if (!window) return false;
    m_window = window;
    m_width = window->getSize().x;
    m_height = window->getSize().y;
    m_initialized = true;
    return true;
}

// fallback "debug" render — just draws every entity as a coloured rect.
// the editor's GameViewport actually has its own fancier render path that does
// textures + selection highlights. this one is unused rn tbh but kept as a
// sanity check / reference impl
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
    m_window = nullptr;   // borrowed ptr, DONT delete — editor owns it
    m_initialized = false;
}

}
