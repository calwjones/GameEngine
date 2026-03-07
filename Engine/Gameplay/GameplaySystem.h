#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

namespace Engine {

class Application;
class AudioManager;
class Camera;
class Entity;

// ⭐ VIVA POINT ⭐ this class is "the game" when u press Play. it owns
// EVERYTHING that's transient during a play session: the player ref, spawn
// point, score, timer, win flag, live projectile list, + the "to re-spawn
// on stop" buckets for collectibles picked up / enemies killed.
//
// BEFORE this existed, all this logic was 300 lines of spaghetti inside
// EditorApplication::update(). extracted it into its own class so the editor
// just calls begin() -> tick() -> end() and doesnt know HOW a frame works.
//
// begin(cfg) — call once when entering play. stashes the config, finds player,
//   records spawn pos, wires up the camera
// tick(dt)   — called repeatedly by the fixed-timestep loop in EditorApplication.
//   does physics -> entity updates -> collision -> triggers -> camera follow
// end()      — call on stop. editor then drains collected/defeated + re-adds
//   them to the scene as part of the snapshot restore
class GameplaySystem {
public:
    // deps i need injected. passed in once via begin()
    struct Config {
        Application* app = nullptr;         // subsystem bag — entities, physics, collision
        AudioManager* audio = nullptr;      // optional — nullptr = silent build
        Camera* camera = nullptr;           // smooth follow
        float levelWidth = 800.f;
        float levelHeight = 600.f;
        float respawnYMargin = 100.f;       // how far below levelHeight before u count as fallen off
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

    // DEATHS THIS TICK. gets ++ every time the player respawns (fall / hazard /
    // enemy hit / projectile hit). editor reads this each frame via consumeDeaths()
    // to drive the world-mode lives counter. read-and-reset in one op so we
    // cant double count across frames. clean idiom stolen from ECS patterns
    int consumeDeaths() { int n = m_deathsThisTick; m_deathsThisTick = 0; return n; }

    // editor reaches into these on Stop to re-attach collected coins / killed
    // enemies so the snapshot restore is clean. separate vecs so the editor
    // knows which were coins vs which were enemies (diff handling)
    std::vector<Entity*>& collectedEntities() { return m_collected; }
    std::vector<Entity*>& defeatedEntities() { return m_defeated; }

private:
    Config m_cfg;
    Entity* m_player = nullptr;
    sf::Vector2f m_playerSpawnPos{100.f, 400.f};
    int m_score = 0;
    float m_playTime = 0.f;
    bool m_won = false;
    std::string m_nextLevel;

    std::vector<Entity*> m_projectiles;
    std::vector<Entity*> m_collected;
    std::vector<Entity*> m_defeated;

    bool m_playerWasOnGround = false;
    int m_deathsThisTick = 0;
};

}
