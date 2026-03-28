#include "CollisionSystem.h"
#include <cmath>
#include <functional>
#include <unordered_set>
#include <utility>

namespace Engine {

// minimum translation vector: push out on whichever axis has less overlap
CollisionSystem::Result CollisionSystem::checkDetailed(const Entity& a, const Entity& b) const {
    Result r;
    sf::Vector2f cA = a.getCenter(), cB = b.getCenter();
    float hwa = a.size.x / 2.f, hha = a.size.y / 2.f;
    float hwb = b.size.x / 2.f, hhb = b.size.y / 2.f;
    float dx = cA.x - cB.x, dy = cA.y - cB.y;
    float ox = hwa + hwb - std::abs(dx), oy = hha + hhb - std::abs(dy);

    if (ox > 0 && oy > 0) {
        r.collided = true;
        if (ox < oy) {
            r.penetration = ox;
            r.side = dx > 0 ? Side::LEFT : Side::RIGHT;
            r.normal = {dx > 0 ? 1.f : -1.f, 0.f};
        } else {
            r.penetration = oy;
            r.side = dy > 0 ? Side::TOP : Side::BOTTOM;
            r.normal = {0.f, dy > 0 ? 1.f : -1.f};
        }
    }
    return r;
}

CollisionSystem::Side CollisionSystem::resolveCollision(Entity& moving, Entity& other) {
    if (moving.isTrigger || other.isTrigger) return Side::NONE;

    auto r = checkDetailed(moving, other);
    if (!r.collided) return Side::NONE;

    Entity* target = !moving.isStatic ? &moving : (!other.isStatic ? &other : nullptr);
    if (!target) return r.side;

    sf::Vector2f n = (target == &moving) ? r.normal : -r.normal;
    target->position += n * r.penetration;

    if (r.side == Side::TOP || r.side == Side::BOTTOM) {
        target->velocity.y = 0.f;
        if ((target == &moving && r.side == Side::BOTTOM) ||
            (target == &other && r.side == Side::TOP))
            target->isOnGround = true;
    } else {
        target->velocity.x = 0.f;
    }
    return r.side;
}

void CollisionSystem::findPotentialPairs(const std::vector<Entity*>& entities,
                                          std::function<void(Entity&, Entity&)> callback) {
    m_grid.clear();

    for (auto* e : entities) {
        int minX = (int)std::floor(e->position.x / m_cellSize);
        int minY = (int)std::floor(e->position.y / m_cellSize);
        int maxX = (int)std::floor((e->position.x + e->size.x) / m_cellSize);
        int maxY = (int)std::floor((e->position.y + e->size.y) / m_cellSize);

        for (int cx = minX; cx <= maxX; cx++)
            for (int cy = minY; cy <= maxY; cy++)
                m_grid[{cx, cy}].push_back(e);
    }

    // std::less gives a defined total order across unrelated allocations
    struct PairHash {
        size_t operator()(const std::pair<Entity*, Entity*>& p) const noexcept {
            auto h1 = std::hash<Entity*>()(p.first);
            auto h2 = std::hash<Entity*>()(p.second);
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };
    std::unordered_set<std::pair<Entity*, Entity*>, PairHash> checked;
    checked.reserve(entities.size() * 2);
    std::less<Entity*> ptrLess;
    for (auto& [key, cell] : m_grid) {
        for (size_t i = 0; i < cell.size(); i++) {
            for (size_t j = i + 1; j < cell.size(); j++) {
                Entity* a = cell[i];
                Entity* b = cell[j];
                if (ptrLess(b, a)) std::swap(a, b);
                if (checked.insert({a, b}).second) {
                    callback(*a, *b);
                }
            }
        }
    }
}

}
