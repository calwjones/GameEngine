#include "StateManager.h"

namespace Engine {

void StateManager::setState(GameState s) {
    if (s == m_state) return;
    m_prev = m_state;
    m_state = s;
    if (m_onChange) m_onChange(m_prev, m_state);
}

void StateManager::togglePause() {
    if (m_state == GameState::PLAYING) setState(GameState::PAUSED);
    else if (m_state == GameState::PAUSED) setState(GameState::PLAYING);
}

const char* StateManager::toString(GameState s) {
    switch (s) {
        case GameState::MENU:    return "Menu";
        case GameState::PLAYING: return "Playing";
        case GameState::PAUSED:  return "Paused";
        case GameState::EDITOR:  return "Editor";
    }
    return "Unknown";
}

}
