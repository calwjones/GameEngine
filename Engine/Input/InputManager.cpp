#include "InputManager.h"

namespace Engine {

void InputManager::update() {
    m_prevKeys = m_keys;
    m_prevMouse = m_mouse;
    for (int i = 0; i < sf::Keyboard::KeyCount; i++)
        m_keys[i] = sf::Keyboard::isKeyPressed((sf::Keyboard::Key)i);
    for (int i = 0; i < sf::Mouse::ButtonCount; i++)
        m_mouse[i] = sf::Mouse::isButtonPressed((sf::Mouse::Button)i);
    m_mousePos = sf::Mouse::getPosition();
}

void InputManager::reset() {
    m_keys.fill(false);
    m_prevKeys.fill(false);
    m_mouse.fill(false);
    m_prevMouse.fill(false);
}

}
