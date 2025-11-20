#include "CollisionSystem.h"
#include <cmath>
#include <functional>
#include <unordered_set>
#include <utility>

namespace Engine {

// VIVA: this is the classic "minimum translation vector" / min-penetration-axis trick.
// idea: if two rects overlap, we want to push them apart along whichever
// axis has the LEAST overlap (bc thats the axis where they barely missed).
//
// math:
//   ox = overlap on x axis = (hwa + hwb) - |cA.x - cB.x|     ("sum of half widths" minus "center distance")
//   oy = overlap on y axis = (hha + hhb) - |cA.y - cB.y|
// if both > 0 they're overlapping. pick the smaller one as push axis.
// sign of dx/dy tells us which SIDE b got hit on (left vs right, top vs bot)
CollisionSystem::Result CollisionSystem::checkDetailed(const Entity& a, const Entity& b) const {
    Result r;
    sf::Vector2f cA = a.getCenter(), cB = b.getCenter();
    float hwa = a.size.x / 2.f, hha = a.size.y / 2.f;
    float hwb = b.size.x / 2.f, hhb = b.size.y / 2.f;
    float dx = cA.x - cB.x, dy = cA.y - cB.y;
    float ox = hwa + hwb - std::abs(dx), oy = hha + hhb - std::abs(dy);

    if (ox > 0 && oy > 0) {
        r.collided = true;
        // push out the SHORT way — if ox < oy we were barely poking in on x
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

// actually un-overlaps the two entities + kills their velocity on the hit axis.
// returns what side got hit so callers can react (player uses TOP for "im grounded")
CollisionSystem::Side CollisionSystem::resolveCollision(Entity& moving, Entity& other) {
    if (moving.isTrigger || other.isTrigger) return Side::NONE;   // triggers just overlap, never block

    auto r = checkDetailed(moving, other);
    if (!r.collided) return Side::NONE;

    // only move the non-static one. if both static (eg ground vs wall) do nothing.
    // if both dynamic we just move 'moving' for simplicity
    Entity* target = !moving.isStatic ? &moving : (!other.isStatic ? &other : nullptr);
    if (!target) return r.side;

    // flip normal if we're pushing 'other' instead of 'moving' bc the normal is from moving's POV
    sf::Vector2f n = (target == &moving) ? r.normal : -r.normal;
    target->position += n * r.penetration;

    // kill velocity on the axis we just hit so we dont re-collide next frame.
    // also set isOnGround if this was a top-landing (player jump logic keys off this)
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

// BROAD PHASE. builds the spatial hash grid then walks each cell looking for pairs.
// a big entity can straddle several cells — thats fine, we just insert it into
// all of em. means a pair can show up in multiple cells if both entities overlap
// multiple cells together. we dedupe that with the 'checked' set below.
void CollisionSystem::findPotentialPairs(const std::vector<Entity*>& entities,
                                          std::function<void(Entity&, Entity&)> callback) {
    m_grid.clear();

    // bucket every entity into the cells it touches
    for (auto* e : entities) {
        int minX = (int)std::floor(e->position.x / m_cellSize);
        int minY = (int)std::floor(e->position.y / m_cellSize);
        int maxX = (int)std::floor((e->position.x + e->size.x) / m_cellSize);
        int maxY = (int)std::floor((e->position.y + e->size.y) / m_cellSize);

        for (int cx = minX; cx <= maxX; cx++)
            for (int cy = minY; cy <= maxY; cy++)
                m_grid[{cx, cy}].push_back(e);
    }

    // VIVA — subtle UB bug i fixed. to dedupe pairs i need a canonical (a,b) order
    // so {X,Y} and {Y,X} hash the same. i was originally using `a > b` on the raw
    // ptrs — BUT comparing pointers to different allocations with > is undefined
    // behaviour per the C++ standard. works on every real allocator tho, so the
    // bug was invisible. std::less<Entity*> explicitly gives u a defined total
    // order even across unrelated allocs. also using unordered_set instead of set
    // bc unordered_set uses a hash bucket (no per-frame rbtree allocations)
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
