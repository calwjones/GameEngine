#pragma once
#include <functional>

namespace Engine {

enum class GameState { MENU, PLAYING, PAUSED, EDITOR };

class StateManager {
public:
    using Callback = std::function<void(GameState, GameState)>;

private:
    GameState m_state = GameState::MENU;
    GameState m_prev = GameState::MENU;
    Callback m_onChange;

public:
    GameState getState() const { return m_state; }
    GameState getPrevious() const { return m_prev; }

    void setState(GameState s);
    void setOnChange(Callback cb) { m_onChange = std::move(cb); }

    bool isPlaying() const { return m_state == GameState::PLAYING; }
    bool isPaused() const { return m_state == GameState::PAUSED; }
    bool isMenu() const { return m_state == GameState::MENU; }

    void togglePause();

    static const char* toString(GameState s);
};

}
