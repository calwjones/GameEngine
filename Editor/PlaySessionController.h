#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include "../Engine/Gameplay/GameplaySystem.h"

namespace Engine { class Entity; }

namespace Editor {

struct EditorContext;

struct EntitySnapshot {
    Engine::Entity* entity;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool isOnGround;
};

struct GroupSession {
    bool active = false;
    int groupIndex = -1;
    int currentLevel = 0;
    int livesRemaining = 3;
    int totalCoins = 0;
};

enum class PlayOverlay {
    None,
    LevelComplete,
    WorldComplete,
    GameOver,
    SingleWin,
};

class PlaySessionController {
    EditorContext& ctx;

    Engine::GameplaySystem m_gameplay;
    std::vector<EntitySnapshot> m_playSnapshot;
    GroupSession m_groupSession;
    PlayOverlay m_playOverlay = PlayOverlay::None;
    sf::Vector2f m_savedViewOffset{0, 0};
    float m_savedViewZoom = 1.f;
    std::string m_pendingNextLevel;
    float m_accumulator = 0.f;
    int m_levelCoinsSnapshot = 0;

    static constexpr float RESPAWN_Y_MARGIN = 100.f;

public:
    explicit PlaySessionController(EditorContext& c) : ctx(c) {}

    void tick(float dt);

    void startPlaying();
    void stopPlaying();
    bool isPlaying() const;

    void startGroup(int groupIndex);
    void advanceGroupLevel();
    void restartGroup();
    void endGroupSession();

    void clearSnapshot() { m_playSnapshot.clear(); }

    PlayOverlay overlay() const { return m_playOverlay; }
    void setOverlay(PlayOverlay o) { m_playOverlay = o; }

    int score() const { return m_gameplay.score(); }
    int levelCoinsSnapshot() const { return m_levelCoinsSnapshot; }

    const GroupSession& groupSession() const { return m_groupSession; }
    GroupSession& mutableGroupSession() { return m_groupSession; }

    Engine::GameplaySystem& gameplay() { return m_gameplay; }
    const Engine::GameplaySystem& gameplay() const { return m_gameplay; }

private:
    void snapshotEntities();
    void restoreSnapshot();
};

}
