#include <catch2/catch_test_macros.hpp>
#include "Engine/Collision/CollisionSystem.h"
#include "Engine/Entity/Entity.h"

using Engine::CollisionSystem;
using Engine::Entity;
using Side = CollisionSystem::Side;

static Entity makeBox(float x, float y, float w, float h) {
    Entity e;
    e.position = {x, y};
    e.size = {w, h};
    return e;
}

TEST_CASE("AABB intersection", "[collision]") {
    CollisionSystem cs;
    auto a = makeBox(0, 0, 10, 10);
    auto b = makeBox(5, 5, 10, 10);
    auto c = makeBox(20, 20, 5, 5);

    REQUIRE(cs.checkCollision(a, b));
    REQUIRE_FALSE(cs.checkCollision(a, c));
}

TEST_CASE("checkDetailed reports MTV axis", "[collision]") {
    CollisionSystem cs;

    SECTION("smaller X overlap resolves horizontally") {
        auto a = makeBox(0, 0, 10, 20);
        auto b = makeBox(8, 0, 10, 20);
        auto r = cs.checkDetailed(a, b);
        REQUIRE(r.collided);
        REQUIRE(r.side == Side::RIGHT);
        REQUIRE(r.penetration == 2.f);
    }

    SECTION("smaller Y overlap resolves vertically") {
        auto a = makeBox(0, 0, 20, 10);
        auto b = makeBox(0, 8, 20, 10);
        auto r = cs.checkDetailed(a, b);
        REQUIRE(r.collided);
        REQUIRE(r.side == Side::BOTTOM);
        REQUIRE(r.penetration == 2.f);
    }

    SECTION("no overlap reports nothing") {
        auto a = makeBox(0, 0, 10, 10);
        auto b = makeBox(100, 100, 10, 10);
        auto r = cs.checkDetailed(a, b);
        REQUIRE_FALSE(r.collided);
        REQUIRE(r.side == Side::NONE);
    }
}

TEST_CASE("resolveCollision lifts dynamic off static ground", "[collision]") {
    CollisionSystem cs;
    Entity player = makeBox(0, 8, 16, 16);
    player.velocity = {0.f, 200.f};
    Entity ground = makeBox(0, 20, 100, 20);
    ground.isStatic = true;

    auto side = cs.resolveCollision(player, ground);
    REQUIRE(side == Side::BOTTOM);
    REQUIRE(player.isOnGround);
    REQUIRE(player.velocity.y == 0.f);
    REQUIRE(player.position.y + player.size.y <= ground.position.y + 0.001f);
}

TEST_CASE("resolveCollision ignores triggers", "[collision]") {
    CollisionSystem cs;
    Entity player = makeBox(0, 0, 10, 10);
    Entity coin = makeBox(5, 0, 10, 10);
    coin.isTrigger = true;

    auto pBefore = player.position;
    auto side = cs.resolveCollision(player, coin);
    REQUIRE(side == Side::NONE);
    REQUIRE(player.position == pBefore);
}

TEST_CASE("findPotentialPairs deduplicates across cells", "[collision]") {
    CollisionSystem cs;
    cs.setCellSize(32.f);

    Entity a = makeBox(0, 0, 64, 64);
    Entity b = makeBox(16, 16, 64, 64);
    Entity c = makeBox(500, 500, 10, 10);
    std::vector<Entity*> ents{&a, &b, &c};

    int calls = 0;
    bool sawAB = false;
    cs.findPotentialPairs(ents, [&](Entity& x, Entity& y) {
        ++calls;
        if ((&x == &a && &y == &b) || (&x == &b && &y == &a)) sawAB = true;
    });

    REQUIRE(sawAB);
    REQUIRE(calls == 1);
}
