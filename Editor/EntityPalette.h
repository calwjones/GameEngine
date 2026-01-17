#pragma once

namespace Editor {

// prefabs available in the palette. None = no pending request
enum class EntityTemplate { None, Player, Platform, LargePlatform, Ground, Wall, Enemy,
                            FlyingEnemy, ShootingEnemy, Collectible, MovingPlatform, Goal, Hazard };

// palette panel — click sets m_createRequest, EditorApplication consumes it + spawns via addFromTemplate (where undo gets wired up)
class EntityPalette {
    bool m_visible = true;
    EntityTemplate m_createRequest = EntityTemplate::None;   // None = nothing pending

public:
    void render();
    bool& isVisible() { return m_visible; }
    [[nodiscard]] EntityTemplate consumeCreateRequest();
};

}
