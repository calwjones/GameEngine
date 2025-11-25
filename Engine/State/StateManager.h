#pragma once
#include <functional>

namespace Engine {

// 4 values but tbh the editor only uses MENU/PLAYING/PAUSED — EDITOR is legacy
enum class GameState { MENU, PLAYING, PAUSED, EDITOR };

// just a typed enum wrapper with a "hey something changed" callback.
// i use the enum directly in EditorApplication (m_state) instead of owning
// one of these, so this class is kinda optional rn. kept for reference
class StateManager {
public:
    using Callback = std::function<void(GameState, GameState)>;   // (from, to)

private:
    GameState m_state = GameState::MENU;
    GameState m_prev = GameState::MENU;
    Callback m_onChange;

public:
    GameState getState() const { return m_state; }
    GameState getPrevious() const { return m_prev; }

    // only fires the callback if we actually transitioned to a new state
    void setState(GameState s) {
        if (s == m_state) return;
        m_prev = m_state;
        m_state = s;
        if (m_onChange) m_onChange(m_prev, m_state);
    }

    void setOnChange(Callback cb) { m_onChange = std::move(cb); }

    bool isPlaying() const { return m_state == GameState::PLAYING; }
    bool isPaused() const { return m_state == GameState::PAUSED; }
    bool isMenu() const { return m_state == GameState::MENU; }

    void togglePause() {
        if (m_state == GameState::PLAYING) setState(GameState::PAUSED);
        else if (m_state == GameState::PAUSED) setState(GameState::PLAYING);
    }

    static const char* toString(GameState s) {
        switch (s) {
            case GameState::MENU:    return "Menu";
            case GameState::PLAYING: return "Playing";
            case GameState::PAUSED:  return "Paused";
            case GameState::EDITOR:  return "Editor";
            default:                 return "Unknown";
        }
    }
};

}
