#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>

namespace Engine {

// polls SFML every frame. nothing fancy.
// why two arrays (m_keys + m_prevKeys)? so we can tell "pressed THIS frame
// but not last frame" = "just pressed". otherwise holding space = jump 60x/sec
class InputManager {
    std::array<bool, sf::Keyboard::KeyCount> m_keys{}, m_prevKeys{};
    std::array<bool, sf::Mouse::ButtonCount> m_mouse{}, m_prevMouse{};
    sf::Vector2i m_mousePos;

public:
    // called once per frame. copy current state to prev, then re-poll from SFML
    void update() {
        m_prevKeys = m_keys;
        m_prevMouse = m_mouse;
        for (int i = 0; i < sf::Keyboard::KeyCount; i++)
            m_keys[i] = sf::Keyboard::isKeyPressed((sf::Keyboard::Key)i);
        for (int i = 0; i < sf::Mouse::ButtonCount; i++)
            m_mouse[i] = sf::Mouse::isButtonPressed((sf::Mouse::Button)i);
        m_mousePos = sf::Mouse::getPosition();
    }

    bool isKeyPressed(sf::Keyboard::Key k) const { return m_keys[k]; }
    // edge detect — true for exactly 1 frame when the state flips
    bool isKeyJustPressed(sf::Keyboard::Key k) const { return m_keys[k] && !m_prevKeys[k]; }
    bool isKeyJustReleased(sf::Keyboard::Key k) const { return !m_keys[k] && m_prevKeys[k]; }

    bool isMousePressed(sf::Mouse::Button b) const { return sf::Mouse::isButtonPressed(b); }
    bool isMouseJustPressed(sf::Mouse::Button b) const { return m_mouse[b] && !m_prevMouse[b]; }
    bool isMouseJustReleased(sf::Mouse::Button b) const { return !m_mouse[b] && m_prevMouse[b]; }

    sf::Vector2i getMousePosition() const { return m_mousePos; }

    void reset() { m_keys.fill(false); m_prevKeys.fill(false); m_mouse.fill(false); m_prevMouse.fill(false); }
};

}
