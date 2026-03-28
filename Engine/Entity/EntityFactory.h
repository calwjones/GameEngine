#pragma once
#include "Entity.h"
#include <string>
#include <functional>
#include <unordered_map>

namespace Engine {

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
        // fall back so old levels still load
        auto* e = new Entity();
        e->type = type;
        return e;
    }

    bool hasType(const std::string& type) const {
        return m_creators.find(type) != m_creators.end();
    }
};

}
