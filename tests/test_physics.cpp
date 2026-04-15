#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Entity/Entity.h"

using Engine::PhysicsEngine;
using Engine::Entity;
using Catch::Matchers::WithinAbs;

namespace {
Entity makeDynamic() {
    Entity e("test", "test");
    e.hasGravity = true;
    e.isStatic = false;
    return e;
}
}

TEST_CASE("gravity accumulates on dynamic entity", "[physics]") {
    PhysicsEngine p;
    p.setGravity(1000.f);
    Entity e = makeDynamic();

    p.applyGravity(e, 0.1f);
    REQUIRE_THAT(e.velocity.y, WithinAbs(100.f, 1e-4f));

    p.applyGravity(e, 0.1f);
    REQUIRE_THAT(e.velocity.y, WithinAbs(200.f, 1e-4f));
}

TEST_CASE("gravity clamps to max fall speed", "[physics]") {
    PhysicsEngine p;
    p.setGravity(1000.f);
    p.setMaxFallSpeed(500.f);
    Entity e = makeDynamic();

    for (int i = 0; i < 100; ++i) p.applyGravity(e, 0.1f);
    REQUIRE_THAT(e.velocity.y, WithinAbs(500.f, 1e-4f));
}

TEST_CASE("static entities ignore gravity", "[physics]") {
    PhysicsEngine p;
    Entity e = makeDynamic();
    e.isStatic = true;

    p.applyGravity(e, 0.5f);
    REQUIRE(e.velocity.y == 0.f);
}

TEST_CASE("entities with hasGravity=false ignore gravity", "[physics]") {
    PhysicsEngine p;
    Entity e = makeDynamic();
    e.hasGravity = false;

    p.applyGravity(e, 0.5f);
    REQUIRE(e.velocity.y == 0.f);
}

TEST_CASE("friction decays horizontal velocity exponentially", "[physics]") {
    PhysicsEngine p;
    p.setGroundFriction(10.f);
    Entity e = makeDynamic();
    e.isOnGround = true;
    e.velocity.x = 100.f;

    p.updateVelocity(e, 0.1f);
    REQUIRE_THAT(e.velocity.x, WithinAbs(100.f * std::exp(-1.f), 1e-4f));
}

TEST_CASE("ground friction is stronger than air friction", "[physics]") {
    PhysicsEngine p;
    p.setGroundFriction(10.f);
    p.setAirFriction(0.5f);

    Entity onGround = makeDynamic();
    onGround.isOnGround = true;
    onGround.velocity.x = 100.f;

    Entity inAir = makeDynamic();
    inAir.isOnGround = false;
    inAir.velocity.x = 100.f;

    p.updateVelocity(onGround, 0.1f);
    p.updateVelocity(inAir, 0.1f);

    REQUIRE(onGround.velocity.x < inAir.velocity.x);
}

TEST_CASE("small horizontal velocity snaps to zero", "[physics]") {
    PhysicsEngine p;
    Entity e = makeDynamic();
    e.velocity.x = 0.05f;

    p.updateVelocity(e, 0.1f);
    REQUIRE(e.velocity.x == 0.f);
}

TEST_CASE("static entities ignore velocity update", "[physics]") {
    PhysicsEngine p;
    Entity e = makeDynamic();
    e.isStatic = true;
    e.velocity.x = 100.f;

    p.updateVelocity(e, 0.5f);
    REQUIRE(e.velocity.x == 100.f);
}

TEST_CASE("impulse adds to velocity", "[physics]") {
    PhysicsEngine p;
    Entity e = makeDynamic();
    e.velocity = {10.f, 20.f};

    p.applyImpulse(e, {5.f, -30.f});
    REQUIRE(e.velocity.x == 15.f);
    REQUIRE(e.velocity.y == -10.f);
}

TEST_CASE("impulse is ignored on static entities", "[physics]") {
    PhysicsEngine p;
    Entity e = makeDynamic();
    e.isStatic = true;

    p.applyImpulse(e, {100.f, 100.f});
    REQUIRE(e.velocity.x == 0.f);
    REQUIRE(e.velocity.y == 0.f);
}
