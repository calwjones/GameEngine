#pragma once
#include "../Entity/Entity.h"
#include "../Entity/EntityFactory.h"
#include <vector>
#include <string>

namespace Engine {

class TileLayer;

class LevelLoader {
    std::string m_error;
    EntityFactory* m_factory = nullptr;
    float m_width = 800.f;
    float m_height = 600.f;

public:
    void setFactory(EntityFactory* factory) { m_factory = factory; }
    // outTiles is optional. If present, any "tiles" object in the JSON is written into it;
    // otherwise the tile data is silently skipped.
    std::vector<Entity*> loadFromJSON(const std::string& path, TileLayer* outTiles = nullptr);
    // tiles is optional. If non-null and non-empty, the layer is serialized under "tiles".
    bool saveToJSON(const std::string& path, const std::vector<Entity*>& entities,
                    float width = 800.f, float height = 600.f, const TileLayer* tiles = nullptr);
    const std::string& getLastError() const { return m_error; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
};

}
