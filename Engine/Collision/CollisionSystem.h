#pragma once
#include "../Entity/Entity.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace Engine {

// AABB collision with a spatial hash broad phase
class CollisionSystem {
public:
    enum class Side { NONE, TOP, BOTTOM, LEFT, RIGHT };

    struct Result {
        bool collided = false;
        Side side = Side::NONE;
        float penetration = 0.f;
        sf::Vector2f normal{0.f, 0.f};
    };

    bool checkCollision(const Entity& a, const Entity& b) const {
        return a.getBounds().intersects(b.getBounds());
    }

    Result checkDetailed(const Entity& a, const Entity& b) const;

    Side resolveCollision(Entity& moving, Entity& other);

    bool pointInEntity(const sf::Vector2f& p, const Entity& e) const {
        return p.x >= e.position.x && p.x <= e.position.x + e.size.x &&
               p.y >= e.position.y && p.y <= e.position.y + e.size.y;
    }

    struct CellKey {
        int x, y;
        bool operator==(const CellKey& o) const { return x == o.x && y == o.y; }
    };

    struct CellKeyHash {
        size_t operator()(const CellKey& k) const {
            return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 16);
        }
    };

    void setCellSize(float size) { m_cellSize = size; }
    float getCellSize() const { return m_cellSize; }

    void findPotentialPairs(const std::vector<Entity*>& entities,
                            std::function<void(Entity&, Entity&)> callback);

private:
    float m_cellSize = 128.f;
    std::unordered_map<CellKey, std::vector<Entity*>, CellKeyHash> m_grid;
};

}
