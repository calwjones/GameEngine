#pragma once
#include "../Engine/Entity/EntityManager.h"
#include <vector>

namespace Editor {

// left sidebar — entity list, filter, context menu. mutations dont happen here, menu items just set a pending request that EditorApplication polls + routes thru the undo stack
class ScenePanel {
    bool m_visible = true;
    std::string m_filter;

    // pending requests, consumed by EditorApplication each frame
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

    // [[nodiscard]] so the compiler yells if someone forgets to check — silently dropping a consumed request = ghost click
    [[nodiscard]] Engine::Entity* consumeDeleteRequest();
    [[nodiscard]] Engine::Entity* consumeDuplicateRequest();
    [[nodiscard]] bool consumeAddRequest();
    [[nodiscard]] Engine::Entity* consumeMoveUpRequest();
    [[nodiscard]] Engine::Entity* consumeMoveDownRequest();
};

}
