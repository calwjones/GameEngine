#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Game/Enemy.h"
#include "Game/FlyingEnemy.h"
#include "Game/ShootingEnemy.h"

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

TEST_CASE("enemy patrol bounces at right bound", "[behavior]") {
    Game::Enemy e;
    e.size = {32.f, 32.f};
    e.position = {150.f, 0.f};
    e.setPatrolSpeed(100.f);
    e.setDirection(1.f);
    e.setPatrolBounds(0.f, 200.f);

    for (int i = 0; i < 60; ++i) e.update(1.f / 60.f);

    REQUIRE(e.getDirection() == -1.f);
    REQUIRE(e.position.x <= 168.f);
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

TEST_CASE("flying enemy y oscillates around baseY", "[behavior]") {
    Game::FlyingEnemy e;
    e.size = {28.f, 28.f};
    e.position = {0.f, 100.f};
    e.setBaseY(100.f);
    e.setAmplitude(50.f);
    e.setFrequency(2.f);
    e.setPatrolSpeed(0.f);

    float minY = 1e9f, maxY = -1e9f;
    for (int i = 0; i < 240; ++i) {
        e.update(1.f / 60.f);
        minY = std::min(minY, e.position.y);
        maxY = std::max(maxY, e.position.y);
    }

    REQUIRE_THAT(maxY - 100.f, WithinAbs(50.f, 2.f));
    REQUIRE_THAT(100.f - minY, WithinAbs(50.f, 2.f));
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

TEST_CASE("shooting enemy fires on cadence", "[behavior]") {
    Game::ShootingEnemy e;
    e.size = {32.f, 32.f};
    e.position = {0.f, 0.f};
    e.setFireRate(1.f);
    e.setPatrolSpeed(0.f);

    int shots = 0;
    for (int i = 0; i < 300; ++i) {
        e.update(1.f / 60.f);
        if (e.consumeFireFlag()) ++shots;
    }

    REQUIRE(shots >= 4);
    REQUIRE(shots <= 6);
}
