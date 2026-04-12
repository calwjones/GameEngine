#include <catch2/catch_test_macros.hpp>
#include "Engine/Tile/TileLayer.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityFactory.h"
#include "Engine/Entity/EntityTypeRegistry.h"
#include "Engine/Level/LevelLoader.h"
#include "Engine/Collision/CollisionSystem.h"
#include <filesystem>

using Engine::TileLayer;
using Engine::Entity;
using Engine::CollisionSystem;

namespace {
struct TempFile {
    std::filesystem::path path;
    TempFile() {
        auto dir = std::filesystem::temp_directory_path();
        path = dir / ("tilemap_test_" + std::to_string(std::rand()) + ".json");
    }
    ~TempFile() { std::error_code ec; std::filesystem::remove(path, ec); }
};
}

TEST_CASE("tilelayer set/get and bounds", "[tilemap]") {
    TileLayer t(10, 5, 32.f);
    REQUIRE(t.width() == 10);
    REQUIRE(t.height() == 5);
    REQUIRE(t.cellSize() == 32.f);
    REQUIRE(t.at(0, 0) == 0);

    t.set(3, 2, 7);
    REQUIRE(t.at(3, 2) == 7);
    REQUIRE(t.isSolid(3, 2));
    REQUIRE_FALSE(t.isSolid(4, 2));

    // out-of-bounds is silent and safe
    t.set(-1, 0, 5);
    t.set(0, -1, 5);
    t.set(10, 0, 5);
    t.set(0, 5, 5);
    REQUIRE(t.at(-1, 0) == 0);
    REQUIRE(t.at(10, 0) == 0);

    t.clear();
    REQUIRE(t.at(3, 2) == 0);
}

TEST_CASE("tilelayer world-to-cell round trip", "[tilemap]") {
    TileLayer t(20, 20, 32.f);
    auto c = t.worldToCell(65.f, 100.f);
    REQUIRE(c.x == 2);
    REQUIRE(c.y == 3);

    auto w = t.cellToWorld(2, 3);
    REQUIRE(w.x == 64.f);
    REQUIRE(w.y == 96.f);

    // negatives floor correctly
    auto neg = t.worldToCell(-1.f, -33.f);
    REQUIRE(neg.x == -1);
    REQUIRE(neg.y == -2);
}

TEST_CASE("tile collision pushes entity off a solid cell", "[tilemap]") {
    TileLayer t(10, 10, 32.f);
    t.set(3, 5, 1); // solid tile at world (96, 160) .. (128, 192)

    CollisionSystem cs;

    SECTION("falling onto tile top → pushed up, grounded") {
        Entity player;
        player.size = {16.f, 16.f};
        player.position = {104.f, 154.f}; // bottom overlaps top of tile by 10px
        player.velocity = {0.f, 300.f};
        player.isOnGround = false;

        cs.resolveTileCollision(player, t);
        REQUIRE(player.position.y + player.size.y <= 160.f + 0.01f);
        REQUIRE(player.velocity.y == 0.f);
        REQUIRE(player.isOnGround);
    }

    SECTION("walking into a wall → pushed back, vx zeroed") {
        Entity walker;
        walker.size = {16.f, 16.f};
        walker.position = {88.f, 166.f}; // mostly left of tile, small overlap
        walker.velocity = {200.f, 0.f};

        cs.resolveTileCollision(walker, t);
        REQUIRE(walker.position.x + walker.size.x <= 96.f + 0.01f);
        REQUIRE(walker.velocity.x == 0.f);
    }

    SECTION("static and trigger entities are ignored") {
        Entity staticE;
        staticE.isStatic = true;
        staticE.position = {100.f, 170.f};
        cs.resolveTileCollision(staticE, t);
        REQUIRE(staticE.position.x == 100.f);

        Entity triggerE;
        triggerE.isTrigger = true;
        triggerE.position = {100.f, 170.f};
        cs.resolveTileCollision(triggerE, t);
        REQUIRE(triggerE.position.x == 100.f);
    }
}

TEST_CASE("level loader round-trips tile layer", "[tilemap]") {
    Engine::EntityFactory factory;
    Engine::registerBuiltinTypes(factory);
    Engine::LevelLoader loader;
    loader.setFactory(&factory);

    TileLayer src(5, 4, 32.f);
    src.set(0, 0, 1);
    src.set(4, 3, 2);
    src.set(2, 2, 9);

    std::vector<Entity*> entities;
    TempFile tmp;
    REQUIRE(loader.saveToJSON(tmp.path.string(), entities, 800.f, 600.f, &src));

    TileLayer dst;
    auto loaded = loader.loadFromJSON(tmp.path.string(), &dst);
    for (auto* e : loaded) delete e;

    REQUIRE(dst.width() == 5);
    REQUIRE(dst.height() == 4);
    REQUIRE(dst.cellSize() == 32.f);
    REQUIRE(dst.at(0, 0) == 1);
    REQUIRE(dst.at(4, 3) == 2);
    REQUIRE(dst.at(2, 2) == 9);
    REQUIRE(dst.at(1, 1) == 0);
}
