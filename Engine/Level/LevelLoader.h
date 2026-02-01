#pragma once
#include "../Entity/Entity.h"
#include "../Entity/EntityFactory.h"
#include <vector>
#include <string>

namespace Engine {

// JSON level i/o. uses RapidJSON (header-only, in external/).
//
// THE SCHEMA has 2 layers — base + properties:
//   base:       position, size, velocity, color, isStatic, hasGravity, etc
//               (anything defined on the Entity class itself)
//   properties: subclass-specific stuff like patrolSpeed, fireRate, nextLevel
//               (serializeProperties()/serializeStringProperties() return these)
// saver merges both into one JSON object per entity, loader splits em back out.
// having ONE object instead of nested types keeps the JSON flat + greppable.
//
// IMPORTANT: setFactory() must be called first. otherwise unknown types fall
// back to base Entity and u lose every subclass's behaviour silently
class LevelLoader {
    std::string m_error;
    EntityFactory* m_factory = nullptr;
    float m_width = 800.f;
    float m_height = 600.f;

public:
    void setFactory(EntityFactory* factory) { m_factory = factory; }
    std::vector<Entity*> loadFromJSON(const std::string& path);
    bool saveToJSON(const std::string& path, const std::vector<Entity*>& entities, float width = 800.f, float height = 600.f);
    const std::string& getLastError() const { return m_error; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
};

}
