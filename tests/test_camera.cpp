#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Engine/Rendering/Camera.h"

using Engine::Camera;
using Catch::Matchers::WithinAbs;

TEST_CASE("camera clamps normally when view fits inside level", "[camera]") {
    Camera c;
    c.setBounds(0.f, 0.f, 1600.f, 900.f);
    c.setViewSize(400.f, 300.f);

    SECTION("target inside level clamps to target") {
        c.setPosition({800.f, 450.f});
        c.snapToTarget();
        REQUIRE_THAT(c.getPosition().x, WithinAbs(800.f, 0.01f));
        REQUIRE_THAT(c.getPosition().y, WithinAbs(450.f, 0.01f));
    }

    SECTION("target past left edge clamps to half-view from left") {
        c.setPosition({0.f, 450.f});
        c.snapToTarget();
        REQUIRE_THAT(c.getPosition().x, WithinAbs(200.f, 0.01f));
    }

    SECTION("target past right edge clamps to half-view from right") {
        c.setPosition({2000.f, 450.f});
        c.snapToTarget();
        REQUIRE_THAT(c.getPosition().x, WithinAbs(1400.f, 0.01f));
    }
}

TEST_CASE("camera centers when view is larger than level", "[camera]") {
    Camera c;
    c.setBounds(0.f, 0.f, 1600.f, 900.f);

    SECTION("view wider than level centers X") {
        c.setViewSize(3200.f, 300.f);
        c.setPosition({0.f, 450.f});
        c.snapToTarget();
        REQUIRE_THAT(c.getPosition().x, WithinAbs(800.f, 0.01f));
        REQUIRE_THAT(c.getPosition().y, WithinAbs(450.f, 0.01f));
    }

    SECTION("view taller than level centers Y") {
        c.setViewSize(400.f, 1800.f);
        c.setPosition({800.f, 0.f});
        c.snapToTarget();
        REQUIRE_THAT(c.getPosition().x, WithinAbs(800.f, 0.01f));
        REQUIRE_THAT(c.getPosition().y, WithinAbs(450.f, 0.01f));
    }

    SECTION("view equal to level centers both") {
        c.setViewSize(1600.f, 900.f);
        c.setPosition({0.f, 0.f});
        c.snapToTarget();
        REQUIRE_THAT(c.getPosition().x, WithinAbs(800.f, 0.01f));
        REQUIRE_THAT(c.getPosition().y, WithinAbs(450.f, 0.01f));
    }
}

TEST_CASE("camera lerps toward target", "[camera]") {
    Camera c;
    c.setBounds(0.f, 0.f, 10000.f, 10000.f);
    c.setViewSize(400.f, 300.f);
    c.setPosition({1000.f, 1000.f});
    c.setTarget({2000.f, 1000.f});
    c.setLerpSpeed(5.f);

    float before = c.getPosition().x;
    c.update(1.f / 60.f);
    float after = c.getPosition().x;

    REQUIRE(after > before);
    REQUIRE(after < 2000.f);
}

TEST_CASE("camera ignores bounds after clearBounds", "[camera]") {
    Camera c;
    c.setBounds(0.f, 0.f, 100.f, 100.f);
    c.setViewSize(50.f, 50.f);
    c.clearBounds();

    c.setPosition({-9999.f, 9999.f});
    c.snapToTarget();
    REQUIRE_THAT(c.getPosition().x, WithinAbs(-9999.f, 0.01f));
    REQUIRE_THAT(c.getPosition().y, WithinAbs(9999.f, 0.01f));
}
