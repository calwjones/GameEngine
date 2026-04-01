#pragma once
#include "Entity.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

class EntityManager {
    std::vector<Entity*> m_entities;
    std::unordered_map<std::string, std::vector<Entity*>> m_byType;
    std::vector<Entity*> m_pendingAdd, m_pendingRemove;
    bool m_updating = false;

public:
    ~EntityManager() { clear(); }

    void addEntity(Entity* e);
    void removeEntity(Entity* e);
    bool detachEntity(Entity* e);   // remove without deleting

    template <typename T, typename... Args>
    T* spawn(Args&&... args) {
        auto* e = new T(std::forward<Args>(args)...);
        addEntity(e);
        return e;
    }

    Entity* getEntityByName(const std::string& name);

    const std::vector<Entity*>& getEntitiesByType(const std::string& type);

    std::vector<Entity*>& getAllEntities() { return m_entities; }
    const std::vector<Entity*>& getAllEntities() const { return m_entities; }
    size_t getEntityCount() const { return m_entities.size(); }

    void moveEntity(Entity* e, int direction);

    void updateAll(float dt);
    void renderAll(Renderer& renderer);
    void clear();

private:
    void processPending();
    void indexAdd(Entity* e);
    void indexRemove(Entity* e);
};

}
