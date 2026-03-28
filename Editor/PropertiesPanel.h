#pragma once
#include "../Engine/Entity/Entity.h"
#include "CommandHistory.h"

namespace Editor {

// drags batched into one undo step via IsItemActivated/Deactivated
class PropertiesPanel {
    bool m_visible = true;
    Engine::Entity* m_lastEntity = nullptr;
    EntityState m_editSnapshot;
    bool m_editing = false;

public:
    void render(Engine::Entity* e, CommandHistory* history = nullptr);
    bool& isVisible() { return m_visible; }

private:
    void checkUndo(Engine::Entity* e, CommandHistory* history);
};

}
