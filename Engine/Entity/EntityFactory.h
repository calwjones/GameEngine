#pragma once
#include "Entity.h"
#include <string>
#include <functional>
#include <unordered_map>

namespace Engine {

// classic factory pattern. JSON loader sees "type": "enemy" and needs to new
// up a Game::Enemy — but Engine/ cant depend on Game/ (layering), so we flip
// it: Game/ REGISTERS its constructors here at startup + loader looks em up.
//
// side effect: adding a new entity type = 1 line in EditorApplication::initialize()
// (the registerType call) + a subclass file. everything else (palette, scene
// panel, serialization) picks it up automatically via EntityTypeRegistry
class EntityFactory {
    std::unordered_map<std::string, std::function<Entity*()>> m_creators;

public:
    void registerType(const std::string& type, std::function<Entity*()> creator) {
        m_creators[type] = std::move(creator);
    }

    Entity* create(const std::string& type) const {
        auto it = m_creators.find(type);
        if (it != m_creators.end())
            return it->second();
        // unknown type — make a plain Entity so the level still loads.
        // means old JSON w/ deleted types still opens, just loses behaviour
        auto* e = new Entity();
        e->type = type;
        return e;
    }

    bool hasType(const std::string& type) const {
        return m_creators.find(type) != m_creators.end();
    }
};

}
