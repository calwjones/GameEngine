#pragma once
#include "../Engine/Entity/EntityManager.h"
#include <vector>

namespace Editor {

class ScenePanel {
    bool m_visible = true;
    std::string m_filter;

    Engine::Entity* m_deleteRequest = nullptr;
    Engine::Entity* m_duplicateRequest = nullptr;
    bool m_addRequest = false;
    Engine::Entity* m_moveUpRequest = nullptr;
    Engine::Entity* m_moveDownRequest = nullptr;

public:
    void render(Engine::EntityManager& mgr,
                std::vector<Engine::Entity*>& selection,
                Engine::Entity*& primary,
                bool playing = false);
    bool& isVisible() { return m_visible; }

    [[nodiscard]] Engine::Entity* consumeDeleteRequest();
    [[nodiscard]] Engine::Entity* consumeDuplicateRequest();
    [[nodiscard]] bool consumeAddRequest();
    [[nodiscard]] Engine::Entity* consumeMoveUpRequest();
    [[nodiscard]] Engine::Entity* consumeMoveDownRequest();
};

}
