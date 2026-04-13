#pragma once
#include <string>
#include <vector>

namespace Editor {

enum class EntityTemplate { None, Player, Platform, LargePlatform, Ground, Wall, Enemy,
                            FlyingEnemy, ShootingEnemy, Collectible, MovingPlatform, Goal, Hazard };

class EntityPalette {
    bool m_visible = true;
    EntityTemplate m_createRequest = EntityTemplate::None;

    std::vector<std::string> m_prefabFiles;
    std::string m_prefabInstantiateRequest;
    std::string m_prefabSaveName;
    bool m_prefabSaveRequested = false;

public:
    void render();
    bool& isVisible() { return m_visible; }
    [[nodiscard]] EntityTemplate consumeCreateRequest();

    void scanPrefabs();
    [[nodiscard]] std::string consumePrefabInstantiateRequest();
    [[nodiscard]] bool consumePrefabSaveRequest(std::string& outName);
};

}
