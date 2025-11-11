#pragma once
#include "Entity.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

// owns every entity ptr. responsible for add/remove + the big per-frame update loop.
//
// THE TWO TRICKY BITS:
//
// 1) DEFERRED ADD/REMOVE. if an entity's update() adds/removes another entity
//    mid-loop (e.g. shooter spawns projectile, enemy dies to stomp), we cant
//    touch m_entities while we're iterating it — that invalidates iterators
//    and = crash. so mutations queue into m_pendingAdd/Remove and get flushed
//    AFTER the update loop in processPending(). classic pattern.
//
// 2) PER-TYPE INDEX. m_byType is m_entities bucketed by type string
//    ("enemy" -> [e1, e2, ...]). the editor's per-frame work needs to scan
//    "just the shooters" or "just the moving platforms" — without this index
//    we'd do dynamic_cast on every entity every frame which is slow + ugly.
//    with it, we get the bucket by key (O(1)) + static_cast the contents.
//    has to be kept in sync manually on every add/remove (indexAdd/indexRemove).
class EntityManager {
    std::vector<Entity*> m_entities;                                    // source of truth, owns the ptrs
    std::unordered_map<std::string, std::vector<Entity*>> m_byType;     // mirror, for fast type queries
    std::vector<Entity*> m_pendingAdd, m_pendingRemove;
    bool m_updating = false;   // if true, add/remove go to the pending queues

public:
    ~EntityManager() { clear(); }

    void addEntity(Entity* e);
    void removeEntity(Entity* e);
    bool detachEntity(Entity* e);  // remove WITHOUT deleting — undo needs to keep the ptr alive
    Entity* getEntityByName(const std::string& name);

    // returns ref to the type bucket (or an empty vector if none). safe to hold briefly
    const std::vector<Entity*>& getEntitiesByType(const std::string& type);

    std::vector<Entity*>& getAllEntities() { return m_entities; }
    const std::vector<Entity*>& getAllEntities() const { return m_entities; }
    size_t getEntityCount() const { return m_entities.size(); }

    // moves an entity up/down in the list — only affects draw order + scene panel display
    void moveEntity(Entity* e, int direction);

    void updateAll(float dt);
    void renderAll(Renderer& renderer);
    void clear();

private:
    void processPending();   // flush the deferred add/remove queues
    void indexAdd(Entity* e);
    void indexRemove(Entity* e);
};

}
