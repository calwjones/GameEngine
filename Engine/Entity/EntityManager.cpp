#include "EntityManager.h"
#include <algorithm>

namespace Engine {

void EntityManager::indexAdd(Entity* e) {
    m_byType[e->type].push_back(e);
}

void EntityManager::indexRemove(Entity* e) {
    auto it = m_byType.find(e->type);
    if (it == m_byType.end()) return;
    auto& bucket = it->second;
    bucket.erase(std::remove(bucket.begin(), bucket.end(), e), bucket.end());
    if (bucket.empty()) m_byType.erase(it);
}

void EntityManager::addEntity(Entity* e) {
    if (!e) return;
    if (m_updating) {
        m_pendingAdd.push_back(e);
    } else {
        m_entities.push_back(e);
        indexAdd(e);
    }
}

void EntityManager::removeEntity(Entity* e) {
    if (!e) return;
    if (m_updating) {
        m_pendingRemove.push_back(e);
    } else {
        auto it = std::find(m_entities.begin(), m_entities.end(), e);
        if (it != m_entities.end()) {
            indexRemove(*it);
            delete *it;
            m_entities.erase(it);
        }
    }
}

// detach keeps the ptr alive for undo stacks
bool EntityManager::detachEntity(Entity* e) {
    if (!e) return false;
    auto it = std::find(m_entities.begin(), m_entities.end(), e);
    if (it != m_entities.end()) {
        indexRemove(*it);
        m_entities.erase(it);
        return true;
    }
    return false;
}

Entity* EntityManager::getEntityByName(const std::string& name) {
    for (auto* e : m_entities)
        if (e->name == name) return e;
    return nullptr;
}

const std::vector<Entity*>& EntityManager::getEntitiesByType(const std::string& type) {
    static const std::vector<Entity*> empty;
    auto it = m_byType.find(type);
    return it == m_byType.end() ? empty : it->second;
}

void EntityManager::moveEntity(Entity* e, int direction) {
    auto it = std::find(m_entities.begin(), m_entities.end(), e);
    if (it == m_entities.end()) return;
    size_t idx = (size_t)(it - m_entities.begin());
    if (direction > 0 && idx + 1 < m_entities.size())
        std::swap(m_entities[idx], m_entities[idx + 1]);
    else if (direction < 0 && idx > 0)
        std::swap(m_entities[idx], m_entities[idx - 1]);
}

void EntityManager::updateAll(float dt) {
    m_updating = true;
    for (auto* e : m_entities) e->update(dt);
    m_updating = false;
    processPending();
}

// static first so dynamic entities draw on top
void EntityManager::renderAll(Renderer& renderer) {
    for (auto* e : m_entities) if (e->isStatic) e->render(renderer);
    for (auto* e : m_entities) if (!e->isStatic) e->render(renderer);
}

void EntityManager::clear() {
    for (auto* e : m_entities) delete e;
    for (auto* e : m_pendingAdd) delete e;
    m_entities.clear();
    m_byType.clear();
    m_pendingAdd.clear();
    m_pendingRemove.clear();
}

void EntityManager::processPending() {
    for (auto* e : m_pendingAdd) {
        m_entities.push_back(e);
        indexAdd(e);
    }
    m_pendingAdd.clear();

    for (auto* e : m_pendingRemove) {
        auto it = std::find(m_entities.begin(), m_entities.end(), e);
        if (it != m_entities.end()) {
            indexRemove(*it);
            delete *it;
            m_entities.erase(it);
        }
    }
    m_pendingRemove.clear();
}

}
