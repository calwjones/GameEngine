#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Game/Enemy.h"
#include "Game/FlyingEnemy.h"
#include "Game/ShootingEnemy.h"
#include <cmath>

using Catch::Matchers::WithinAbs;

TEST_CASE("enemy serialize round-trip preserves patrol config", "[behavior]") {
    Game::Enemy src;
    src.setPatrolSpeed(123.f);
    src.setDirection(-1.f);
    src.setPatrolBounds(10.f, 200.f);

    auto props = src.serializeProperties();

    Game::Enemy dst;
    dst.deserializeProperties(props);

    REQUIRE(dst.getPatrolSpeed() == 123.f);
    REQUIRE(dst.getDirection() == -1.f);
    REQUIRE(dst.hasBounds());
    REQUIRE(dst.getMinX() == 10.f);
    REQUIRE(dst.getMaxX() == 200.f);
}

TEST_CASE("enemy reverses and clamps to right bound after overshoot", "[behavior]") {
    Game::Enemy e;
    e.size = {32.f, 32.f};
    e.position = {168.f, 0.f};
    e.setPatrolSpeed(100.f);
    e.setDirection(1.f);
    e.setPatrolBounds(0.f, 200.f);

    e.update(0.1f);

    REQUIRE(e.getDirection() == -1.f);
    REQUIRE_THAT(e.position.x, WithinAbs(168.f, 1e-4f));
}

TEST_CASE("enemy reverses and clamps to left bound after overshoot", "[behavior]") {
    Game::Enemy e;
    e.size = {32.f, 32.f};
    e.position = {0.f, 0.f};
    e.setPatrolSpeed(100.f);
    e.setDirection(-1.f);
    e.setPatrolBounds(0.f, 200.f);

    e.update(0.1f);

    REQUIRE(e.getDirection() == 1.f);
    REQUIRE_THAT(e.position.x, WithinAbs(0.f, 1e-4f));
}

TEST_CASE("enemy stays within patrol bounds over many steps", "[behavior]") {
    Game::Enemy e;
    e.size = {32.f, 32.f};
    e.position = {100.f, 0.f};
    e.setPatrolSpeed(200.f);
    e.setDirection(1.f);
    e.setPatrolBounds(0.f, 200.f);

    for (int i = 0; i < 600; ++i) {
        e.update(1.f / 60.f);
        REQUIRE(e.position.x >= 0.f);
        REQUIRE(e.position.x + e.size.x <= 200.f);
    }
}

TEST_CASE("flying enemy serialize round-trip preserves sine params", "[behavior]") {
    Game::FlyingEnemy src;
    src.setPatrolSpeed(70.f);
    src.setDirection(-1.f);
    src.setAmplitude(40.f);
    src.setFrequency(3.5f);
    src.setBaseY(120.f);
    src.setPatrolBounds(50.f, 300.f);

    auto props = src.serializeProperties();

    Game::FlyingEnemy dst;
    dst.deserializeProperties(props);

    REQUIRE(dst.getPatrolSpeed() == 70.f);
    REQUIRE(dst.getDirection() == -1.f);
    REQUIRE(dst.getAmplitude() == 40.f);
    REQUIRE(dst.getFrequency() == 3.5f);
    REQUIRE(dst.getBaseY() == 120.f);
    REQUIRE(dst.hasBounds());
}

TEST_CASE("flying enemy y matches closed-form sine after one step", "[behavior]") {
    Game::FlyingEnemy e;
    e.size = {28.f, 28.f};
    e.setBaseY(100.f);
    e.setAmplitude(50.f);
    e.setFrequency(2.f);
    e.setPatrolSpeed(0.f);

    const float dt = 0.25f;
    e.update(dt);

    const float expected = 100.f + 50.f * std::sin(2.f * dt);
    REQUIRE_THAT(e.position.y, WithinAbs(expected, 1e-4f));
}

TEST_CASE("shooting enemy serialize round-trip preserves fire config", "[behavior]") {
    Game::ShootingEnemy src;
    src.setPatrolSpeed(45.f);
    src.setDirection(1.f);
    src.setFireRate(1.5f);
    src.setProjectileSpeed(280.f);
    src.setPatrolBounds(20.f, 400.f);

    auto props = src.serializeProperties();

    Game::ShootingEnemy dst;
    dst.deserializeProperties(props);

    REQUIRE(dst.getPatrolSpeed() == 45.f);
    REQUIRE(dst.getFireRate() == 1.5f);
    REQUIRE(dst.getProjectileSpeed() == 280.f);
    REQUIRE(dst.hasBounds());
    REQUIRE(dst.getMinX() == 20.f);
    REQUIRE(dst.getMaxX() == 400.f);
}

TEST_CASE("shooting enemy fires once per fire rate interval", "[behavior]") {
    Game::ShootingEnemy e;
    e.setFireRate(1.f);
    e.setPatrolSpeed(0.f);

    e.update(0.5f);
    REQUIRE_FALSE(e.consumeFireFlag());

    e.update(0.5f);
    REQUIRE(e.consumeFireFlag());

    e.update(0.9f);
    REQUIRE_FALSE(e.consumeFireFlag());

    e.update(0.1f);
    REQUIRE(e.consumeFireFlag());
}

TEST_CASE("shooting enemy fires exactly N times over N intervals", "[behavior]") {
    Game::ShootingEnemy e;
    e.setFireRate(0.5f);
    e.setPatrolSpeed(0.f);

    int shots = 0;
    for (int i = 0; i < 10; ++i) {
        e.update(0.5f);
        if (e.consumeFireFlag()) ++shots;
    }
    REQUIRE(shots == 10);
}
