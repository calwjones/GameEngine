#pragma once
#include "../Entity/Entity.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace Engine {

// AABB = Axis Aligned Bounding Box. means no rotations, just rectangles aligned
// to the xy axes. WAY simpler than full physics + fine for a 2d platformer.
//
// VIVA POINT — two phase collision:
//   BROAD phase = spatial hash grid. "which pairs of entities could POSSIBLY
//       be overlapping?" answered in O(n) instead of O(n²). if u have 100
//       entities thats 10000 pair checks vs ~200, huge win. cells are 128px.
//   NARROW phase = actual rect overlap check on just those candidate pairs.
// without the broad phase u'd still be fine at n=20 but start lagging around
// n=100+ entities
class CollisionSystem {
public:
    // which side of 'other' did 'moving' hit? — used so the player
    // can tell "i landed on top" (TOP) vs "i walked into a wall" (LEFT/RIGHT)
    enum class Side { NONE, TOP, BOTTOM, LEFT, RIGHT };

    struct Result {
        bool collided = false;
        Side side = Side::NONE;
        float penetration = 0.f;    // how deep we're overlapping — resolver pushes by this amount
        sf::Vector2f normal{0.f, 0.f};
    };

    // cheap yes/no check. uses SFML's built in rect intersect — fine for triggers
    bool checkCollision(const Entity& a, const Entity& b) const {
        return a.getBounds().intersects(b.getBounds());
    }

    Result checkDetailed(const Entity& a, const Entity& b) const;

    // the ACTUAL resolver. finds the minimum axis to push 'moving' out of 'other'
    // so they're no longer overlapping. also sets isOnGround=true if we just
    // landed on top of something. returns which side we hit
    Side resolveCollision(Entity& moving, Entity& other);

    bool pointInEntity(const sf::Vector2f& p, const Entity& e) const {
        return p.x >= e.position.x && p.x <= e.position.x + e.size.x &&
               p.y >= e.position.y && p.y <= e.position.y + e.size.y;
    }

    // spatial hash cell coords. operator== + a hash fn so it can live in unordered_map
    struct CellKey {
        int x, y;
        bool operator==(const CellKey& o) const { return x == o.x && y == o.y; }
    };

    struct CellKeyHash {
        // NOT a great hash but good enough for our tiny cell counts.
        // textbook hash would use something like boost::hash_combine
        size_t operator()(const CellKey& k) const {
            return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 16);
        }
    };

    void setCellSize(float size) { m_cellSize = size; }
    float getCellSize() const { return m_cellSize; }

    // THE broad phase entry point. bucket every entity into cells, then for
    // each cell look at pairs within it. callback gets called once per unique
    // potentially-colliding pair. narrow phase happens inside the callback
    void findPotentialPairs(const std::vector<Entity*>& entities,
                            std::function<void(Entity&, Entity&)> callback);

private:
    float m_cellSize = 128.f;
    std::unordered_map<CellKey, std::vector<Entity*>, CellKeyHash> m_grid;
};

}
