#include "SelectionController.h"
#include "EditorContext.h"
#include "../Engine/Entity/Entity.h"
#include "../Engine/Entity/EntityManager.h"
#include <algorithm>

namespace Editor {

void SelectionController::select(Engine::Entity* e) {
    m_selection.clear();
    if (e) m_selection.push_back(e);
    m_selected = e;
}

void SelectionController::toggle(Engine::Entity* e) {
    auto it = std::find(m_selection.begin(), m_selection.end(), e);
    if (it != m_selection.end()) {
        m_selection.erase(it);
        if (m_selected == e)
            m_selected = m_selection.empty() ? nullptr : m_selection.back();
    } else {
        m_selection.push_back(e);
        m_selected = e;
    }
}

void SelectionController::clear() {
    m_selection.clear();
    m_selected = nullptr;
}

void SelectionController::remove(Engine::Entity* e) {
    auto it = std::find(m_selection.begin(), m_selection.end(), e);
    if (it != m_selection.end()) m_selection.erase(it);
    if (m_selected == e)
        m_selected = m_selection.empty() ? nullptr : m_selection.back();
    if (m_player == e) m_player = nullptr;
}

bool SelectionController::contains(Engine::Entity* e) const {
    return std::find(m_selection.begin(), m_selection.end(), e) != m_selection.end();
}

void SelectionController::validate() {
    auto& ents = ctx.game.getEntityManager().getAllEntities();
    auto alive = [&](Engine::Entity* e) {
        return std::find(ents.begin(), ents.end(), e) != ents.end();
    };
    m_selection.erase(
        std::remove_if(m_selection.begin(), m_selection.end(),
                       [&](Engine::Entity* e) { return !alive(e); }),
        m_selection.end());
    if (m_selected && !alive(m_selected))
        m_selected = m_selection.empty() ? nullptr : m_selection.back();
    if (m_player && !alive(m_player))
        m_player = nullptr;
}

void SelectionController::findPlayer() {
    m_player = nullptr;
    for (auto* e : ctx.game.getEntityManager().getAllEntities()) {
        if (e->type == "player") { m_player = e; return; }
    }
}

}
