#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

namespace Game {
class Player;
class Projectile;
}

namespace Engine {

class Application;
class AudioManager;
class Camera;
class Entity;

class GameplaySystem {
public:
    struct Config {
        Application* app = nullptr;
        AudioManager* audio = nullptr;
        Camera* camera = nullptr;
        float levelWidth = 800.f;
        float levelHeight = 600.f;
        float respawnYMargin = 100.f;
    };

    void begin(const Config& cfg);
    void tick(float fixedDt);
    void end();

    Entity* player() const { return m_player; }
    const sf::Vector2f& playerSpawnPos() const { return m_playerSpawnPos; }
    int score() const { return m_score; }
    float playTime() const { return m_playTime; }
    bool won() const { return m_won; }
    const std::string& nextLevel() const { return m_nextLevel; }

    // read-and-reset
    int consumeDeaths() { int n = m_deathsThisTick; m_deathsThisTick = 0; return n; }

    std::vector<Entity*>& collectedEntities() { return m_collected; }
    std::vector<Entity*>& defeatedEntities() { return m_defeated; }

private:
    Config m_cfg;
    Entity* m_player = nullptr;
    Game::Player* m_playerTyped = nullptr;
    sf::Vector2f m_playerSpawnPos{100.f, 400.f};
    int m_score = 0;
    float m_playTime = 0.f;
    bool m_won = false;
    std::string m_nextLevel;

    std::vector<Game::Projectile*> m_projectiles;
    std::vector<Entity*> m_collected;
    std::vector<Entity*> m_defeated;

    bool m_playerWasOnGround = false;
    int m_deathsThisTick = 0;
};

}
