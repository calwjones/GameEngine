#pragma once

namespace Editor {

enum class EntityTemplate { None, Player, Platform, LargePlatform, Ground, Wall, Enemy,
                            FlyingEnemy, ShootingEnemy, Collectible, MovingPlatform, Goal, Hazard };

class EntityPalette {
    bool m_visible = true;
    EntityTemplate m_createRequest = EntityTemplate::None;

public:
    void render();
    bool& isVisible() { return m_visible; }
    [[nodiscard]] EntityTemplate consumeCreateRequest();
};

}
