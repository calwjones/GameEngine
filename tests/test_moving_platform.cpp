#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Game/MovingPlatform.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("moving platform travels from A to B at given speed", "[moving_platform]") {
    Game::MovingPlatform mp;
    mp.setPointA({0.f, 0.f});
    mp.setPointB({100.f, 0.f});
    mp.setSpeed(100.f);
    mp.resetToStart();

    mp.update(0.5f);
    REQUIRE_THAT(mp.position.x, WithinAbs(50.f, 1e-4f));
    REQUIRE_THAT(mp.getDelta().x, WithinAbs(50.f, 1e-4f));
}

TEST_CASE("moving platform reverses at B and returns to A", "[moving_platform]") {
    Game::MovingPlatform mp;
    mp.setPointA({0.f, 0.f});
    mp.setPointB({100.f, 0.f});
    mp.setSpeed(100.f);
    mp.resetToStart();

    mp.update(1.2f);
    REQUIRE_THAT(mp.position.x, WithinAbs(100.f, 1e-4f));

    mp.update(0.5f);
    REQUIRE_THAT(mp.position.x, WithinAbs(50.f, 1e-4f));
    REQUIRE(mp.getDelta().x < 0.f);

    mp.update(0.6f);
    REQUIRE_THAT(mp.position.x, WithinAbs(0.f, 1e-4f));
}

TEST_CASE("moving platform handles zero-distance A==B without NaN", "[moving_platform]") {
    Game::MovingPlatform mp;
    mp.setPointA({100.f, 100.f});
    mp.setPointB({100.f, 100.f});
    mp.setSpeed(80.f);
    mp.position = {100.f, 100.f};

    mp.update(0.5f);
    REQUIRE(mp.getDelta().x == 0.f);
    REQUIRE(mp.getDelta().y == 0.f);
    REQUIRE(mp.position.x == 100.f);
    REQUIRE(mp.position.y == 100.f);
}

TEST_CASE("moving platform moves along diagonal A to B", "[moving_platform]") {
    Game::MovingPlatform mp;
    mp.setPointA({0.f, 0.f});
    mp.setPointB({60.f, 80.f});
    mp.setSpeed(100.f);
    mp.resetToStart();

    mp.update(0.5f);
    REQUIRE_THAT(mp.position.x, WithinAbs(30.f, 1e-4f));
    REQUIRE_THAT(mp.position.y, WithinAbs(40.f, 1e-4f));
}

TEST_CASE("moving platform resetToStart returns to A", "[moving_platform]") {
    Game::MovingPlatform mp;
    mp.setPointA({10.f, 20.f});
    mp.setPointB({200.f, 20.f});
    mp.setSpeed(100.f);
    mp.resetToStart();

    mp.update(0.8f);
    mp.resetToStart();

    REQUIRE(mp.position.x == 10.f);
    REQUIRE(mp.position.y == 20.f);
    REQUIRE(mp.getDelta().x == 0.f);
    REQUIRE(mp.getDelta().y == 0.f);
}

TEST_CASE("moving platform serialize round-trip preserves endpoints and speed", "[moving_platform]") {
    Game::MovingPlatform src;
    src.setPointA({12.f, 34.f});
    src.setPointB({567.f, 89.f});
    src.setSpeed(45.f);

    auto props = src.serializeProperties();

    Game::MovingPlatform dst;
    dst.deserializeProperties(props);

    REQUIRE(dst.getPointA().x == 12.f);
    REQUIRE(dst.getPointA().y == 34.f);
    REQUIRE(dst.getPointB().x == 567.f);
    REQUIRE(dst.getPointB().y == 89.f);
    REQUIRE(dst.getSpeed() == 45.f);
}
