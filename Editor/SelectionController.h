#pragma once
#include <vector>

namespace Engine { class Entity; }

namespace Editor {

struct EditorContext;

class SelectionController {
    EditorContext& ctx;
    std::vector<Engine::Entity*> m_selection;
    Engine::Entity* m_selected = nullptr;
    Engine::Entity* m_player = nullptr;

public:
    explicit SelectionController(EditorContext& c) : ctx(c) {}

    void select(Engine::Entity* e);
    void toggle(Engine::Entity* e);
    void clear();
    void remove(Engine::Entity* e);
    bool contains(Engine::Entity* e) const;
    void validate();
    void findPlayer();

    const std::vector<Engine::Entity*>& all() const { return m_selection; }
    std::vector<Engine::Entity*>& mutableAll() { return m_selection; }
    Engine::Entity* current() const { return m_selected; }
    void setCurrent(Engine::Entity* e) { m_selected = e; }
    Engine::Entity* player() const { return m_player; }
    void setPlayer(Engine::Entity* e) { m_player = e; }
};

}
