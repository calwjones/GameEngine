#include "CollisionSystem.h"
#include "../Tile/TileLayer.h"
#include <algorithm>
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

CollisionSystem::Side CollisionSystem::resolveTileCollision(Entity& e, const TileLayer& tiles) {
    if (e.isTrigger || e.isStatic) return Side::NONE;
    const float cs = tiles.cellSize();
    if (cs <= 0.f || tiles.width() <= 0 || tiles.height() <= 0) return Side::NONE;

    Side lastSide = Side::NONE;

    // Up to two passes: resolving one cell can leave overlap with a diagonal neighbor.
    // Two passes handle the common corner case; anything beyond is a geometry bug.
    for (int pass = 0; pass < 2; ++pass) {
        int minX = (int)std::floor(e.position.x / cs);
        int minY = (int)std::floor(e.position.y / cs);
        int maxX = (int)std::floor((e.position.x + e.size.x - 0.0001f) / cs);
        int maxY = (int)std::floor((e.position.y + e.size.y - 0.0001f) / cs);

        struct Hit { int cx, cy; float ox, oy; };
        std::vector<Hit> hits;
        for (int cy = minY; cy <= maxY; ++cy) {
            for (int cx = minX; cx <= maxX; ++cx) {
                if (!tiles.isSolid(cx, cy)) continue;
                float tx = cx * cs, ty = cy * cs;
                float ox = std::min(e.position.x + e.size.x, tx + cs) - std::max(e.position.x, tx);
                float oy = std::min(e.position.y + e.size.y, ty + cs) - std::max(e.position.y, ty);
                if (ox > 0.f && oy > 0.f) hits.push_back({cx, cy, ox, oy});
            }
        }
        if (hits.empty()) break;

        // Resolve the deepest overlap first so shallow grazes don't flip the axis.
        std::sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b) {
            return std::min(a.ox, a.oy) > std::min(b.ox, b.oy);
        });

        for (const auto& h : hits) {
            float tx = h.cx * cs, ty = h.cy * cs;
            float ox = std::min(e.position.x + e.size.x, tx + cs) - std::max(e.position.x, tx);
            float oy = std::min(e.position.y + e.size.y, ty + cs) - std::max(e.position.y, ty);
            if (ox <= 0.f || oy <= 0.f) continue;

            if (ox < oy) {
                float ecx = e.position.x + e.size.x * 0.5f;
                float tcx = tx + cs * 0.5f;
                if (ecx < tcx) {
                    e.position.x -= ox;
                    lastSide = Side::RIGHT;
                } else {
                    e.position.x += ox;
                    lastSide = Side::LEFT;
                }
                e.velocity.x = 0.f;
            } else {
                float ecy = e.position.y + e.size.y * 0.5f;
                float tcy = ty + cs * 0.5f;
                if (ecy < tcy) {
                    e.position.y -= oy;
                    e.velocity.y = 0.f;
                    e.isOnGround = true;
                    lastSide = Side::BOTTOM;
                } else {
                    e.position.y += oy;
                    e.velocity.y = 0.f;
                    lastSide = Side::TOP;
                }
            }
        }
    }
    return lastSide;
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
