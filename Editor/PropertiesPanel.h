#pragma once
#include "../Engine/Entity/Entity.h"
#include "CommandHistory.h"

namespace Editor {

// right sidebar — editable fields for the selected entity. drag sliders are batched into a single undo step via IsItemActivated/Deactivated, one drag = one command
class PropertiesPanel {
    bool m_visible = true;
    Engine::Entity* m_lastEntity = nullptr;   // resets edit state if selection changes mid-drag
    EntityState m_editSnapshot;               // "before" state, grabbed on IsItemActivated
    bool m_editing = false;

public:
    void render(Engine::Entity* e, CommandHistory* history = nullptr);
    bool& isVisible() { return m_visible; }

private:
    void checkUndo(Engine::Entity* e, CommandHistory* history);   // call right after each imgui widget
};

}
