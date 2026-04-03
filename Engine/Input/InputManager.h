#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>

namespace Engine {

class InputManager {
    std::array<bool, sf::Keyboard::KeyCount> m_keys{}, m_prevKeys{};
    std::array<bool, sf::Mouse::ButtonCount> m_mouse{}, m_prevMouse{};
    sf::Vector2i m_mousePos;

public:
    void update();

    bool isKeyPressed(sf::Keyboard::Key k) const { return m_keys[k]; }
    bool isKeyJustPressed(sf::Keyboard::Key k) const { return m_keys[k] && !m_prevKeys[k]; }
    bool isKeyJustReleased(sf::Keyboard::Key k) const { return !m_keys[k] && m_prevKeys[k]; }

    bool isMousePressed(sf::Mouse::Button b) const { return sf::Mouse::isButtonPressed(b); }
    bool isMouseJustPressed(sf::Mouse::Button b) const { return m_mouse[b] && !m_prevMouse[b]; }
    bool isMouseJustReleased(sf::Mouse::Button b) const { return !m_mouse[b] && m_prevMouse[b]; }

    sf::Vector2i getMousePosition() const { return m_mousePos; }

    void reset();
};

}
